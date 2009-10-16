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

#ifndef _NEWMARKOV_H
#define _NEWMARKOV_H

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "sqlite_class.h"
#include "tokens.h"

using std::map;
using std::vector;
using std::string;

#define MARKOV_MAXORDER 6
#define TRIP_AI_MAXPERMUTATIONS 100

class CMarkov
{
	private:
		SQLite* db;
		bool InsideTransaction;
		unsigned order; 			//order of model
		unsigned internalCount;
		bool CheckIfLinked(vector<unsigned>& words);
		vector<unsigned> partial(const vector<unsigned>& head, 
								unsigned end, unsigned method=2);
		vector<unsigned> partialreverse(unsigned head, 
								unsigned end, unsigned method=2);
		void all(vector<vector<unsigned> >& permutations, const unsigned& method);
	public:
		void CMarkovInit(SQLite* dbf);
		~CMarkov() { }
		void CloseDB() { db->CloseDB(); }
		void OpenDB() { db->OpenDB(); }
		void BeginTransaction();
		void EndTransaction();
		void setOrder() { }
		void remember(vector<unsigned>& sentence);
	
		vector<vector<unsigned> >
			connect (const vector<unsigned>& keywords, unsigned method=0, 
					 long perm_begin = -1, long perm_end = -1);
		vector< vector<unsigned> >
			dconnect(const vector<unsigned>& keywords, unsigned method=0, 
						unsigned MAXPERMS = TRIP_AI_MAXPERMUTATIONS);
	
		void savedata();
		long readdata();
		unsigned count();// { return internalCount; }
		void ClearAll();
		void AddRow(const string&);
		
		unsigned LinkStrength(unsigned x, unsigned y, unsigned order = 1);
		unsigned LinkStrength(unsigned x, bool forward, unsigned order = 1);
		
		void Reindex();

};

#endif
