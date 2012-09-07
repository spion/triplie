/*
 *  Copyright (C) Gorgi Kosev a.k.a. Spion, John Peterson.
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
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <getopt.h>
#include <math.h>
#include <queue>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
//#define TRIP_DEBUG
#ifdef TRIP_DEBUG
#define LOG(...) log(__VA_ARGS__)
#else
#define LOG
#endif
#include "ai/ai.hpp"
#include "common.h"
#include "irc/IRC.h"
#include "wildcard/wildcards.cpp"
#define TRIP_DISTRUBUTED 1
using namespace std;

IRC* conn;
AI* tai = 0; //("botdata/triplie.db");
CAdminList admins;
CAdminList ignores;
Wildcard wildmatch;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
pthread_t thread_r, thread_s;
ofstream logfile;
unsigned int aimodel;
unsigned int aipermute;
double last_reply = 0;
queue<double> msg_time;
vector<string> defchans;
string confFile = "triplie.conf", db = "botdata/triplie.db", server, serverpass = "", defnick, defuser, defname, defchar = "!";
u32 defport = 6667, sleepmin = 0, sleepmax = 0, partake_rate = 0, partake_lim = 5, speak_min = 0, speak_max = 0;
bool shouldreconnect, ai = true, job = false, ramdb = false, mute = false, running = true, message = false;

void forktobg(string);
void readadmins();
void readignores();
int admin_auth(const std::string& strhost);
int check_ignore(const std::string& strhost);
void cmd_thread(void* irc_conn);
int readsettings(string);
void signal_handler(int sig);
void close();

void sleep_lim() {
	range_lim(sleepmin, u32(0));
	range_lim(sleepmax, u32(0));
}
void partake_lim_() {
	range_lim(partake_rate, u32(0), u32(100));
}
void speak_lim() {
	if (speak_min) range_lim(speak_min, u32(5));
	if (speak_max) range_lim(speak_max, u32(5));
}

int opt[] = {'h', 'V', 'j', 'v', 'c', 'A', 'd', 'r', 'a', 'i', 's', 'p', 'n', 'I', 'N', 'C', 'o', 'S', 'P', 'e', 'm'};
string args = "hVjv:c:Ad:ra:i:s:p:n:I:N:C:o:S:P:e:m";
string optl[] = {"help", "version", "job", "verbosity", "config", "noai", "database", "ramdb", "admin", "ignore", "server", "pass", "nick", "ident", "name", "channel", "command", "sleep", "partake", "speak", "message"};
void usage() {
	fprintf(stdout, "Usage: triplie [-h] [-V] [-j] [-A] [-r] [-v verbosity] [-c config] [-d database] [-a admin(s)] [-i ignore(s)] [-s server[:port]] [-p pass] [-n nick] [-I ident] [-N name] [-C channel(s)] [-o char] [-S lo hi] [-P rate lim] [-e lo hi]\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay this message.\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay version.\n"
		"\t\033[1m-%c, --%s\033[0m\trun process in background.\n"
		"\t\033[1m-%c, --%s\033[0m\tlogging verbosity. \033[1;30mdefault %u.\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tconfig file. \033[1;30moverride arguments before it. ex: '%s' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tdisable AI.\n"
		"\t\033[1m-%c, --%s\033[0m\tdatabase file. \033[1;30mex: '%s' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tuse ram database.\n"
		"\t\033[1m-%c, --%s\033[0m\tadmin(s). \033[1;30mex: 'u@s u@s'.\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tignore(s). \033[1;30mex: 'u@s u@s'.\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tserver:port. \033[1;30mex: 's:%u' (default port).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tserver password.\n"
		"\t\033[1m-%c, --%s\033[0m\tnickname.\n"
		"\t\033[1m-%c, --%s\033[0m\tident name.\n"
		"\t\033[1m-%c, --%s\033[0m\tname.\n"
		"\t\033[1m-%c, --%s\033[0m\tchannel name(s). \033[1;30mex: '#c #c'.\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tcommand character. \033[1;30mex: '%s' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tsleep range before reply. \033[1;30mex: '%u %u' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tunsolicited reply percentage and limit. \033[1;30mlimit is the total channel message count during the past minute. ex: '%u %u' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tunsolicited speak limit range. \033[1;30mlimit is the total channel message count during the past minute. selected randomly from within range. ex: '%u %u' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tmessage on join, part, quit.\n",		
		opt[0], optl[0].c_str(),
		opt[1], optl[1].c_str(),
		opt[2], optl[2].c_str(),
		opt[3], optl[3].c_str(), verb,
		opt[4], optl[4].c_str(), confFile.c_str(),
		opt[5], optl[5].c_str(),
		opt[6], optl[6].c_str(), db.c_str(),
		opt[7], optl[7].c_str(),
		opt[8], optl[8].c_str(),
		opt[9], optl[9].c_str(),
		opt[10], optl[10].c_str(), defport,
		opt[11], optl[11].c_str(),
		opt[12], optl[12].c_str(),
		opt[13], optl[13].c_str(),
		opt[14], optl[14].c_str(),
		opt[15], optl[15].c_str(),
		opt[16], optl[16].c_str(), defchar.c_str(),
		opt[17], optl[17].c_str(), sleepmin, sleepmax,
		opt[18], optl[18].c_str(), partake_rate, partake_lim,
		opt[19], optl[19].c_str(), speak_min, speak_max,
		opt[20], optl[20].c_str());
}
bool get_arg(int argc, char** argv) {
	u32 i = 0;
	struct option longopts[] = {
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument ,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument ,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument,	NULL,	opt[i++]},
		{NULL,				0,					NULL,	0}
	};
	int c;
	vector<string> v_tmp;
	while ((c = getopt_long(argc, argv, args.c_str(), longopts, 0)) != -1) {
		switch (c) {
		case 'V':
			log("%s %s\n", VERDATE, VER);
			return true;
		case 'h':
			usage();
			return true;
		case 'j':
			job = true;
			break;
		case 'v':
			verb = atoi(optarg);
			break;
		case 'c':
			confFile = optarg;
			break;
		case 'A':
			ai = false;
			break;
		case 'd':
			db = optarg;
			break;
		case 'r':
			ramdb = true;
			break;
		case 'a':
			admins.userhosts.clear();
			tokenize(optarg, admins.userhosts, " ");
			break;
		case 'i':
			ignores.userhosts.clear();
			tokenize(optarg, ignores.userhosts, " ");
			break;
		case 's':
			v_tmp.clear();
			tokenize(optarg, v_tmp, ":");
			if (v_tmp.size() > 0) server = v_tmp[0];
			if (v_tmp.size() > 1) defport = atoi(v_tmp[1].c_str());
			break;
		case 'p':
			serverpass = optarg;
			break;
		case 'n':
			defnick = optarg;
			break;
		case 'I':
			defuser = optarg;
			break;
		case 'N':
			defname = optarg;
			break;
		case 'C':
			defchans.clear();
			tokenize(optarg, defchans, " ");
			break;
		case 'o':
			defchar = optarg;
			break;
		case 'S':
			v_tmp.clear();
			tokenize(optarg, v_tmp, " ");
			if (v_tmp.size() > 0) sleepmin = atoi(v_tmp[0].c_str());
			if (v_tmp.size() > 1) sleepmax = atoi(v_tmp[1].c_str());
			sleep_lim();
			break;
		case 'P':
			v_tmp.clear();
			tokenize(optarg, v_tmp, " ");
			if (v_tmp.size() > 0) partake_rate = atoi(v_tmp[0].c_str());
			if (v_tmp.size() > 1) partake_lim = atoi(v_tmp[1].c_str());
			partake_lim_();
			break;
		case 'e':
			v_tmp.clear();
			tokenize(optarg, v_tmp, " ");
			if (v_tmp.size() > 0) speak_min = atoi(v_tmp[0].c_str());
			if (v_tmp.size() > 1) speak_max = atoi(v_tmp[1].c_str());
			speak_lim();
			break;
		case 'm':
			message = true;
			break;
		default:
			fprintf(stderr, "unknown option: %c\n", c);
			break;
		}
	}
	return false;
}

void signal_handler(int sig, siginfo_t *info, void *ptr) {
    switch (sig) {
        case SIGHUP:
            //log_message(LOG_FILE,"hangup signal catched");
            break;
        case SIGINT:
            break;
        case SIGTERM:
            //log_message(LOG_FILE,"terminate signal catched");
            break;
	    case SIGCHLD: /* ignore child */
		case SIGTSTP: /* ignore tty signals */
		case SIGTTIN:
		case SIGTTOU:
            signal(sig, SIG_IGN);
            return;
    }
	log("Received signal %d ...\n", sig);
	close();
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}

