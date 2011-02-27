/*
 *  Copyright (C) Gorgi Kosev a.k.a. Spion
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

//#define TRIP_DEBUG 1

#include "irc/IRC.h"
#include "wildcard/wildcards.cpp"
#include "ai/ai.hpp"
#include <cctype>
#include <fstream>

#include <ctime>

#define TRIP_DISTRUBUTED 1

//#include <cstdlib>


using namespace std;

IRC* conn;
AI* tai; //("botdata/triplie.db");
CAdminList admins;
CAdminList ignores;
unsigned int aimodel;
unsigned int aipermute;

string server, defnick, defuser, defname, defchar, serverpass = "";
vector<string> defchans;
unsigned int defport;
bool shouldreconnect;

ofstream logfile;

int sleepmin, sleepmax;

/* ---------------- */
int procprivm(char* params, irc_reply_data* hostd, void* conn);
int end_of_motd(char* params, irc_reply_data* hostd, void* conn);
void forktobg(string);
void readadmins();
void readignores();
int admin_auth(const std::string& strhost);
int check_ignore(const std::string& strhost);

void cmd_thread(void* irc_conn);
int readsettings(string);
void signal_handler(int sig);

/* ****************************************
   The code.
 * ************************************* */
void signal_handler(int sig) {
    switch (sig) {
        case SIGHUP:
            //log_message(LOG_FILE,"hangup signal catched");
            break;
        case SIGTERM:
            //log_message(LOG_FILE,"terminate signal catched");
            exit(0);
            break;
    }
}

int main(int argc, char** argv) {
    string confFile = "triplie.conf";
    if (argc > 1) {
        confFile = argv[1];
    }
    tai = new AI("botdata/triplie.db");
    conn = new IRC();
    setlocale(LC_ALL, "en_US.utf8");
    defchar = "!";
    sleepmin = 0;
    sleepmax = 0;
    shouldreconnect = true;

    /* Displaying a banner with info... */
#ifdef TRIP_DEBUG
    cout << "Debug mode enabled." << endl;
#endif
    cout << "Triple AI bot started" << endl
            << "Admin database@'admins.dat' ai db@'botdata/triplie.db'" << endl;

            readsettings(confFile);
    cout << "Server " << server << ":" << defport << endl;
    cout << "Nickname: " << defnick << " Ident: " << defuser << endl;

    //* this might need another seed in WIN32 *//
    srand(time(0));

    readadmins();
    readignores();
    tai->readalldata();
    cout << tai->countwords() << " words, ";
    cout << tai->countrels() << " relations, ";
    cout << tai->countvrels() << " associations in database." << endl;

#ifdef TRIP_DEBUG
    cout << "Running in debug mode..." << endl;
#else
    /* fork to background and prevent running twice */
    tai->CloseDB();
    forktobg(confFile);
    tai->OpenDB();
#endif
    aimodel = 2;
    /* start IRC session */
#ifdef TRIP_DEBUG
    cout << "Hooking irc commands to functions..." << endl;
#endif
    conn->hook_irc_command((char *) "PRIVMSG", &procprivm);
    conn->hook_irc_command((char *) "001", &end_of_motd);
#ifdef TRIP_DEBUG
    cout << "Connecting to the server..." << endl;
#endif
    conn->start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), "");
#ifdef TRIP_DEBUG
    cout << "Entering message loop..." << endl;
#endif

    tai->connect_to_workers("workers.dat");
    while (shouldreconnect) {
        conn->message_loop();
        if (shouldreconnect) {
            sleep(61);
            conn->disconnect();
#ifdef TRIP_DEBUG
            cout << "Reconnecting..." << endl;
#endif
            conn->start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), serverpass.c_str());
        }
    }

    tai->SendAllSlavesAndWait("06 \n");
    tai->savealldata();
#ifdef TRIP_DEBUG
    cout << "Loop exit, triplie shutdown." << endl;
#endif
    return 0;
}

