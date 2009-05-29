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



#ifndef _AI_H
#define _AI_H

#include "context.h"
#include "newmarkov.h"
#include "../protocol/triprotomaster.h"

#include <deque>

#define TRIP_MAXKEY_DEFAULT 8


using namespace std;

class AI {
	private:
		CDictionary dictionary;
		CMarkov markov;
		CGraph vertical;
		SQLite db;
		
		vector<TriplieMasterProto> slaves;
		
		bool Distributed; 

		//Linguistics model.

		unsigned long int relcount;

		vector<unsigned> keywords;
		vector<unsigned> my_dellayed_context;
		
		vector<vector<unsigned> > shuffles;

		list<float> scores;


		//Thinking model - Vertical relations (context)
		map<string, CContextQueue> context;
		map<string, bool> conNicks;
		unsigned vertCount;

		//keyword functions
		const float scorekeywords(const map<unsigned, bool>& keymap);
		const float scorekeyword(unsigned wrd);
		const float scorekeyword_bycount(unsigned wcnt);
	
		//debug functions
		void outvector(vector<unsigned>& v);

		//connection methods
		void buildcleanup();

		//shuffle functions
		void generateshuffles();
		void scoreshuffles(int method);
		void keywordsbestshuffle();
		int aipermute;
		unsigned maxpermutecount;
	public:
		AI(string dbf);
		void CloseDB() { db.CloseDB(); }
		void OpenDB() { db.OpenDB(); }
		void BeginTransaction() { db.BeginTransaction(); }
		void EndTransaction() { db.EndTransaction(); }
		void UnsafeFastMode() { db.Query("PRAGMA journal_mode = MEMORY"); }
		void UnsafeQuery(const string& s) { db.Query(s); }
		unsigned TRIP_MAXKEY;
		const long int countrels();
		unsigned countwords();
		const unsigned countvrels();
	
		void readalldata(const string& datafolder);
		void savealldata(const string& datafolder);
		void prune_vertical_nonkeywords();

		void setdatastring(const string& datastring);
		void learndatastring(const string& bywho, const string& where, const time_t& when);
		const string getdatastring(const string& where, const time_t& when);
		const string getdatastring();

		void extractkeywords();
		void expandkeywords();
		void connectkeywords(int method, int nopermute = 0);
		void setpermute(int permute) { TRIP_MAXKEY = permute; }
		void maxpermute(unsigned num) { maxpermutecount = num; }
		bool useRandom;

	// remote worker functions
		const string getnumericdatastring();
		void setnumericdatastring(const vector<string>&);
		void learnonlymarkov();

	// distributed support functions
		void BeginMarkovTransaction() { markov.BeginTransaction(); }
		void EndMarkovTransaction() { markov.EndTransaction(); }
		void BeginDictionaryTransaction() { dictionary.BeginTransaction(); }
		void EndDictionaryTransaction() { dictionary.EndTransaction(); }
		
		void BeginContextTransaction() { vertical.BeginTransaction(); }
		void EndContextTransaction() { vertical.EndTransaction(); }
		

		void InjectWord(unsigned w, unsigned val) { 
			dictionary.AddWord(w, val, false);
		}
		void InjectMarkov(const string& s)
		{
			markov.AddRow(s);
		}

	// Bootstraps a clean worker.
		void BootstrapDB() { dictionary.ClearAll(); markov.ClearAll(); }

	// local master stub functions
		void SendLearnKeywords();
		vector<vector<unsigned> > GetRepliesFromAll();

		void SendAllSlavesAndWait(const string&);
		void connect_to_workers(string file);
	
};

#endif