void log_speak(const char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	fprintf(stderr, "\033[1;30m%s \033[1;34m%s\033[0m", time_hm().c_str(), buf);
	va_end(l);
}
void cmdreply(string wheretosend, const char* f, ...) {
	const int len = 0xfff;
	char buf[len];
	va_list l;
	va_start(l, f);
	vsnprintf(buf, len, f, l);
	va_end(l);
	if (wheretosend.empty())
		fprintf(stderr, "\033[1;30m%s\033[0m\n", buf);
	else
		conn->privmsg(wheretosend.c_str(), buf);
}

u32 msg_count(queue<double>& q, double t) {
	while(q.front() < t && !q.empty()) {
		q.pop();
	}
	return q.size();
}

bool reply_time() {
	return last_reply < seconds()-10*1000;
}

void close() {
	if (!running) return;
	running = false;
	shouldreconnect = false;
	if (conn) conn->disconnect();
	if (tai) {
		tai->SendAllSlavesAndWait("06 \n");
		tai->savealldata();
		tai->CloseDB();
	}
}

void proccmd(string& rawcmd, string msg, string msgtarget, string wheretosend) {
	IRC *irc_conn = (IRC*)conn;
	vector<string> tokens;
	tokenize(msg, tokens, " ,:");
	u32 x = tokens.size();
	string dc = defchar;
	bool change = false, reply_match = wildmatch.wildcardfit(string(dc + dc + "*").c_str(), tokens[0].c_str());
	if (reply_match) tokens[0] = tokens[0].substr(1);
	bool reply = wheretosend.empty() || reply_match;
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
	} else if (tokens[0] == dc + "sleep") {
		change = x >= 3;
		if (change) {
			sleepmin = atol(tokens[1].c_str());
			sleepmax = atol(tokens[2].c_str());
			sleep_lim();
		}
		if (reply) cmdreply(wheretosend, "sleep %u %u", sleepmin, sleepmax);
	} else if (tokens[0] == dc + "mute") {
		change = x >= 2;
		if (change)
			mute = atoi(tokens[1].c_str()) > 0;
		if (reply) cmdreply(wheretosend, "mute %d", mute);
	} else if (tokens[0] == dc + "partake") {
		change = x >= 3;
		if (change) {
			partake_rate = atoi(tokens[1].c_str());
			partake_lim = atoi(tokens[2].c_str());
			partake_lim_();
		}
		if (reply) cmdreply(wheretosend, "partake %u %u", partake_rate, partake_lim);
	} else if (tokens[0] == dc + "speak") {
		change = x >= 3;
		if (change) {
			speak_min = atoi(tokens[1].c_str());
			speak_max = atoi(tokens[2].c_str());
			speak_lim();
		}
		if (reply) cmdreply(wheretosend, "speak %u %u", speak_min, speak_max);
	} else if (tokens[0] == dc + "message") {
		change = x >= 2;
		if (change)
			message = atoi(tokens[1].c_str()) > 0;
		if (reply) cmdreply(wheretosend, "message %d", message);
	} else if (tai && msg == dc + "db stats") {
			cmdreply(wheretosend, "%'u words, %'u relations, %'u associations in database", tai->countwords()+1, tai->countrels(), tai->countvrels());
	} else if (tai && tokens[0] == dc + "ai") {
		change = x >= 3;
		if (tokens[1] == "order") {
			if (change) aimodel = atol(tokens[2].c_str());
			if (reply) cmdreply(wheretosend, "AI markov model order %d", aimodel);
		}
		if (tokens[1] == "permute") {
			if (change) {
				aipermute = atol(tokens[2].c_str());
				if (aipermute == 0) aipermute = 1;
				tai->setpermute(aipermute);
			}
			if (reply) cmdreply(wheretosend, "AI permutation size %d", aipermute);
		}
		if (tokens[1] == "permutations") {
			if (change) {
				unsigned pcount = atol(tokens[2].c_str());
				if (!pcount) pcount = TRIP_AI_MAXPERMUTATIONS;
				tai->maxpermute(pcount);
			}
			if (reply) cmdreply(wheretosend, "maximum random permutations %u", tai->getmaxpermute());
		}
		if (tokens[1] == "random") {
			if (change) {
				if ((tokens[2] == "on") ||
						(tokens[2] == "yes") ||
						(tokens[2] == "1") ||
						(tokens[2] == "true")) {
					tai->useRandom = true;
				} else
					tai->useRandom = false;				
			}
			if (reply) cmdreply(wheretosend, "using %s.", tai->useRandom ? "random permutations" : "all permutations (slow)");
		}
		if (tokens[1] == "connect") {
			change = x > 3;
			if (change) {
				string temp_keys = subtokstring(tokens, 2, 100, " ");
				tai->setdatastring(temp_keys);
				//tai->setdatastring(tokens[2] + " " + tokens[3]);
				//tai->extractkeywords();
				tai->connectkeywords(aimodel);
				if (reply) cmdreply(wheretosend, "result(%s) :%s", temp_keys.c_str(), tai->getdatastring("(null)", rand()).c_str());
			}
		}
		if (tokens[1] == "associate") {
			change = x > 2;
			if (change) {
				tai->setdatastring(subtokstring(tokens, 2, 100, " "));
				tai->extractkeywords();
				tai->expandkeywords();	
				if (reply) cmdreply(wheretosend, "%s", tai->getdatastring("(null)", rand()).c_str());				
			}
		}
	}
}
void proccmd(string msg, string msgtarget, string wheretosend) {
	string rawcmd;
	proccmd(rawcmd, msg, msgtarget, wheretosend);
}