void forktobg(string configFile) {
    int i;
    i = fork();
    if (i < 0) exit(1); /* fork error */
    if (i > 0) exit(0); /* parent exits */
    setsid();
    for (i = getdtablesize(); i >= 0; --i) close(i);
    i = open("/dev/null", O_RDWR); /* open stdin */
    int k = dup(i); /* stdout */
    k = dup(i); /* stderr */
#ifdef linux

    int lfp;
    char fstr[10];
    umask(027);

    string lockFile = configFile + ".lock";
    lfp = open(lockFile.c_str(), O_RDWR | O_CREAT, 0640);
    if (lfp < 0) exit(1);
    if (lockf(lfp, F_TLOCK, 0) < 0) exit(0);
    sprintf(fstr, "%d\n", getpid());
    k = write(lfp, fstr, strlen(fstr));

#endif
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

/* ***************
 * IRC HOOKS     *
 * ***************/

int end_of_motd(char*, irc_reply_data*, void*) {
    IRC* irc_conn = conn;
    vector<string>::iterator it;
    for (it = defchans.begin(); it != defchans.end(); it++) {
        irc_conn->join(it->c_str());
    }

    return 0;
}

int procprivm(char* params, irc_reply_data* hostd, void* conn) {

#ifdef TRIP_DEBUG
    cout << "Processing private message..." << endl;
#endif
    Wildcard wildmatch;
    string msg, userhost, msgtarget, rawcmd, mynick, rnick, stupidtmp;

    unsigned long int x; // long int i; long int j; long int litmp;
    int isadmin;

    vector<string> tokens;
    vector<string> tuplets;

    IRC* irc_conn = (IRC *) conn;

    char dictsize[11] = {0};
    char relcountstr[16] = {0};

    /* code */


    msg = string(params);


    rnick = hostd->nick;
    userhost = rnick;
    userhost += "!" + string(hostd->ident);
    userhost += "@" + string(hostd->host);

    msgtarget = hostd->target;

    string wheretosend;
    if ((msgtarget[0] == '#') || (msgtarget[0] == '&')) {
        wheretosend = msgtarget;
    } else {
        wheretosend = rnick;
    }

    mynick = irc_conn->current_nick();
#ifdef TRIP_DEBUG
    cout << "Lowercasing data..." << endl;
#endif
    lowercase(mynick);
    lowercase(msg);
    //for(x=0;x<msg.size();x++) { msg[x]=to//lower(msg[x]); }

    tokenize(msg, tokens, " ,:");
    isadmin = admin_auth(userhost);
    if (check_ignore(userhost)) return 0;
#ifdef TRIP_DEBUG
    cout << "Processing private message text..." << endl;
#endif
    rawcmd = "";
    x = (long int) tokens.size();
    if (x >= 1) {
#ifdef TRIP_DEBUG
        cout << "Message has tokens..." << endl;
#endif
        string dc = defchar;
        string stupidtmp = dc + "*";
        /* lets see if we have a command here... from the admin. */
        if ((wildmatch.wildcardfit(stupidtmp.c_str(), tokens[0].c_str()))
                && (isadmin)
                ) {
            /* command to join/part channel */

            if (tokens[0] == dc + "join") {
                if (x >= 2) irc_conn->join(tokens[1].c_str());
            } else if (tokens[0] == dc + "part") {
                if (x >= 2) irc_conn->part(tokens[1].c_str());
            } else if (tokens[0] == dc + "op") {
                irc_conn->mode(msgtarget.c_str(), "+oooooo", subtokstring(tokens, 1, 6, " ").c_str());
            } else if (tokens[0] == dc + "deop") {
                irc_conn->mode(msgtarget.c_str(), "-oooooo", subtokstring(tokens, 1, 6, " ").c_str());
            } else if (tokens[0] == dc + "vo") {
                irc_conn->mode(msgtarget.c_str(), "+vvvvvv", subtokstring(tokens, 1, 6, " ").c_str());
            } else if (tokens[0] == dc + "devo") {
                irc_conn->mode(msgtarget.c_str(), "-vvvvvv", subtokstring(tokens, 1, 6, " ").c_str());
            } else if (tokens[0] == dc + "topic") {
                rawcmd = "TOPIC " + msgtarget + " :" + subtokstring(tokens, 1, 100, " ");
                irc_conn->raw(rawcmd.c_str());
            } else if (tokens[0] == dc + "quit") {
                if (x >= 2) {
                    rawcmd = subtokstring(tokens, 1, 100, " ");
                } else {
                    rawcmd = "Quit";
                }
                irc_conn->quit((string(":") + rawcmd).c_str());
            } else if (tokens[0] == dc + "say") {
                if (x >= 2) {
                    rawcmd = "PRIVMSG " + msgtarget + " :" + subtokstring(tokens, 1, 100, " ");
                    irc_conn->raw(rawcmd.c_str());
                } else {
                    rawcmd = "";
                }
            } else if (tokens[0] == dc + "msg") {
                if (x >= 3) {
                    rawcmd = "PRIVMSG " + tokens[1] + " :" + subtokstring(tokens, 2, 100, " ");
                    irc_conn->raw(rawcmd.c_str());
                } else {
                    rawcmd = "";
                }
            } else if (tokens[0] == dc + "die") {
                if (x >= 2) {
                    rawcmd = subtokstring(tokens, 1, 100, " ");
                } else {
                    rawcmd = "Quit";
                }
                shouldreconnect = false;
                irc_conn->quit((string(":") + rawcmd).c_str());
                sleep(1);
            } else if ((tokens[0] == dc + "db") && (x >= 2)) {

                if (tokens[1] == "stats") {
                    sprintf(dictsize, "%u", tai->countwords() + 1);
                    sprintf(relcountstr, "%ld", tai->countrels());
                    rawcmd = dictsize;
                    rawcmd = string("database has ") + rawcmd + " words, ";
                    rawcmd += string(relcountstr) + " relations, ";
                    rawcmd += convert<string > (tai->countvrels()) + " associations";
                    irc_conn->privmsg(wheretosend.c_str(), rawcmd.c_str());
                }
            } else if ((tokens[0] == dc + "ai") && (x >= 3)) {
                if (tokens[1] == "order") {
                    aimodel = atol(tokens[2].c_str());
                    irc_conn->privmsg(wheretosend.c_str(),
                            "AI markov model order changed.");
                }
                if (tokens[1] == "permute") {
                    aipermute = atol(tokens[2].c_str());
                    if (aipermute == 0) {
                        aipermute = 1;
                    }
                    tai->setpermute(aipermute);
                    irc_conn->privmsg(wheretosend.c_str(),
                            "AI permutation size changed.");
                }
                if (tokens[1] == "permutations") {
                    unsigned pcount = atol(tokens[2].c_str());
                    if (!pcount) {
                        pcount = TRIP_AI_MAXPERMUTATIONS;
                    }
                    tai->maxpermute(pcount);
                    irc_conn->privmsg(wheretosend.c_str(),
                            (string("Maximum random permutations: ") +
                            convert<string > (pcount)).c_str());
                }
                if (tokens[1] == "random") {
                    if ((tokens[2] == "on") ||
                            (tokens[2] == "yes") ||
                            (tokens[2] == "1") ||
                            (tokens[2] == "true")) {
                        tai->useRandom = true;
                        irc_conn->privmsg(wheretosend.c_str(),
                                "Using random permutations.");
                    } else {
                        tai->useRandom = false;
                        irc_conn->privmsg(wheretosend.c_str(),
                                "Using all permutations (slow).");
                    }
                }
                if (tokens[1] == "connect") {
                    if (x > 3) {
                        string temp_keys = subtokstring(tokens, 2, 100, " ");
                        tai->setdatastring(temp_keys);
                        //tai->setdatastring(tokens[2] + " " + tokens[3]);
                        //tai->extractkeywords();
                        tai->connectkeywords(aimodel);
                        string res = string("Result(") + temp_keys + string(") :")
                                + tai->getdatastring("(null)", rand());
                        irc_conn->privmsg(wheretosend.c_str(),
                                res.c_str());
                    }
                }

                if (tokens[1] == "associate") {
                    if (x > 2) {
                        tai->setdatastring(subtokstring(tokens, 2, 100, " "));
                        tai->extractkeywords();
                        tai->expandkeywords();
                        string res = tai->getdatastring("(null)", rand());
                        irc_conn->privmsg(wheretosend.c_str(), res.c_str());
                    }
                }

            } else if ((tokens[0] == dc + "sleep") && (x >= 3)) {
                sleepmin = atol(tokens[1].c_str());
                sleepmax = atol(tokens[2].c_str());
                irc_conn->privmsg(wheretosend.c_str(), "Sleep time changed.");
            }
        } else {
#ifdef TRIP_DEBUG
            cout << "Message is regular text..." << endl;
#endif
            if ((tokens[0] == mynick) || (msgtarget[0] != '#')) {
                /* We have something to learn from, and to reply to! */
                /* reply */
                if (msgtarget[0] == '#') {
                    if (x < 2) {
                        return 0;
                    }
                    //nothing more to do here, someone is messing.

                    string::size_type lastPos = msg.find_first_not_of(" :,", 0);
                    //finds nick.
                    lastPos = msg.find_first_of(" :,", lastPos);
                    //finds separator.
                    string::size_type mpos = msg.find_first_not_of(" :,", lastPos);
                    // finds text.
                    rawcmd = msg.substr(mpos, msg.size());
                } else {
                    rawcmd = msg;
                }
                string oldrawcmd = rawcmd;
                if (tokens[0].find_first_of("\001") == string::npos
                        || tokens[0] == "\001action") {
#ifdef TRIP_DEBUG
                    cout << "Asked for reply, replying..." << endl;
#endif
                    tai->setdatastring(rawcmd);
                    tai->extractkeywords();
                    tai->expandkeywords();
                    tai->connectkeywords(aimodel);
                    rawcmd = tai->getdatastring(wheretosend, time(0));
#ifdef TRIP_DEBUG
                    cout << "Call to AI module completed" << endl;
#endif
                    if (sleepmax) {
                        sleep(sleepmin + rand() % (abs(sleepmax - sleepmin) + 1));
                    }
                    if (rawcmd != "") {
                        vector<string> repTokens;
                        tokenize(rawcmd, repTokens, " ");
                        if ((tokens[0] == mynick) && (msgtarget[0] == '#')) {
                            rawcmd = rawcmd.substr(1);
                            if (repTokens.size() > 0 && repTokens[0] != "\001action") {
                                rawcmd = rnick + string(": ") + rawcmd;
                            }
                            irc_conn->privmsg(msgtarget.c_str(), rawcmd.c_str());
                        } else {
                            irc_conn->privmsg(rnick.c_str(),
                                    rawcmd.substr(1).c_str());
                        }
                    }
                    rawcmd = oldrawcmd;
                }
            } else {
                string::size_type mpos = msg.find_first_not_of(" :", 0); //finds text.
                rawcmd = msg.substr(mpos, msg.size());
            }

#ifdef TRIP_DEBUG
            cout << "Logging message..." << endl;
#endif
            logfile.open("log.txt", ios::app);
            logfile << ":: " << time(0) << " " << wheretosend << " "
                    << string(hostd->nick) << " : ";
            logfile << rawcmd << endl;
            logfile.close();
            //learn, while ignoring CTCPS, but not ignoring actions.
            if (tokens[0].find_first_of("\001") == string::npos || tokens[0] == "\001action") {
#ifdef TRIP_DEBUG
                cout << "Learning from message..." << endl;
#endif

                tai->setdatastring(rawcmd);
                tai->learndatastring(hostd->nick, wheretosend, time(0));
            } // end of normal text
        } // end of non-commands
    } // end of at-least 1-token text.
#ifdef TRIP_DEBUG
    cout << "Processing complete." << endl;
#endif
    return 0;
}

/* ****************************
 * ADMIN AND SETTINGS SECTION *
 * *************************** */
void readadmins() {
    ifstream adminfile("admins.dat");
    long int i;
    std::string line;

    i = 0;
    if (adminfile.is_open()) {
        while (!adminfile.eof()) {
            getline(adminfile, line);
            if (line != "") {
                admins.userhosts.push_back(line);
                i++;
            }
        }
        adminfile.close();
    }

    cout << i << " admins in list" << endl;
}

int admin_auth(const std::string& strhost) {
    Wildcard matcher;
    unsigned int i;
    int x;
    i = 0;
    x = 0;
    for (i = 0; i < admins.userhosts.size(); i++) {
        if (matcher.wildcardfit(admins.userhosts[i].c_str(), strhost.c_str()))
            x = 1;
    }
    return x;
}

void readignores() {
    ifstream adminfile("ignores.dat");
    long int i;
    std::string line;

    i = 0;
    if (adminfile.is_open()) {
        while (!adminfile.eof()) {
            getline(adminfile, line);
            if (line != "") {
                ignores.userhosts.push_back(line);
                i++;
            }
        }
        adminfile.close();
    }

    cout << i << " ignores in list" << endl;
}

int check_ignore(const std::string& strhost) {
    Wildcard matcher;
    unsigned int i;
    int x;
    i = 0;
    x = 0;

    for (i = 0; i < ignores.userhosts.size(); i++) {
        if (matcher.wildcardfit(ignores.userhosts[i].c_str(), strhost.c_str()))
            x = 1;
    }
    return x;

}

int readsettings(string confFile) {
    ifstream setts(confFile.c_str());
    std::string strsetting;
    defport = 6666;
    while (!(setts.eof())) {
        setts >> strsetting;
        if (strsetting == "server") {
            setts >> server >> defport;
        } else if (strsetting == "nick") {
            setts >> defnick;
        } else if (strsetting == "ident") {
            setts >> defuser;
        } else if (strsetting == "name") {
            getline(setts, defname);
        } else if (strsetting == "chan") {
            getline(setts, strsetting);
            tokenize(strsetting, defchans, " \n");
            //setts >> defchan >> keychan;
        } else if (strsetting == "char") {
            setts >> defchar;
        } else if (strsetting == "pass") {
            setts >> serverpass;
        } else if (strsetting == "sleep") {
            setts >> sleepmin >> sleepmax;
        }
    }
    return 1;
}
