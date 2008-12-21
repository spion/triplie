/*
 *      context.cc
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

#include "context.h"


void CContextQueue::push(const string& bywho, vector<unsigned>& keywords, 
						 const time_t& when)
{
	deque<CContext>::iterator qit;
	
	if (context.size() >= conMax) {
		context.pop_front();
		conNicks.clear();
		for (qit = context.begin(); qit != context.end(); ++qit)
			conNicks[qit->nick] = true;
	}
	CContext element;
	element.nick = bywho;
	element.keywords = keywords;
	element.addtime = when;

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
		learn();
	}
	if (my_dellayed_context.size())
	{
		vector<unsigned> mykeys = my_dellayed_context;
		my_dellayed_context.clear();
		push("(me)",mykeys, my_dellayed_context_time); 
	}
}


void CContextQueue::learn()
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
		vertical->BeginTransaction();
		for (it = itx->begin(); it != itx->end(); ++it)
		{
			for (jt = ity->begin(); jt != ity->end(); ++jt)
			{
				vertical->AddLink(*it,*jt,0);
				vertical->IncLink(*it,*jt);
				//cout << "Link: " << dictionary.GetWord(*it)
				//	 << "->" << dictionary.GetWord(*jt) << endl;
			}
		}
		vertical->EndTransaction();
	}
	// and finally, remove all but the last 2 nicks in list.
	qit = context.begin();
	lastnick = qit->nick;
	while (qit->nick == lastnick) ++qit;
	context.erase(context.begin(), qit);
}


