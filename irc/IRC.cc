/*
        cpIRC - C++ class based IRC protocol wrapper
        Copyright (C) 2003 Iain Sheppard

        This library is free software; you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public
        License as published by the Free Software Foundation; either
        version 2.1 of the License, or (at your option) any later version.

        This library is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
        Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with this library; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

        Contacting the author:
        ~~~~~~~~~~~~~~~~~~~~~~

        email:	iainsheppard@yahoo.co.uk
        IRC:	#magpie @ irc.quakenet.org
 */

#include "IRC.h"

#ifdef WIN32
#include <windows.h>
#else

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define closesocket(s) close(s)
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#endif

#include <err.h>


//remove at will
//#define __IRC_DEBUG__ 1

IRC::IRC() {
    hooks = 0;
    chan_users = 0;
    connected = false;
    sentnick = false;
    sentpass = false;
    sentuser = false;
    cur_nick = 0;
}

IRC::~IRC() {
    if (hooks)
        delete_irc_command_hook(hooks);
}

void IRC::insert_irc_command_hook(irc_command_hook* hook, char* cmd_name, int (*function_ptr)(char*, irc_reply_data*, void*)) {
    if (hook->function) {
        if (!hook->next) {
            hook->next = new irc_command_hook;
            hook->next->function = 0;
            hook->next->irc_command = 0;
            hook->next->next = 0;
        }
        insert_irc_command_hook(hook->next, cmd_name, function_ptr);
    } else {
        hook->function = function_ptr;
        hook->irc_command = new char[strlen(cmd_name) + 1];
        strcpy(hook->irc_command, cmd_name);
    }
}

void IRC::hook_irc_command(char* cmd_name, int (*function_ptr)(char*, irc_reply_data*, void*)) {
    if (!hooks) {
        hooks = new irc_command_hook;
        hooks->function = 0;
        hooks->irc_command = 0;
        hooks->next = 0;
        insert_irc_command_hook(hooks, cmd_name, function_ptr);
    } else {
        insert_irc_command_hook(hooks, cmd_name, function_ptr);
    }
}

void IRC::delete_irc_command_hook(irc_command_hook* cmd_hook) {
    if (cmd_hook->next)
        delete_irc_command_hook(cmd_hook->next);
    if (cmd_hook->irc_command)
        delete cmd_hook->irc_command;
    delete cmd_hook;
}

void IRC::print(int a, int c, char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	log1("\033[%d;%dm%s\033[0m", a, c, buf);
}
void IRC::print(char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	print(0, 37, buf);
}

int IRC::start(const char* server, int port,
        const char* nick, const char* user, const char* name,
        const char* pass) {
#ifdef WIN32
    HOSTENT* resolv;
#else
    hostent* resolv;
#endif
    sockaddr_in rem;

    if (connected)
        return 1;

    irc_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (irc_socket == INVALID_SOCKET) {
#ifdef __IRC_DEBUG__
        printf("Error opening socket.\n");
#endif
        return 1;
    }
#ifdef __IRC_DEBUG__
    printf("Resolving irc server...\n");
#endif
    resolv = gethostbyname(server);
    if (!resolv) {
#ifdef __IRC_DEBUG__
        printf("Unable to resolve irc server.\n");
#endif
        closesocket(irc_socket);
        return 1;
    }
    memcpy(&rem.sin_addr, resolv->h_addr, 4);
    rem.sin_family = AF_INET;
    rem.sin_port = htons(port);
#ifdef __IRC_DEBUG__
    printf("Connecting IRC socket...\n");
#endif
    if (connect(irc_socket, (const sockaddr*) &rem, sizeof (rem)) == SOCKET_ERROR) {
#ifdef __IRC_DEBUG__
        printf("Connection terminated.\n");
#endif
        closesocket(irc_socket);
        return 1;
    }

    dataout = fdopen(irc_socket, "w");
    //datain=fdopen(irc_socket, "r");

    if (!dataout /*|| !datain*/) {
#ifdef __IRC_DEBUG__
        printf("Failed to open streams!\n");
#endif
        closesocket(irc_socket);
        return 1;
    }

    connected = true;

    cur_nick = new char[strlen(nick) + 1];
    strcpy(cur_nick, nick);

    fprintf(dataout, "PASS %s\r\n", pass);
    fprintf(dataout, "NICK %s\r\n", nick);
    fprintf(dataout, "USER %s * 0 :%s\r\n", user, name);
    fflush(dataout);
#ifdef __IRC_DEBUG__
    printf("Sent connection info to the irc server...\n");
#endif

    return 0;
}

