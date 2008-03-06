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
#include "ai/tokens.h"
#include "ai/ai.hpp"
#include <cctype>
#include <fstream>

//#include <cstdlib>


using namespace std;

IRC conn;
AI tai;
CAdminList admins;
CAdminList ignores;
unsigned int aimodel;
unsigned int aipermute;

string server, defchan, defnick, defuser, defname, defchar;
vector<string> defchans;
unsigned int defport;
bool shouldreconnect;


/* ---------------- */
int  procprivm(char* params, irc_reply_data* hostd, void* conn);
int  end_of_motd(char* params, irc_reply_data* hostd, void* conn);
void forktobg();
void readadmins();
void readignores();
int  admin_auth(const std::string& strhost);
int  check_ignore(const std::string& strhost);

void cmd_thread(void* irc_conn);
int  readsettings();
void signal_handler(int sig);




/* ****************************************
    The code.
 * ************************************* */
void signal_handler(int sig)
{
	switch(sig) {
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
	defchar="!";
	shouldreconnect=true;

	/* Displaying a banner with info... */
	cout << "Triple AI bot started" << endl
	     << "Admin database@'admins.dat' words@'word.dat' rels@'rels.dat'" << endl;

	readsettings();
	cout << "Server " << server << ":" << defport << endl;
	cout << "Nickname: " << defnick << " Ident: " << defuser << endl;
	cout << "Default channel: " << defchan << endl;
	//* this might need another seed in WIN32 *//
	srand(time(0));

	readadmins();
	readignores();
	tai.readalldata("botdata");
	cout << tai.countwords() << " words, ";
	cout << tai.countrels() << " relations known." << endl;

	#ifdef TRIP_DEBUG
	cout << "Running in debug mode..." << endl;
	#else
	/* fork to background and prevent running twice */
	forktobg();
	#endif
	aimodel = 2;
	tai.setpermute(1);
	/* start IRC session */
	#ifdef TRIP_DEBUG
	cout << "Hooking irc commands to functions..." << endl;
	#endif
	conn.hook_irc_command("PRIVMSG", &procprivm);
	conn.hook_irc_command("376", &end_of_motd);
	#ifdef TRIP_DEBUG
	cout << "Connecting to the server..." << endl;
	#endif
	conn.start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), "");
	#ifdef TRIP_DEBUG
	cout << "Entering message loop..." << endl;
	#endif
	while (shouldreconnect) {
		conn.message_loop();
		if (shouldreconnect) {
			sleep(61);
			conn.disconnect();
			#ifdef TRIP_DEBUG
			cout << "Reconnecting..." << endl;
			#endif
			conn.start(server.c_str(), defport, defnick.c_str(), defuser.c_str(), defname.c_str(), "");
		}
	}
	tai.savealldata(string("botdata"));
	#ifdef TRIP_DEBUG
	cout << "Loop exit, triplie shutdown." << endl;
	#endif
    return 0;
}

void forktobg() {
	int i,lfp;
	char fstr[10];
	i=fork();
	if (i<0) exit(1); /* fork error */
	if (i>0) exit(0); /* parent exits */
	setsid();
	for (i=getdtablesize();i>=0;--i) close(i);
	i=open("/dev/null",O_RDWR); /* open stdin */
	dup(i); /* stdout */
	dup(i); /* stderr */
	umask(027);
	lfp=open("triplie.lock",O_RDWR|O_CREAT,0640);
	if (lfp<0) exit(1); /* can not open */
	if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
	/* only first instance continues */
	sprintf(fstr,"%d\n",getpid());
	write(lfp,fstr,strlen(fstr)); /* record pid to lockfile */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
}


/* ***************
 * IRC HOOKS     *
 * ***************/

int end_of_motd(char* params, irc_reply_data* hostd, void* conn) {
		IRC* irc_conn=(IRC*)conn;
		vector<string>::iterator it;
		for (it=defchans.begin(); it!=defchans.end(); it++)
		{
			irc_conn->join(it->c_str());
		}

		return 0;
}

