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
#include <boost/regex.hpp>
#include <cctype>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include "ai/ai.hpp"
#include "ai/tokens.h"
#include "common.h"
#include "wildcard/wildcards.cpp"
using namespace std;

AI* tai;
string db = "botdata/triplie.db", r = "::\\s([0-9]+)\\s(\\S+)\\s(\\S+)[\\s\:]+(.+)";
int opt[] = {'h', 'V', 'd', 'r'};
string args = "hVd:r:";
string optl[] = {"help", "version", "database", "regex"};
void usage() {
	fprintf(stdout, "Usage: feedtriplie [-h] [-V] [-d database] -r regex\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay this message.\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay version.\n"
		"\t\033[1m-%c, --%s\033[0m\tdatabase file. \033[1;30mex: '%s' (default).\033[0m\n"
		"\t\033[1m-%c, --%s\033[0m\tinput format as regular expression. \033[1;30mex: '%s' (default).\033[0m\n",		
		opt[0], optl[0].c_str(),
		opt[1], optl[1].c_str(),
		opt[2], optl[2].c_str(), db.c_str(),
		opt[3], optl[3].c_str(), r.c_str());
}
bool get_arg(int argc, char** argv) {
	u32 i = 0;
	struct option longopts[] = {
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument ,	NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument ,	NULL,	opt[i++]},
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
		case 'd':
			db = optarg;
			break;
		case 'r':
			r = optarg;
			break;
		default:
			fprintf(stderr, "unknown option: %c\n", c);
			break;
		}
	}
	return false;
}

int main(int argc, char** argv) {
	if (get_arg(argc, argv)) return 0;
	tai = new AI(db);
	setlocale(LC_ALL, "en_US.utf8");
	string theline;
    theline="";
	/* Displaying a banner with info... */
	cout << "Database " << db << endl
		<< "Triple AI bot started" << endl
	    << "Reading data from stdin ..." << endl;
	u64 i = 0;
	u16 u_interval = 100;
	double time_start = seconds(), time_last = seconds();
	tai->UnsafeFastMode();
	const boost::regex e(r);
	cout << "Matching " << r << endl;
	boost::smatch what;
	while (cin && !cin.eof()) {
		getline(cin,theline);
		lowercase(theline);
		if (boost::regex_match(theline,what,e,boost::match_extra))
		{
			if (what.size() > 3)
			{
				//time, where, who, text
				string happen_time = what[1];
				string happen_where = what[2];
				string happen_who = what[3];
				string happen_theline;
				unsigned happen_time_sec = 0;
				if (what.size() > 4)
				{
					happen_theline = what[4]; 
				}
				else
				{
					happen_where = "(single)";
					happen_who = what[2];
					happen_theline = what[3];
				}
				if (happen_time.find_first_not_of("0123456789") ==
					string::npos)
				{
					happen_time_sec = convert<unsigned>(happen_time);
				}
				else
				{
					vector<string> time_tokens;
					tokenize(happen_time,time_tokens,": -TZ");
					unsigned multipliers[] = {1, 60, 60, 24, 0};
					for (unsigned j = time_tokens.size() - 1, jj=0; 
						 (multipliers[jj]); --j, ++jj)
					{
						unsigned in_seconds = convert<unsigned>(time_tokens[j]);
						unsigned backjj = jj;
						while (backjj) 
						{ 
							in_seconds *= multipliers[backjj--];
						}
						happen_time_sec += in_seconds;
						if (j == 0) break;
					}
				}
				log2("%u::%s::%s::%s\n", happen_time_sec, happen_where.c_str(), happen_who.c_str(), happen_theline.c_str());
				vector<string> tokens;
				tokenize(happen_theline,tokens," ,:");
				if (tokens.size() > 0 && 
					(tokens[0].find_first_of("\001") == string::npos || tokens[0] == "\001action"))
				{
   					tai->setdatastring(happen_theline);
					tai->learndatastring(happen_who, happen_where, happen_time_sec);
				}
				else {
					log2("is a CTCP\n");
				}
			}
		}
		i++;
		if (i%u_interval == 0) {
			log("\r\033[KReading %'llu lines, %'.0f s, %'.0f lines/s ...", i, (seconds()-time_start)/1000, u_interval/((seconds()-time_last)/1000));
			time_last = seconds();
		}
	}
	tai->UnsafeQuery("VACUUM; ANALYZE;");
	cout << endl << "Done reading." << endl;
	tai->savealldata();
	tai->CloseDB();
	cout << "Bye bye." << endl;
    return 0;
}
