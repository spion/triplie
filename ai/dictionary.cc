/*
 *      dictionary.cc
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

#include "dictionary.h"
#include <fstream>

using std::ifstream;
using std::ofstream;

unsigned CDictionary::GetKey(const string& word) {
	map<string, unsigned>::iterator x = dict.find(word);
	if (x != dict.end()) return x->second;
	else { return 0; }
}

void CDictionary::AddWord(const string& word, const unsigned& howmany) { 
	string cleanword = word;
	if (!GetKey(cleanword)) {
		if (counter < TRIP_MAXSIZE) {
			dict[cleanword] = counter;
			counters[counter] = howmany;
			backdict[counter++] = cleanword;
		}
	}
	else
	{
		counters[GetKey(cleanword)] += howmany;
	}
	totaloccurances += howmany;
}

unsigned CDictionary::count() { return counter; }

unsigned CDictionary::occurances(unsigned wrd) { 
	return counters[wrd];
}

unsigned CDictionary::occurances(const string& wrd)
{
	return counters[GetKey(wrd)];
}
unsigned CDictionary::occurances()
{
	return totaloccurances;
}

unsigned int CDictionary::readwords(string wordsfile) {
	ifstream wordfile (wordsfile.c_str());
	std::string line;
	unsigned occurances;
	counter=1;
	if (wordfile.is_open())
	{
		while ( (! wordfile.eof() ) && (counter<=TRIP_MAXSIZE) )
		{
			wordfile >> line >> occurances;
			if (line != "") {
				AddWord(line, occurances);
			}
    	}
    	wordfile.close();
	}
	return counter;
}

void CDictionary::savewords(string wordsfile) {
	ofstream myfile (wordsfile.c_str());
	unsigned i;
	if (myfile.is_open()) {
		i=1;
		while ((backdict[i] != "") && (i<counter)) {
   			myfile << backdict[i] << " " << counters[i] << string("\n");
			i++;
		}
		myfile.close();
	}
}

const string CDictionary::GetWord(unsigned key) {
	if (key < counter) return backdict[key];
	else { return ""; }
}
