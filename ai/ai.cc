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

#include <fstream>
#include <algorithm>
#include "tokens.h"
#include "ai.hpp"
//#include <iostream>

AI::AI() {
	useDijkstra = false;
	keywords.clear();
	relcount=0;
	conMax=10;
	conLearn=2;
	conCount=0;
	markov.setOrder(6);
	setpermute(1);
	vertCount=0;
}


void AI::prune_vertical_nonkeywords()
{
	TGraphH::iterator it;
	TNodeLinks::iterator jt;
	for (it = vertical.forward.begin(); 
		it != vertical.forward.end(); ++it)
	{
		if (scorekeyword(it->first) < 0)
		{
			vertical.forward.erase(it);
		}
		else
		{
			for (jt = it->second.begin(); jt != it->second.end(); ++jt)
			{
				if (scorekeyword(jt->first) < 0)
				{
					it->second.erase(jt);
				}
			}
		}
	}
}

//REFACTOR: will continue to reside in AI
void AI::readalldata(const string& datafolder) {
	relcount=0;
	dictionary.clear();
	relcount+=dictionary.readwords(datafolder + "/words.dat");
	relcount+=markov.readdata(datafolder + "/rels");
	vertCount+=vertical.ReadLinks(datafolder + "/relsv.dat");
	prune_vertical_nonkeywords();
}


//REFACTOR: will continue to reside in AI
void AI::savealldata(const string& datafolder) {
	dictionary.savewords(datafolder+"/words.dat");
	markov.savedata(datafolder+"/rels");
	vertical.SaveLinks(datafolder + "/relsv.dat");
}


void AI::learndatastring(const string& bywho) {
	markov.remember(keywords);
	context_push(bywho);
}


/* --- CONTEXT --- */

void AI::context_push(const string& bywho)
{
	deque<CContext>::iterator qit;
	extractkeywords();
	
	if (context.size() >= conMax) {
		context.pop_front();
		conNicks.clear();
		for (qit = context.begin(); qit != context.end(); ++qit)
			conNicks[qit->nick] = true;
	}
	CContext element;
	element.nick = bywho;
	element.keywords = keywords;
	element.addtime = time(0);

	context.push_back(element);
	conNicks[bywho] = true;

	if (conNicks.size() > 1) { 
		conCount++;
		conNicks.clear();
		conNicks[bywho] = true;
	}
	if (conCount >= conLearn)
	{
		conCount=1;
		learncontext();
	}
	if (my_dellayed_context.size())
	{
		keywords = my_dellayed_context;
		my_dellayed_context.clear();
		context_push("(me)"); 
	}
}


void AI::learncontext()
{
	deque<CContext>::iterator qit;
	vector<unsigned>::iterator it,jt;
	vector<unsigned>::iterator i,j;
	vector< vector<unsigned> > fullcontext;
	vector<time_t> fullcontexttimes;
	vector<string> fullcontextnicks;
	vector<unsigned> nickcontext;
	vector< vector<unsigned> >::iterator itx, ity;
	time_t lasttime;
	string lastnick = "";
	for (qit = context.begin(); qit != context.end(); ++qit)
	{
		if (qit->nick != lastnick)
		{
			if (lastnick != "")
			{
				fullcontext.push_back(nickcontext);
				fullcontexttimes.push_back(lasttime);
				fullcontextnicks.push_back(lastnick);
			}
			lastnick = qit->nick;
			nickcontext.clear();
			if (fullcontext.size() > 1) // two nicks in list
				break; //from fullcontext filling loop
		}
		for (it = qit->keywords.begin(); it != qit->keywords.end(); ++it)
		{
			nickcontext.push_back(*it);
		}
		lasttime = qit->addtime;
	} // for qit
	if (fullcontext.size() <= 1) return;
	// now we must connect all words from fullcontext[0]
	// with all words from fullcontext[1]
	ity = fullcontext.begin(); itx = ity++;
	if ( //don't learn context that has more then TIMEOUT seconds distance
		(abs(fullcontexttimes[1] - fullcontexttimes[0]) < TRIP_CONTEXT_TIMEOUT)
		&& //don't learn context TO words you have generated. [1]->[0]
		(fullcontextnicks[1] != (string)("(me)"))
		//itx fullcontext[0], ity fullcontext[1] connect itx to ity
	   )
	{
		for (it = itx->begin(); it != itx->end(); ++it)
		{
			for (jt = ity->begin(); jt != ity->end(); ++jt)
			{
				vertical.IncLink(*it,*jt);
				//cout << "Link: " << dictionary.GetWord(*it)
				//	 << "->" << dictionary.GetWord(*jt) << endl;
			}
		}
	}
	// and finally, remove all but the last 2 nicks in list.
	qit = context.begin();
	lastnick = qit->nick;
	while (qit->nick == lastnick) ++qit;
	context.erase(context.begin(), qit);
}