void* reader(void*) {
    char c;
	string d;
    while(running) {
		c = fgetc(stdin);
		if(c < 0) { sleep(1); continue; }
		if (c == 0x0a) {
			if(conn) {
				if (wildmatch.wildcardfit(string(defchar + "*").c_str(), d.c_str()))
					proccmd(d, defchans[0], "");
				else
					conn->privmsg(defchans[0].c_str(), d.c_str());
			}
			d.clear();
		} else
			d.push_back(c);
    }
	pthread_exit(NULL);
	return NULL;
}

void* speak(void*) {
	string m = "";
	u32 speak_s = 0;
	sleep((speak_min + speak_max)/2);
    while(running) {
		sleep(1);
		if (!tai || mute || (!speak_min && !speak_max)) continue;	
		if (!msg_count(msg_time, seconds()-speak_s*1000)) {
			pthread_mutex_lock(&count_mutex);
			m = tai->getspeak();
			pthread_mutex_unlock(&count_mutex);
			if (!m.empty()) {
				log_speak("%s: <%s> %s\n", defchans[0].c_str(), defnick.c_str(), m.c_str());
				msg_time.push(seconds());
				if (conn) conn->privmsg(defchans[0].c_str(), m.c_str());
				speak_s = speak_min + rand() % (abs(speak_max - speak_min) + 1);	
			}
		}		
    }
	pthread_exit(NULL);
	return NULL;
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

void logmsg(s64 t, string wheretosend, string nick, string rawcmd) {
	logfile.open("triplie.log", ios::app);
	logfile << ":: " << t << " " << wheretosend << " " << nick << " : " << rawcmd << endl;
	logfile.close();
}

int procjoin(char* params, irc_reply_data* hostd, void* conn) {
	if (!message || hostd->nick == defnick) return 0;
	string m = "welcome" + tai->getspeak("welcome", 5);
	if (params[0] == ':') params++;
	if (params[0] != '#') return 0;
	m = format("%s: %s", hostd->nick, m.c_str());
	log_speak("%s: %s\n", (const char*)params, m.c_str());
	((IRC*)conn)->privmsg((const char*)params, m.c_str());
	return 0;
}
int procpart(char* params, irc_reply_data* hostd, void* conn) {
	if (!message) return 0;
	string m = "bye" + tai->getspeak("bye", 5);
	if (params[0] == ':') params++;
	if (strchr(params, ' '))
		*strchr(params, ' ') = '\0';
	if (params[0] != '#') return 0;
	m = format("%s: %s", hostd->nick, m.c_str());
	log_speak("%s: %s\n", (const char*)params, m.c_str());
	((IRC*)conn)->privmsg((const char*)params, m.c_str());
	return 0;
}
int procquit(char* params, irc_reply_data* hostd, void* conn) {
	if (!message) return 0;
	string m = "bye" + tai->getspeak("bye", 5);
	if (params[0] == ':') params++;
	if (params[0] != '#') return 0;
	m = format("%s: %s", hostd->nick, m.c_str());
	log_speak("%s\n", m.c_str());
	((IRC*)conn)->privmsg(defchans[0].c_str(), m.c_str());
	return 0;
}

int procprivm(char* params, irc_reply_data* hostd, void* conn) {
    log2("Processing private message...\n");
    string msg, userhost, msgtarget, rawcmd, mynick, rnick;
    int isadmin;
    vector<string> tokens;
    vector<string> tuplets;
    IRC* irc_conn = (IRC *) conn;
    msg = string(params);
    rnick = hostd->nick;
    userhost = rnick;
    if (hostd->ident) userhost += "!" + string(hostd->ident);
    if (hostd->host) userhost += "@" + string(hostd->host);
    msgtarget = hostd->target;

    string wheretosend;
    if ((msgtarget[0] == '#') || (msgtarget[0] == '&'))
        wheretosend = msgtarget;
    else
        wheretosend = rnick;

    mynick = irc_conn->current_nick();
    log2("Lowercasing data ...\n");
    lowercase(mynick);
    lowercase(msg);

    tokenize(msg, tokens, " ,:");
    isadmin = admin_auth(userhost);
    if (check_ignore(userhost)) {
		logmsg(time(0), wheretosend, hostd->nick, msg);
		return 0;
	}
    log2("Processing private message text ...\n");
    rawcmd = "";
    if (tokens.size() >= 1) {
        log2("Message has tokens ...\n");
        /* lets see if we have a command here... from the admin. */
        if ((wildmatch.wildcardfit(string(defchar + "*").c_str(), tokens[0].c_str()))
                && (isadmin)
                ) {
			proccmd(rawcmd, msg.substr(1), msgtarget, wheretosend);
        } else {
            log2("Message is regular text...");
			bool privMsg = msgtarget[0] != '#';
			if (!privMsg) msg_time.push(seconds());
			bool addressMsg = find_in_words(msg, mynick) || privMsg;
			boost::algorithm::trim(msg);
			if (msg.empty()) {
				LOG("Message is empty ...\n");
				return 0;
			}
			bool partake = (rand()%100+1) <= partake_rate && (msg_count(msg_time, seconds()-60*1000) <= partake_lim || !partake_lim);
			if (tai && !mute && reply_time() && (addressMsg || partake)) {
                /* We have something to learn from, and to reply to! */
                /* reply */
                rawcmd = msg; 
                string oldrawcmd = rawcmd;
                if (tokens[0].find_first_of("\001") == string::npos || tokens[0] == "\001action") {
					log2("Asked for reply, replying ...\n");
					pthread_mutex_lock(&count_mutex);					
					tai->setdatastring(rawcmd);
					tai->extractkeywords();
					tai->expandkeywords();
					tai->connectkeywords(aimodel);
					rawcmd = tai->getdatastring(wheretosend, time(0));
					pthread_mutex_unlock(&count_mutex);
					log2("Call to AI module completed\n");
					log_speak("%s: <%s> %s <%s> %s\n", msgtarget.c_str(), rnick.c_str(), msg.substr(1).c_str(), mynick.c_str(), rawcmd.c_str());
                    if (sleepmax)
                        sleep(sleepmin + rand() % (abs(sleepmax - sleepmin) + 1));
					if (rawcmd.empty() && addressMsg)
						rawcmd = " I'm unable to find a response to that.";
                    if (rawcmd != "") {
                        vector<string> repTokens;
                        tokenize(rawcmd, repTokens, " ");
                        if (!privMsg) {
                            rawcmd = rawcmd.substr(1);
                            if (addressMsg && repTokens.size() > 0 && repTokens[0] != "\001action") {
                                rawcmd = rnick + string(": ") + rawcmd;
                            }
                            irc_conn->privmsg(msgtarget.c_str(), rawcmd.c_str());
							msg_time.push(seconds());
                        } else {
                            irc_conn->privmsg(rnick.c_str(),
                                    rawcmd.substr(1).c_str());
							last_reply = seconds();
                        }
                    }
                    rawcmd = oldrawcmd;
                }
            } else {
                rawcmd = msg.substr(msg.find_first_not_of(" :", 0), msg.size());
            }

            log2("Logging message ...\n");
			logmsg(time(0), wheretosend, hostd->nick, rawcmd);
            //learn, while ignoring CTCPS, but not ignoring actions.
            if (tai && tokens[0].find_first_of("\001") == string::npos || tokens[0] == "\001action") {
                log2("Learning from message...\n");
				pthread_mutex_lock(&count_mutex);
                tai->setdatastring(rawcmd);
                tai->learndatastring(hostd->nick, wheretosend, time(0));
				pthread_mutex_unlock(&count_mutex);
            } // end of normal text
        } // end of non-commands
    } // end of at-least 1-token text.
    log2("Processing complete.\n");
    return 0;
}

int main(int argc, char** argv) {
    readsettings(confFile);
	readadmins();
    readignores();
	if (get_arg(argc, argv)) return 0;
    if (ai) tai = new AI(db, ramdb);
    conn = new IRC();
    setlocale(LC_ALL, "en_US.utf8");
    shouldreconnect = true;

	cout << admins.userhosts.size() << " admins in list" << endl;
    cout << ignores.userhosts.size() << " ignores in list" << endl;	
	
	// register signal handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = signal_handler;
	sa.sa_flags = SA_SIGINFO;
	for (int i = 1; i <= 30; i++)
		sigaction(i, &sa, NULL);
	
    /* Displaying a banner with info... */
    log2("Debug mode enabled.\n");
    cout << "Triple AI bot started" << endl
		<< "Admin database@'admins.dat' ai db@'" << db << "'" << endl;
    cout << "Server " << server << ":" << defport << endl;
    cout << "Nickname: " << defnick << " Ident: " << defuser << endl;
	cout << "Channel: " << defchans[0] << endl;

    //* this might need another seed in WIN32 *//
    srand(time(0));

    if (tai) {
		tai->readalldata();
		log("%'u words, %'u relations, %'u associations in database.\n", tai->countwords(), tai->countrels(), tai->countvrels());
	}

	// register stdin handler
	pthread_create(&thread_r, NULL, reader, NULL);	
	// register speak handler
	pthread_create(&thread_s, NULL, speak, NULL);	
	
    log2("Running in debug mode...\n");
    /* fork to background and prevent running twice */
	if (job) {
		log("Entering background ...\n");
        if (tai) tai->CloseDB();
        forktobg(confFile);
        if (tai) tai->OpenDB();
	}
    aimodel = 2;
    /* start IRC session */
    log2("Hooking irc commands to functions...\n");
    conn->hook_irc_command((char *) "PRIVMSG", &procprivm);
	conn->hook_irc_command((char *) "JOIN", &procjoin);
	conn->hook_irc_command((char *) "PART", &procpart);
	conn->hook_irc_command((char *) "QUIT", &procquit);
    conn->hook_irc_command((char *) "001", &end_of_motd);
    log2("Connecting to the server...\n");
    conn->start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), serverpass.c_str());
    log2("Entering message loop...\n");
	log("Connected to server.\n");

    if (tai) tai->connect_to_workers("workers.dat");
    while (shouldreconnect) {
        conn->message_loop();
        if (shouldreconnect) {
            sleep(61);
            conn->disconnect();
            log2("Reconnecting...\n");
            conn->start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), serverpass.c_str());
        }
    }
	close();
    log2("Loop exit, triplie shutdown.\n");
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
    if (dup(i) < 0) exit(1); /* stdout */    
    if (dup(i) < 0) exit(1); /* stderr */
