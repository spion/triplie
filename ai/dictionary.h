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
#include "sqlite_class.h"
#define TRIP_MAXSIZE 65500

#define MAX_WSIZE 60

#define LEVEN_MAGIC_LIMIT 0.13
#define LEVEN_IGNORE_WORDSIZE 5

using std::string;
using std::map;
using std::pair;


class CDictionary
{
	private:
		SQLite* db;
		//map<string, unsigned> dict;
		//string backdict[TRIP_MAXSIZE];
		//unsigned counter;
		//unsigned counters[TRIP_MAXSIZE];
		unsigned long totaloccurances;
		bool InsideTransaction;
	public:
		void CDictionaryInit(SQLite* dbf);
		void CloseDB() { db->CloseDB(); }
		void OpenDB() { db->OpenDB(); }
		void clear() { }
		const string GetWord(unsigned key); 
		unsigned GetKey(const string& word);
		void AddWord(const string& word, const unsigned& howmany = 1);
		void AddWord(const unsigned& word, const unsigned& howmany = 1, bool noInject = true) ;
		unsigned count();
		unsigned int readwords();
		void savewords();
	
		unsigned occurances(unsigned wrd);
		unsigned occurances(const string& wrd);
		unsigned occurances();

		map< unsigned, map<unsigned,pair<string, double> >  >
			FindSimilarWords(const vector<unsigned>& wordlist);

		void BeginTransaction();
		void EndTransaction();
		void ClearAll();
		
};

double leven(string s, string t);
int minimum(int x, int y, int z);



#endif