void IRC::disconnect() {
    if (connected) {
        fclose(dataout);
#ifdef __IRC_DEBUG__
        printf("Disconnected from server.\n");
#endif
        connected = false;
        quit("Leaving");
#ifdef WIN32
        shutdown(irc_socket, 2);
#endif
        closesocket(irc_socket);
    }
}

int IRC::quit(const char* quit_message) {
    if (connected) {
        if (quit_message)
            fprintf(dataout, "QUIT %s\r\n", quit_message);
        else
            fprintf(dataout, "QUIT\r\n");
        if (fflush(dataout)) {
            disconnect();
            return 1;
        }
        disconnect();
    }
    return 0;
}

int IRC::message_loop() {
    char buffer[2048];
    int ret_len;
    int rawtest;
    int slreason;
    slreason = 0;
    struct timeval tv;
    fd_set readfds;
    rawtest = 0;
    if (!connected) {
#ifdef __IRC_DEBUG__
        printf("Not connected!\n");
#endif
        return 1;
    }
    idle_time = 0;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(irc_socket, &readfds);

        tv.tv_sec = 1;
        tv.tv_usec = 100000;
        slreason = select(irc_socket + 1, &readfds, NULL, NULL, &tv);

        if ((FD_ISSET(irc_socket, &readfds)) && (slreason > 0)) {
            idle_time = 0;
            rawtest = 0;
            ret_len = 0;
#ifdef TRIP_DEBUG
            printf("Reading socket\n");
#endif
            ret_len = recv(irc_socket, buffer, 2047, 0);
            if (ret_len == SOCKET_ERROR || !ret_len) {
                return 1;
            }
            buffer[ret_len] = '\0';
            split_to_replies(buffer);
        } else {
            idle_time += 1;
            if (idle_time == 1) {
#ifdef TRIP_DEBUG
                printf("Sleeping... %d\n", rawtest);
#endif
            }
            if (idle_time > 90) {
                if (rawtest > 1) {
                    disconnect();
                    return 1;
                } else {
                    rawtest += 1;
                    raw("PING :111");
                    idle_time = 0;
                }
            }//if idletime
        }//if fd is not set
    }

    return 0;
}

void IRC::split_to_replies(char* data) {
    char* p;

    while ((p = strstr(data, "\r\n"))) {
        *p = '\0';
        parse_irc_reply(data);
        data = p + 2;
    }
}


int IRC::is_on(const char* channel, const char* nick) {
    channel_user* cup;
    cup = chan_users;
    while (cup) {
        if (!strcmp(cup->channel, channel) && !strcmp(cup->nick, nick)) {
            return 1;
        }
        cup = cup->next;
    }
    return 0;
}

int IRC::is_op(const char* channel, const char* nick) {
    channel_user* cup;

    cup = chan_users;

    while (cup) {
        if (!strcmp(cup->channel, channel) && !strcmp(cup->nick, nick)) {
            return cup->flags&IRC_USER_OP;
        }
        cup = cup->next;
    }

    return 0;
}

int IRC::is_voice(const char* channel, const char* nick) {
    channel_user* cup;

    cup = chan_users;

    while (cup) {
        if (!strcmp(cup->channel, channel) && !strcmp(cup->nick, nick)) {
            return cup->flags&IRC_USER_VOICE;
        }
        cup = cup->next;
    }

    return 0;
}

