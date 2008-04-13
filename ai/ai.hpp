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


//we dont always need iostream... unless we debug, right?

#ifndef _AI_H
#define _AI_H

#include "dictionary.h"
#include "newmarkov.h"
#include "graph.h"
#include "context.h"

#include <deque>

#define TRIP_MAXKEY 6
#define TRIP_CONTEXT_TIMEOUT 120

using namespace std;

class AI {
	private:
		CDictionary dictionary;
		CMarkov markov;
	
		//Linguistics model.

		//Moves to CMarkovLang, goes array. It might move to the
		//more-basic class from which CMarkovLang is derived, CBiGraph.
		//Basically, CBiGraph will be a simple non-array algorithmless version
		//of CMarkovLang i.e. a basic data-interface, like CDictionary.

		//Moves to CMarkovLang, goes array.
		unsigned long int relcount;

		vector<unsigned> keywords;
		vector<unsigned> my_dellayed_context;
		
		//Goes into CAIPermutator, will be retrieved from there.
		list<vector<unsigned> > shuffles;

		list<float> scores;


		//Thinking model - Vertical relations (context)
		CGraph vertical;
		map<string, CContextQueue> context;
		map<string, bool> conNicks;
		unsigned vertCount;

		//keyword functions
		const float scorekeywords();
		const float scorekeyword(unsigned wrd);
	
		//debug functions
		void outvector(vector<unsigned>& v);

		//connection methods
		void buildcleanup();

		//shuffle functions
		void generateshuffles();
		void expandshuffles(int method);
		void keywordsbestshuffle();
		int aipermute;
	public:
		AI();

		const long int countrels();
		unsigned countwords();
	
		void readalldata(const string& datafolder);
		void savealldata(const string& datafolder);
		void prune_vertical_nonkeywords();

		void setdatastring(const string& datastring);
		void learndatastring(const string& bywho, const string& where);
		const string getdatastring(const string& where);

		void extractkeywords();
		void expandkeywords();
		void connectkeywords(int method, int nopermute = 0);
		void setpermute(int permute) { aipermute = permute; }
		bool useDijkstra;
};

#endif