class CMContext
{
	public:
		unsigned wrd;
		double   cnt;
		bool operator > (const CMContext& x1) const
		{
			return (bool)(this->cnt > x1.cnt);
		}
		bool operator < (const CMContext& x1) const
		{
			return (bool)(this->cnt < x1.cnt);
		}
};

void AI::expandkeywords()
{
	map<unsigned, double> kcontext;
	map<unsigned, double>::iterator z;
	map<unsigned, unsigned>::iterator reply_keywrd;
	vector<unsigned>::iterator keywrd;
	vector<CMContext> results;
	vector<CMContext>::iterator rit;
	for (keywrd = keywords.begin(); keywrd != keywords.end(); ++keywrd)
	{
		TGraphH::iterator req_keywrd;
		req_keywrd = vertical.GetFwdLinks(*keywrd);
		if (req_keywrd != vertical.NonExistant()) {
			for (reply_keywrd = req_keywrd->second.begin(); 
				 reply_keywrd != req_keywrd->second.end(); 
				 ++reply_keywrd)
			{
				kcontext[reply_keywrd->first]+= 
				  (scorekeyword(reply_keywrd->first)>0 ? 1.0 :0.0-1.0) *
				  1.0 * reply_keywrd->second / vertical.count
				  * 
				  logf(
					   (1.0 * reply_keywrd->second) * vertical.count
					   /
					   (
					   1.0 * vertical.CountFwdLinksStrength(req_keywrd->first)
						 * vertical.CountBckLinksStrength(reply_keywrd->first)
					   )
					  );
			}
		}
	}

	CMContext element;
	for (z = kcontext.begin(); z != kcontext.end(); ++z)
	{
			element.wrd = z->first;
			element.cnt = z->second;
			results.push_back(element);
	}
	sort(results.begin(), results.end());
	reverse(results.begin(), results.end());
	
	unsigned keycnt = 1;
	keywords.clear();
	for (rit = results.begin(); rit != results.end(); ++rit)
	{
		//if ((useDijkstra) && (keycnt > TRIP_MAXKEY/2)) break;
		if (keycnt > TRIP_MAXKEY) break;
		keywords.push_back(rit->wrd);
		++keycnt;
	}
}

/* public */

const long int AI::countrels() {
	return markov.count();
}

void AI::setdatastring(const string& datastring) {
	vector<string> strkeywords;
	strkeywords.clear();
	string theline = datastring; unsigned long int x;
	for(x=1;x<theline.size();x++)
	{
		switch (theline[x])
		{
			case '.':
			case ',':
			case '!':
			case '?':
			case ':':
			case '(':
			case ')':
			case '-': theline.insert(x," "); ++x; theline.insert(x+1," "); ++x; 
					  break;
			default : theline[x]=tolower(theline[x]);
		}
	}
	if (theline!="") {
		tokenize(theline,strkeywords," \t");
	}
	unsigned long int litmp = strkeywords.size(); unsigned long int i;
	for (i=0;i<litmp;i++) {
		dictionary.AddWord(strkeywords[i]);
	}
	keywords.clear();
	for (i=0; i<strkeywords.size(); i++) 
	{
		keywords.push_back(dictionary.GetKey(strkeywords[i]));
	//	cout << strkeywords[i] << "(" 
	//		 << dictionary.GetKey(strkeywords[i]) << ") ";
	}
	//cout << endl;
}

const string AI::getdatastring() {
	string theline="";
	unsigned int i;
	if (keywords.size())
	{
		for (i=0;i<keywords.size();i++)
		{
			string kwrd = dictionary.GetWord(keywords[i]);
			string::size_type loc = kwrd.find_first_of(".,!?:)-",0);
			if (loc != string::npos) //interpunction char.
				{ theline=theline+kwrd; }
			else  //normal word
				{ theline=theline+" "+kwrd; }
		}
	}
	my_dellayed_context = keywords;
	return theline;
}

void AI::extractkeywords() {
	vector<unsigned> sorted;
	vector<unsigned>::iterator it;//,ix;l
	vector<unsigned>::iterator imax;
	sorted.clear();
	double infinity = numeric_limits<double>::infinity();

	for (it=keywords.begin(); it!=keywords.end(); it++)
	{
		double scorekeywordresult = scorekeyword(*it);
		if ((scorekeywordresult > 0.0) && (scorekeywordresult != infinity))
		{
			sorted.push_back(*it);
		}
	}
	keywords.clear();
	keywords.swap(sorted);
}