void IRC::parse_irc_reply(char* data) {
#ifdef TRIP_DEBUG
    printf("Parse IRC reply\n");
#endif
    char* hostd = (char*)malloc(strlen(data));
    char* cmd;
    char* params;
    //char buffer[514];
    irc_reply_data hostd_tmp;
    channel_user* cup;
    char* p;
    char* chan_temp;

    hostd_tmp.target = 0;

#ifdef __IRC_DEBUG__
    printf("%s\n", data);
#endif


    if (data[0] == ':') {
		strcpy(hostd, &data[1]);
        cmd = strchr(hostd, ' ');
        if (!cmd)
            return;
        *cmd = '\0';
        cmd++;
        params = strchr(cmd, ' ');
        if (params) {
            *params = '\0';
            params++;
        }
        hostd_tmp.nick = hostd;
        hostd_tmp.ident = strchr(hostd, '!');
        if (hostd_tmp.ident) {
            *hostd_tmp.ident = '\0';
            hostd_tmp.ident++;
            hostd_tmp.host = strchr(hostd_tmp.ident, '@');
            if (hostd_tmp.host) {
                *hostd_tmp.host = '\0';
                hostd_tmp.host++;
            }
        }


#ifdef TRIP_DEBUG
        printf("Analyzing command\n");
#endif
        if (!strcmp(cmd, "JOIN")) {
			chan_temp = params;
			if (chan_temp[0] == ':') chan_temp++;
			print(2, 32, "%s: %s (%s@%s) joined\n", chan_temp, hostd_tmp.nick, hostd_tmp.ident, hostd_tmp.host);
            cup = chan_users;
            if (cup) {
                while (cup->nick) {
                    if (!cup->next) {
                        cup->next = new channel_user;
                        cup->next->channel = 0;
                        cup->next->flags = 0;
                        cup->next->next = 0;
                        cup->next->nick = 0;
                    }
                    cup = cup->next;
                }
                cup->channel = new char[strlen(params) + 1];
                strcpy(cup->channel, params);
                cup->nick = new char[strlen(hostd_tmp.nick) + 1];
                strcpy(cup->nick, hostd_tmp.nick);
            }
        } else if (!strcmp(cmd, "PART")) {
			print(2, 33, "%s: %s (%s@%s) part\n", params, hostd_tmp.nick, hostd_tmp.ident, hostd_tmp.host);
            channel_user* d;
            channel_user* prev;

            d = 0;
            prev = 0;
            cup = chan_users;
            while (cup) {
                if (!strcmp(cup->channel, params) && !strcmp(cup->nick, hostd_tmp.nick)) {
                    d = cup;
                    break;
                } else {
                    prev = cup;
                }
                cup = cup->next;
            }
            if (d) {
                if (d == chan_users) {
                    chan_users = d->next;
                    if (d->channel)
                        delete [] d->channel;
                    if (d->nick)
                        delete [] d->nick;
                    delete d;
                } else {
                    if (prev) {
                        prev->next = d->next;
                    }
                    chan_users = d->next;
                    if (d->channel)
                        delete [] d->channel;
                    if (d->nick)
                        delete [] d->nick;
                    delete d;
                }
            }
        } else if (!strcmp(cmd, "QUIT")) {
			chan_temp = params;
			if (chan_temp[0] == ':') chan_temp++;
			print(2, 33, "%s (%s@%s) quit (%s)\n", hostd_tmp.nick, hostd_tmp.ident, hostd_tmp.host, chan_temp);
            channel_user* d;
            channel_user* prev;

            d = 0;
            prev = 0;
            cup = chan_users;
            while (cup) {
                if (!strcmp(cup->nick, hostd_tmp.nick)) {
                    d = cup;
                    if (d == chan_users) {
                        chan_users = d->next;
                        if (d->channel)
                            delete [] d->channel;
                        if (d->nick)
                            delete [] d->nick;
                        delete d;
                    } else {
                        if (prev) {
                            prev->next = d->next;
                        }
                        if (d->channel)
                            delete [] d->channel;
                        if (d->nick)
                            delete [] d->nick;
                        delete d;
                    }
                    break;
                } else {
                    prev = cup;
                }
                cup = cup->next;
            }
        } else if (!strcmp(cmd, "MODE")) {
            char* chan;
            char* changevars;
            channel_user* cup;
            channel_user* d;
            char* tmp;
            int i;
            bool plus;

            chan = params;
            params = strchr(chan, ' ');
            *params = '\0';
            params++;
            changevars = params;
            params = strchr(changevars, ' ');
            if (!params) {
                return;
            }
            if (chan[0] != '#') {
                return;
            }
            *params = '\0';
            params++;

            plus = false;
            for (i = 0; i < (signed)strlen(changevars); i++) {
                switch (changevars[i]) {
                    case '+':
                        plus = true;
                        break;
                    case '-':
                        plus = false;
                        break;
                    case 'o':
                        tmp = strchr(params, ' ');
                        if (tmp) {
                            *tmp = '\0';
                            tmp++;
                        }
                        tmp = params;
                        if (plus) {
                            // user has been opped (chan, params)
                            cup = chan_users;
                            d = 0;
                            while (cup) {
                                if (cup->next && cup->channel) {
                                    if (!strcmp(cup->channel, chan) && !strcmp(cup->nick, tmp)) {
                                        d = cup;
                                        break;
                                    }
                                }
                                cup = cup->next;
                            }
                            if (d) {
                                d->flags = d->flags | IRC_USER_OP;
                            }
                        } else {
                            // user has been deopped (chan, params)
                            cup = chan_users;
                            d = 0;
                            while (cup) {
                                if (!strcmp(cup->channel, chan) && !strcmp(cup->nick, tmp)) {
                                    d = cup;
                                    break;
                                }
                                cup = cup->next;
                            }
                            if (d) {
                                d->flags = d->flags^IRC_USER_OP;
                            }
                        }
                        params = tmp;
                        break;
                    case 'v':
                        tmp = strchr(params, ' ');
                        if (tmp) {
                            *tmp = '\0';
                            tmp++;
                        }
                        if (plus) {
                            // user has been voiced
                            cup = chan_users;
                            d = 0;
                            while (cup) {
                                if (!strcmp(cup->channel, params) && !strcmp(cup->nick, hostd_tmp.nick)) {
                                    d = cup;
                                    break;
                                }
                                cup = cup->next;
                            }
                            if (d) {
                                d->flags = d->flags | IRC_USER_VOICE;
                            }
                        } else {
                            // user has been devoiced
                            cup = chan_users;
                            d = 0;
                            while (cup) {
                                if (!strcmp(cup->channel, params) && !strcmp(cup->nick, hostd_tmp.nick)) {
                                    d = cup;
                                    break;
                                }
                                cup = cup->next;
                            }
                            if (d) {
                                d->flags = d->flags^IRC_USER_VOICE;
                            }
                        }
                        params = tmp;
                        break;
                    default:
                        return;
                        break;
                }
                // ------------ END OF MODE ---------------
            }
        } else if (!strcmp(cmd, "353")) {
            // receiving channel names list
            if (!chan_users) {
                chan_users = new channel_user;
                chan_users->next = 0;
                chan_users->nick = 0;
                chan_users->flags = 0;
                chan_users->channel = 0;
            }
            cup = chan_users;
            chan_temp = strchr(params, '#');
            if (chan_temp) {
                //chan_temp+=3;
                p = strstr(chan_temp, " :");
                if (p) {
                    *p = '\0';
                    p += 2;
                    while (strchr(p, ' ')) {
                        char* tmp;

                        tmp = strchr(p, ' ');
                        *tmp = '\0';
                        tmp++;
                        while (cup->nick) {
                            if (!cup->next) {
                                cup->next = new channel_user;
                                cup->next->channel = 0;
                                cup->next->flags = 0;
                                cup->next->next = 0;
                                cup->next->nick = 0;
                            }
                            cup = cup->next;
                        }
                        if (p[0] == '@') {
                            cup->flags = cup->flags | IRC_USER_OP;
                            p++;
                        } else if (p[0] == '+') {
                            cup->flags = cup->flags | IRC_USER_VOICE;
                            p++;
                        }
                        cup->nick = new char[strlen(p) + 1];
                        strcpy(cup->nick, p);
                        cup->channel = new char[strlen(chan_temp) + 1];
                        strcpy(cup->channel, chan_temp);
                        p = tmp;
                    }
                    while (cup->nick) {
                        if (!cup->next) {
                            cup->next = new channel_user;
                            cup->next->channel = 0;
                            cup->next->flags = 0;
                            cup->next->next = 0;
                            cup->next->nick = 0;
                        }
                        cup = cup->next;
                    }
                    if (p[0] == '@') {
                        cup->flags = cup->flags | IRC_USER_OP;
                        p++;
                    } else if (p[0] == '+') {
                        cup->flags = cup->flags | IRC_USER_VOICE;
                        p++;
                    }
                    cup->nick = new char[strlen(p) + 1];
                    strcpy(cup->nick, p);
                    cup->channel = new char[strlen(chan_temp) + 1];
                    strcpy(cup->channel, chan_temp);
                }
            }
        } else if (!strcmp(cmd, "NOTICE")) {
            hostd_tmp.target = params;
            params = strchr(hostd_tmp.target, ' ');
            if (params)
                *params = '\0';
            params++;
#ifdef __IRC_DEBUG__
            printf("%s >-%s- %s\n", hostd_tmp.nick, hostd_tmp.target, &params[1]);
#endif
        } else if (!strcmp(cmd, "PRIVMSG")) {
            hostd_tmp.target = params;
            params = strchr(hostd_tmp.target, ' ');
            if (!params)
                return;
            *(params++) = '\0';
            print(0, 36, "%s: <%s> %s\n", hostd_tmp.target, hostd_tmp.nick, &params[1]);

        } else if (!strcmp(cmd, "NICK")) {
            if (!strcmp(hostd_tmp.nick, cur_nick)) {
                delete [] cur_nick;
                cur_nick = new char[strlen(params) + 1];
                strcpy(cur_nick, params);
            }
        } else {
			print("%s\n", data);
		}
#ifdef TRIP_DEBUG
        printf("Calling hoook\n");
#endif
        call_hook(cmd, params, &hostd_tmp);
#ifdef TRIP_DEBUG
        printf("Hook call success.\n");
#endif
    } else {
        cmd = data;
        data = strchr(cmd, ' ');
        if (!data)
            return;
        *data = '\0';
        params = data + 1;

        if (!strcmp(cmd, "PING")) {
            if (!params)
                return;
            fprintf(dataout, "PONG %s\r\n", &params[1]);
#ifdef __IRC_DEBUG__
            printf("Ping received, pong sent.\n");
#endif
            fflush(dataout);
        } else {
            hostd_tmp.host = 0;
            hostd_tmp.ident = 0;
            hostd_tmp.nick = 0;
            hostd_tmp.target = 0;
            call_hook(cmd, params, &hostd_tmp);
        }
    }
#ifdef TRIP_DEBUG
    printf("End parsing IRC reply\n");
#endif
}