int procprivm(char* params, irc_reply_data* hostd, void* conn)
{
	Wildcard wildmatch;
    string msg,userhost,msgtarget,rawcmd,mynick,rnick,stupidtmp;

    unsigned long int x;// long int i; long int j; long int litmp;
	int isadmin;

    vector<string> tokens;
	vector<string> tuplets;

    IRC* irc_conn=(IRC*)conn;

	char dictsize[10];
	char relcountstr[15];

	/* code */
	sprintf(dictsize,"%u",tai.countwords()+1);
	sprintf(relcountstr,"%ld",tai.countrels());

    msg = string(params);

	rnick = hostd->nick;
	userhost = rnick;
	userhost += "!" + string(hostd->ident);
	userhost += "@" + string(hostd->host);

	msgtarget = hostd->target;

	mynick = irc_conn->current_nick();

	for(x=0;x<msg.size();x++) { msg[x]=tolower(msg[x]); }

    tokenize(msg,tokens," ,:");
	isadmin = admin_auth(userhost);
	if (check_ignore(userhost)) return 0;
	rawcmd="";
    x = (long int) tokens.size();
    if (x >= 1) {
		string dc=defchar;
		string stupidtmp = dc + "*";
		/* lets see if we have a command here... from the admin. */
		if  (    (wildmatch.wildcardfit(stupidtmp.c_str(), tokens[0].c_str()))
			  && (isadmin)
		    )
		{
			/* command to join/part channel */

        	if (tokens[0] == dc+"join") { if (x>=2) irc_conn->join(tokens[1].c_str()); }
			else if (tokens[0] == dc+"part") { if (x>=2) irc_conn->part(tokens[1].c_str()); }
			else if (tokens[0] == dc+"op") {
				irc_conn->mode(msgtarget.c_str(),"+oooooo", subtokstring(tokens,1,6," ").c_str());
			}
			else if (tokens[0] == dc+"deop") {
				irc_conn->mode(msgtarget.c_str(),"-oooooo", subtokstring(tokens,1,6," ").c_str());
			}
			else if (tokens[0] == dc+"vo") {
				irc_conn->mode(msgtarget.c_str(),"+vvvvvv", subtokstring(tokens,1,6," ").c_str());
			}
			else if (tokens[0] == dc+"devo") {
				irc_conn->mode(msgtarget.c_str(),"-vvvvvv", subtokstring(tokens,1,6," ").c_str());
			}
			else if (tokens[0] == dc+"topic") {
				rawcmd = "TOPIC " + msgtarget + " :" + subtokstring(tokens,1,100," ");
				irc_conn->raw(rawcmd.c_str());
			}
			else if (tokens[0] == dc+"quit") {
				if (x>=2) {
					rawcmd = subtokstring(tokens,1,100," ");
				}
				else { rawcmd = "Quit"; }
				irc_conn->quit(rawcmd.c_str());
			}
			else if (tokens[0] == dc+"die") {
				if (x>=2) {
					rawcmd = subtokstring(tokens,1,100," ");
				}
				else { rawcmd = "Quit"; }
				shouldreconnect=false;
				irc_conn->quit(rawcmd.c_str());
			}
			else if ((tokens[0] == dc+"dict") && (x>=2)) {

				if (tokens[1] == "words") {
						//itoa(dictind,rawcmd,10);
						rawcmd = dictsize;
						rawcmd=string("my knowledge is ")+rawcmd+" words";
						irc_conn->privmsg(msgtarget.c_str(),rawcmd.c_str());
				}
				if (tokens[1] == "rels") {
						rawcmd = relcountstr;
						rawcmd=string("i know of ")+rawcmd+" relations";
						irc_conn->privmsg(msgtarget.c_str(),rawcmd.c_str());
				}
				if (tokens[1] == "save") {
						rawcmd=string("saving my words and relations.");
						tai.savealldata("botdata");
						irc_conn->privmsg(msgtarget.c_str(),rawcmd.c_str());
				}
			}
			else if ((tokens[0] == dc+"ai") && (x>=3)) {
				if (tokens[1] == "order") {
					aimodel = atol(tokens[2].c_str());
					irc_conn->privmsg(msgtarget.c_str(),
									  "AI markov model order changed.");
				}
				if (tokens[1] == "permute") {
					aipermute = atol(tokens[2].c_str());
					tai.setpermute(aipermute);
					irc_conn->privmsg(msgtarget.c_str(),"AI permute toggled.");
				}
			}
		}
		else 
		{
			if (tokens[0] == mynick)  {
				/* We have something to learn from, and to reply to! */
				/* reply */
				if (x<2) { return 0; } //nothing more to do here, someone is messing.

				string::size_type lastPos = msg.find_first_not_of(" :,", 0);
				//finds nick.
    			lastPos                   = msg.find_first_of(" :,", lastPos);
    			//finds separator.
				string::size_type mpos = msg.find_first_not_of(" :,", lastPos);
				// finds text.
				rawcmd = msg.substr(mpos,msg.size());
				tai.setdatastring(rawcmd);
				tai.extractkeywords();
				tai.expandkeywords();

				tai.connectkeywords(aimodel);
				rawcmd = tai.getdatastring();
				if (rawcmd == "") { rawcmd = " *shrug*"; }
				rawcmd = rnick+string(":")+rawcmd; //!!!
				irc_conn->privmsg(msgtarget.c_str(),rawcmd.c_str());
				rawcmd = msg.substr(mpos,msg.size());
			}
			else {
				string::size_type mpos = msg.find_first_not_of(" :", 0); //finds text.
				rawcmd = msg.substr(mpos,msg.size());
			}
			//learn
			tai.setdatastring(rawcmd);
			tai.learndatastring(hostd->nick);
		} // end of normal text
    }
	return 0;
}

/* ****************************
 * ADMIN AND SETTINGS SECTION *
 * *************************** */
void readadmins() {
	ifstream adminfile ("admins.dat");
	long int i;
	std::string line;

	i=0;
	if (adminfile.is_open())
		{
		while (! adminfile.eof() ) {
			getline (adminfile,line);
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
	unsigned int i; int x;
	i=0; x=0;
	for (i=0;i<admins.userhosts.size();i++) {
		if (matcher.wildcardfit(admins.userhosts[i].c_str(),strhost.c_str()))
			x=1;
	}
	return x;
}
void readignores() {
	ifstream adminfile ("ignores.dat");
	long int i;
	std::string line;

	i=0;
	if (adminfile.is_open())
		{
		while (! adminfile.eof() ) {
			getline (adminfile,line);
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
	unsigned int i; int x;
	i=0; x=0;

	for (i=0;i<ignores.userhosts.size();i++) {
		if (matcher.wildcardfit(ignores.userhosts[i].c_str(),strhost.c_str()))
			x=1;
	}
	return x;

}

int readsettings() {
	ifstream setts ("triplie.conf");
	std::string strsetting;
	defport=6666;
	while (!(setts.eof())) {
		setts >> strsetting;
		if (strsetting == "server") {
			setts >> server >> defport;
		}
		else if (strsetting == "nick") {
			setts >> defnick;
		}
		else if (strsetting == "ident") {
			setts >> defuser;
		}
		else if (strsetting == "name") {
			getline(setts,defname);
		}
		else if (strsetting == "chan") {
			getline(setts,strsetting);
			tokenize(strsetting,defchans," \n");
			//setts >> defchan >> keychan;
		}
		else if (strsetting == "char") {
			setts >> defchar;
		}
	}
	return 1;
}
