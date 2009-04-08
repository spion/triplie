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

void CContextQueue::push(const string& bywho, vector<unsigned>& keywords, 
						 const time_t& when)
{
	deque<CContext>::iterator qit;
	string bywhol = bywho;
	lowercase(bywhol);
	if (context.size() > 1)
	{
		relink();
		learn();
	}
	if (context.size() > conMax) {
		context.pop_front();
	}
	CContext element;
	element.nick = bywhol;
	element.keywords = keywords;
	element.addtime = when;
	context.push_back(element);
	if (my_dellayed_context.size())
	{
		element.nick = "(me)";
		element.keywords = my_dellayed_context;
		element.addtime = my_dellayed_context_time;
		context.push_back(element);
		my_dellayed_context.clear();
	}
	
}

bool CContextQueue::isNick(const string& n)
{
	string nl = n; lowercase(nl);
	return false;
}

void CContextQueue::learn()
{
	unsigned qit;
	vector<unsigned>::iterator it,jt;
	CContext& last = context[context.size() - 1];
	
	if (last.nick == string("(me)")) return; // don't enforce my connections.
	
	vertical->BeginTransaction();
	
	map<std::pair<unsigned, unsigned>, bool> IncrementApplied;
	for (qit = 0; qit < context.size(); ++qit)
	{
		CContext& current = context[qit];
		double connectedness = conLinks[qit][context.size() - 1];
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
		IncrementApplied.clear();
	} // for qit
	
	vertical->EndTransaction();
}


void CContextQueue::relink()
{
	conLinks.clear();
	unsigned last_it = context.size() - 1;
	CContext& last = context[context.size() - 1];
	map<unsigned, string> wcache;
	//for (unsigned wit = 0; wit != last.keywords.size(); ++ wit)
	//{
		//wcache[wit] = dictionary->GetWord(last.keywords[wit]);
	//}
	// for all queued texts...
	for (unsigned qit = 0; qit < context.size(); ++qit)
	{
		CContext& current = context[qit];
		// need to be close in time.
		if (abs(current.addtime - last.addtime) > TRIP_CONTEXT_TIMEOUT)
				continue;
		
		conLinks[qit][last_it] = (qit+1.0)*(qit+1.0)
								/ (last_it+1.0) * (last_it+1.0);
			
	}
	
}