void IRC::call_hook(char* irc_command, char* params, irc_reply_data* hostd) {
    irc_command_hook* p;

    if (!hooks)
        return;

    p = hooks;
    while (p) {
        if (!strcmp(p->irc_command, irc_command)) {
            (*(p->function))(params, hostd, this);
            p = 0;
        } else {
            p = p->next;
        }
    }
}

int IRC::notice(const char* target, const char* message) {
    if (!connected)
        return 1;
    fprintf(dataout, "NOTICE %s :%s\r\n", target, message);
    return fflush(dataout);
}

int IRC::notice(char* fmt, ...) {
    va_list argp;
    //	char* target;

    if (!connected)
        return 1;
    va_start(argp, fmt);
    fprintf(dataout, "NOTICE %s :", fmt);
    vfprintf(dataout, va_arg(argp, char*), argp);
    va_end(argp);
    fprintf(dataout, "\r\n");
    return fflush(dataout);
}

int IRC::privmsg(const char* target, const char* message) {
    if (!connected)
        return 1;
    fprintf(dataout, "PRIVMSG %s :%s\r\n", target, message);
    return fflush(dataout);
}

int IRC::privmsg(char* fmt, ...) {
    va_list argp;
    //	char* target;

    if (!connected)
        return 1;
    va_start(argp, fmt);
    fprintf(dataout, "PRIVMSG %s :", fmt);
    vfprintf(dataout, va_arg(argp, char*), argp);
    va_end(argp);
    fprintf(dataout, "\r\n");
    return fflush(dataout);
}

