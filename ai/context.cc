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
#include <iostream>

void lowercase(string& s)
{
	for (unsigned i = 0; i < s.size(); ++i)
		s[i] = tolower(s[i]);
}

void CContextQueue::push(const string& bywho, vector<unsigned>& keywords, 
						 const time_t& when)
{
	deque<CContext>::iterator qit;
	unsigned lastC = context.size() - 1;
	string bywhol = bywho;
	lowercase(bywhol);
	areNicks[bywhol] = true;
	if ((context.size() > 0) && (bywhol == context[lastC].nick))
	{
		context[lastC].keywords.insert(context[lastC].keywords.end(),
									   keywords.begin(), keywords.end()
									  );
		context[lastC].addtime = when;
		
	}
	else {
		if (context.size() > 1)
		{
			relink();
			learn();
		}
		if (context.size() >= conMax) {
			context.pop_front();
		}
		CContext element;
		element.nick = bywhol;
		element.keywords = keywords;
		element.addtime = when;
		context.push_back(element);
		
		if (my_dellayed_context.size())
		{
			vector<unsigned> mykeys = my_dellayed_context;
			my_dellayed_context.clear();
			push("(me)",mykeys, my_dellayed_context_time); 
		}
	}
}

bool CContextQueue::isNick(const string& n)
{
	string nl = n; lowercase(nl);
	if (areNicks.find(nl) != areNicks.end())
	{
		return true;
	}
	return false;
}

void CContextQueue::learn()
{
	unsigned qit;
	vector<unsigned>::iterator it,jt;
	CContext& last = context[context.size() - 1];
	
	if (last.nick == string("(me)")) return; // don't enforce my connections.
	
	map<std::pair<unsigned, unsigned>, bool> IncrementApplied;
	
	vertical->BeginTransaction();
	
	for (qit = 0; qit < context.size() - 1; ++qit)
	{
		CContext& current = context[qit];
		unsigned connectedness = conLinks[qit][context.size() - 1];
		if (connectedness > 0)
		{
			if (connectedness > 10)
			{
				connectedness = 10; // limit the increase to 10 points.
			}
			for (it = current.keywords.begin(); 
				 it != current.keywords.end(); ++it)
			{
				for (jt = last.keywords.begin(); 
					 jt != last.keywords.end(); ++jt)
				{
					
					if (*it != *jt)
					{
						std::pair<unsigned, unsigned> p(*it,*jt);
						if (IncrementApplied.find(p) == IncrementApplied.end())
						{
							vertical->AddLink(*it,*jt,0);
							vertical->IncLink(*it,*jt,connectedness);
							IncrementApplied[p] = true;
						}
					}
				} // for all keys in last.
			} // for all keys in first
		} // should be connected
	} // for qit
	
	vertical->EndTransaction();
}


void CContextQueue::relink()
{
	conLinks.clear();
	CContext& last = context[context.size() - 1];
	map<unsigned, string> wcache;
	for (unsigned wit = 0; wit != last.keywords.size(); ++ wit)
	{
		wcache[wit] = dictionary->GetWord(last.keywords[wit]);
	}
	// for all queued texts...
	for (unsigned qit = 0; qit < context.size() - 1; ++qit)
	{
		CContext& current = context[qit];
		// need to be close in time.
		if (abs(current.addtime - last.addtime) > TRIP_CONTEXT_TIMEOUT)
				continue;
		for (unsigned wit = 0; wit != last.keywords.size(); ++ wit)
		{
			string w = wcache[wit]; // a word from the last.
			
			// if last mentioned someone else in a word...
			if (current.nick == w) { 
				conLinks[qit][context.size() - 1] += 4; // add 3 links
			}
			// If they are closely iterating in a conversation...
			if (qit == context.size() - 2)
			{
				unsigned recheck = 0;
				// or = (conLinks[qit][context.size() - 1] += 1) maybe
				for (unsigned witx = 0; witx < current.keywords.size(); ++witx)
				{	
					string wx = dictionary->GetWord(current.keywords[witx]);
					// and they are both talking about something with the 
					// same keyword symbols 
					if (wx == w) { 
						 recheck = (conLinks[qit][context.size() - 1] += 1); 
							// add one link per symbol (max 4 to 8)
						if (recheck > 7) break;
					}
				}
				//Total 8 points from this is maximum
				if (recheck > 7) break;
			}
		}
	}
	for (unsigned qit = 0; qit < context.size() - 1; ++qit)
	{
		CContext& current = context[qit];
		if (abs(current.addtime - last.addtime) > TRIP_CONTEXT_TIMEOUT)
			continue;
		if (current.keywords.size() == 0) continue;
		string wx = dictionary->GetWord(current.keywords[0]);
		// if last was mentioned by someone else:
		if (wx == last.nick) { 
			conLinks[qit][context.size() - 1] += 2; // add 2 links.
		}
		// total 10 points is absolute maximum.
	}
}