#ifdef linux
    int lfp;
    char fstr[10];
    umask(027);
    string lockFile = configFile + ".lock";
    lfp = open(lockFile.c_str(), O_RDWR | O_CREAT, 0640);
    if (lfp < 0) exit(1);
    if (lockf(lfp, F_TLOCK, 0) < 0) exit(0);
    sprintf(fstr, "%d\n", getpid());
    if (write(lfp, fstr, strlen(fstr)) < 0) exit(1);
#endif
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
}

int admin_auth(const std::string& strhost) {
    unsigned int i;
    int x;
    i = 0;
    x = 0;
    for (i = 0; i < admins.userhosts.size(); i++) {
        if (wildmatch.wildcardfit(admins.userhosts[i].c_str(), strhost.c_str()))
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
}

int check_ignore(const std::string& strhost) {
    unsigned int i;
    int x;
    i = 0;
    x = 0;

    for (i = 0; i < ignores.userhosts.size(); i++) {
        if (wildmatch.wildcardfit(ignores.userhosts[i].c_str(), strhost.c_str()))
            x = 1;
    }
    return x;

}

int readsettings(string confFile) {
    ifstream setts(confFile.c_str());
    std::string strsetting;
    while (!(setts.eof())) {
		setts.clear();
        setts >> strsetting;
		if (strsetting == "db") {
            setts >> db;
        } else if (strsetting == "server") {
            setts >> server >> defport;
		} else if (strsetting == "pass") {
            setts >> serverpass;	
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
        } else if (strsetting == "sleep") {
			setts >> sleepmin >> sleepmax;
        } else if (strsetting == "partake") {
			setts >> partake_rate >> partake_lim;
        } else if (strsetting == "speak") {
			setts >> speak_min >> speak_max;
        }
    }
    return 1;
}