int IRC::join(const char* channel) {
    if (!connected)
        return 1;
    fprintf(dataout, "JOIN %s\r\n", channel);
    return fflush(dataout);
}

int IRC::part(const char* channel) {
    if (!connected)
        return 1;
    fprintf(dataout, "PART %s\r\n", channel);
    return fflush(dataout);
}

int IRC::kick(const char* channel, const char* nick) {
    if (!connected)
        return 1;
    fprintf(dataout, "KICK %s %s\r\n", channel, nick);
    return fflush(dataout);
}

int IRC::raw(const char* data) {
    if (!connected)
        return 1;
    fprintf(dataout, "%s\r\n", data);
    return fflush(dataout);
}

int IRC::kick(const char* channel, const char* nick, const char* message) {
    if (!connected)
        return 1;
    fprintf(dataout, "KICK %s %s :%s\r\n", channel, nick, message);
    return fflush(dataout);
}

int IRC::mode(const char* channel, const char* modes, const char* targets) {
    if (!connected)
        return 1;
    if (!targets)
        fprintf(dataout, "MODE %s %s\r\n", channel, modes);
    else
        fprintf(dataout, "MODE %s %s %s\r\n", channel, modes, targets);
    return fflush(dataout);
}

int IRC::mode(const char* modes) {
    if (!connected)
        return 1;
    mode(cur_nick, modes, 0);
    return 0;
}

int IRC::nick(const char* newnick) {
    if (!connected)
        return 1;
    fprintf(dataout, "NICK %s\r\n", newnick);
    return fflush(dataout);
}

char* IRC::current_nick() {
    return cur_nick;
}
