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

using namespace std;

AI tai("botdata/triplie.db");

/* ---------------- */

/* ****************************************
    The code.
 * ************************************* */

int main(int argc, char** argv) {
	string theline;
    theline="";
	/* Displaying a banner with info... */
	cout << "Triple AI bot started" << endl
	     << "Eating data from stdin..." << endl;
	tai.setpermute(1);
	unsigned long i = 0;
	while (!cin.eof()) {
		getline(cin,theline);
		for(unsigned x=0;x<theline.size();x++) { theline[x]=tolower(theline[x]); }
		if (numtok(theline," \t\n\r") > 0)
		{
    		tai.setdatastring(theline);
			tai.learndatastring(convert<string>(i), "(console)");
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
