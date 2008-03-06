/*
 *      dictionary.h
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
#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#include <string>
#include <map>
#define TRIP_MAXSIZE 65500

using std::string;
using std::map;

class CDictionary
{
	private:
		map<string, unsigned> dict;
		//CHashTable<unsigned> dict;
		string backdict[TRIP_MAXSIZE];
		unsigned counter;
		unsigned counters[TRIP_MAXSIZE];
		unsigned long totaloccurances;
	public:
		CDictionary() {
			clear();
			totaloccurances = 0;
		}
		void clear() {
			backdict[0] = "";
			counter = 1; 
			dict.clear();
		}
		const string GetWord(unsigned key); 
		unsigned GetKey(const string& word);
		void AddWord(const string& word, const unsigned& howmany = 1);
		unsigned count();
		unsigned int readwords(string wordsfile);
		void savewords(string wordsfile);
	
		unsigned occurances(unsigned wrd);
		unsigned occurances(const string& wrd);
		unsigned occurances();
};

#endif