//REFACTOR: Move to CAIPermutator class.
void AI::generateshuffles()
{
	vector<unsigned> shuffie;
	list<vector<unsigned> > results;
	shuffles.clear();
	if (keywords.size() >0) { results.push_back(keywords); }
	int number=keywords.size()-1;
    for (int k=1; k<=number; k++)
    {
        shuffie=keywords;
        unsigned temp = shuffie[k];
		shuffie[k] = shuffie[0];
		shuffie[0] = temp;
        results.push_back(shuffie);
    }
	shuffles.swap(results);
	//cout << "## shufs generated" << endl;
}

//REFACTOR: Move to CAIStatistics
const float AI::scorekeywords() {
	float curscore = 0.01;
	
	unsigned int it; unsigned twrd;
	for (it=0;it<keywords.size();it++)
	{
		//DISCARDED#3
		twrd = keywords[it];
		if (keywords[it])
			curscore+= scorekeyword(twrd);
	}
	//cout << "## score calculated: " << (2.0 * curscore) / 
	//					(keywords.size() + 0.001) << endl;
	return (2.0 * curscore) / (keywords.size()+0.001);
}

void AI::expandshuffles(int method)
{
	list<vector<unsigned> >::iterator it;
	scores.clear();
	for (it=shuffles.begin();it!=shuffles.end();it++)
	{
		keywords.clear();
		keywords = *it;
		connectkeywords(method,1);
		*it = keywords;
		scores.push_back(scorekeywords());
	}
}

void AI::keywordsbestshuffle()
{
	list<float>::iterator scoreit = scores.begin();
	list<vector<unsigned> >::iterator it = shuffles.begin();
	float bestscore = -1000.0; // needs 1000 words in reply to get below.
	while ( (it != shuffles.end()) && (scoreit!=scores.end()) )
	{
		if (*scoreit > bestscore)
		{
			bestscore=*scoreit;
			keywords.clear();
			keywords=*it;
		}
		++it;++scoreit;
	}
	//cout << "## answer score (" << bestscore << ")" << endl;
}


/* * * * * * * * * * * * * * *
 * AI Answering
 * * * * * * * * * * * * * * */

void AI::buildcleanup() {
	vector<string> strkeywords;
	for (unsigned i = 0; i < keywords.size(); ++i)
	{
		strkeywords.push_back(dictionary.GetWord(keywords[i]));
	}
	string bclean;
	unsigned int rpsz = strkeywords.size();
	unsigned int il;
	bool inced;
	il=0;bclean="";inced = false;
	while (il<rpsz) {
		inced = false;
		if (il+1 < rpsz) {
			if (
				(strkeywords[il] == strkeywords[il+1])
			   )
				{ il+=1; inced=true; }
			else if (il+3 < rpsz) {
				if ((strkeywords[il] == strkeywords[il+2]) && 
					(strkeywords[il+1] == strkeywords[il+3]) 
					) {
					il+=2;
					inced=true;
				}
				else if (il+5 < rpsz) {
					if (  (strkeywords[il] == strkeywords[il+3])
			   		&& (strkeywords[il+1] == strkeywords[il+4])
					&& (strkeywords[il+2] == strkeywords[il+5])
			   		) {
				   		il+=3;
						inced=true;
			   		}
				} // il+5=rpsz
			} // il+3=rpsz
		}
		if (!(inced)) {
			if (il<rpsz) { bclean=bclean+" "+strkeywords[il]; }
			il+=1;
		}
	}
	strkeywords.clear();
	tokenize(bclean,strkeywords," ");
	unsigned i = 0;
	while ((i < strkeywords.size()) && (strkeywords[i] == ","))
		++i;
	keywords.clear();
	for ( ; i < strkeywords.size(); ++i)
	{
		if ((i != (strkeywords.size() - 1))
			|| (strkeywords[i] != ","))
			keywords.push_back(dictionary.GetKey(strkeywords[i]));
	}
	
}


void AI::connectkeywords(int method, int nopermute)
{
	string answerstr = ""; string tmpstr = "";
	if ((!aipermute) || (nopermute)) //basic methods.
	{
		vector<unsigned> temp;
		if (useDijkstra)
		{
			temp = markov.dconnect(keywords,method);
		}
		else 
		{
			temp = markov.connect(keywords,method);
		}
		keywords.swap(temp);
		replace(keywords.begin(),keywords.end(),
				(unsigned)0,dictionary.GetKey(","));
  	}
  	else 
  	{
		generateshuffles();
		expandshuffles(method);
		keywordsbestshuffle();
  	}
	buildcleanup();
}

//REFACTOR: move to CAIStatistics.
const float AI::scorekeyword(unsigned wrd)
{
	return 0.0 - logf(0.85 +
						(
							100.0 *
							dictionary.occurances(wrd) 
										/ (2.0 * dictionary.occurances())
						)
					 );
}

unsigned AI::countwords() { return dictionary.count(); }

