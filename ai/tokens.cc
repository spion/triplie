/*
 *      tokens.cc
 *
 *      Copyright 2008 Gorgi Kosev <spion@spion.ws>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <math.h>

#include "tokens.h"

double uniform_deviate (int seed)
{
  return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}

void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void tokenizelossles(const string& str,
                      vector<string>& tokens,
                      const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos+1));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


string gettok(const std::string& str, long int n , 
              const std::string& delimiters) {
    vector<string> tokens;
    long int x;
    tokenize(str,tokens,delimiters);
    x = (long int) tokens.size();
    if ((n>=0) && (n < x)) return tokens[n];
    else { return std::string(""); }
}

long int numtok(const std::string& str, 
              const std::string& delimiters) {
    vector<string> tokens;
    tokenize(str,tokens,delimiters);
    return (long int)tokens.size();
}

long int findtok(const std::string& str, const std::string& token, 
              const std::string& delimiters) {
    vector<string> tokens;
    long int x; long int i;
    tokenize(str,tokens,delimiters);
    x = (long int) tokens.size();
    for (i=0;i<x;i++) {
        if (token == tokens[i]) { return i; break; }
    }
    return -1;
}

string subtokstring(const vector<string>& tokens,unsigned int n1, unsigned int n2, const string& c) {
	string res; unsigned int i;
	i=n1; res = "";
	while ((i <= n2) && (i < tokens.size())) {
		res = res + tokens[i] + c;
		i++;
	}
	if (res.size()>1) { res.erase(res.size()-1,1); }
	return res;
}
