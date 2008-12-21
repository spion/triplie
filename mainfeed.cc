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
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "wildcard/wildcards.cpp"
#include "ai/tokens.h"
#include "ai/ai.hpp"
#include <cctype>
#include <iostream>

#include <boost/regex.hpp>

using namespace std;

AI tai("botdata/triplie.db");

/* ---------------- */

/* ****************************************
    The code.
 * ************************************* */

int main(int argc, char** argv) {
	if (argc < 2) {
		cerr << "Usage " << argv[0] << " <regular_expression>" << endl;
		return 1;
	}
	string theline;
    theline="";
	/* Displaying a banner with info... */
	cout << "Triple AI bot started" << endl
	     << "Eating data from stdin..." << endl;
	unsigned long i = 0;
	string mstr;
	for (int k = 1; k < argc; ++k) {
		mstr += argv[k];
	}
	tai.UnsafeFastMode();
	const boost::regex e(mstr);
	cout << "Matching " << mstr << endl;
	boost::smatch what;
	while (cin && !cin.eof()) {
		getline(cin,theline);
		for(unsigned x=0;x<theline.size();x++) { theline[x]=tolower(theline[x]); }
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
					tokenize(happen_time,time_tokens,": ");
					unsigned multipliers[] = {1, 60, 60, 24, 0};
					for (unsigned j = time_tokens.size() - 1, jj=0; 
						 (j >= 0) && (multipliers[jj]); --j, ++jj)
					{
						unsigned in_seconds = convert<unsigned>(time_tokens[j]);
						unsigned backjj = jj;
						while (backjj) 
						{ 
							in_seconds *= multipliers[backjj--];
						}
						happen_time_sec += in_seconds;
					}
				}
				/*
				cout << happen_time_sec << "::"
					 << happen_where << "::"
					 << happen_who << ":::"
					 << happen_theline << endl;
				*/
   				tai.setdatastring(happen_theline);
				tai.learndatastring(happen_who, happen_where, happen_time_sec);
			}
		}
		i = ((i+1) % 100);
		if (i == 0) {
			cout << "+";
			cout.flush();
		}
	}
	cout << endl << "Done eating." << endl;
	tai.savealldata("botdata");
	cout << "Bye bye." << endl;
    return 0;
}
