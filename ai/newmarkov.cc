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

#include "newmarkov.h"
#include "graph.h"
#include <iostream>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>

#define TRIP_AI_MAXNODES 128000
#define PARTIAL_CACHING

using std::deque;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;


void CMarkov::CMarkovInit(SQLite* dbf)
{
	internalCount = 0;
	db = dbf;
}

void CMarkov::Reindex(unsigned order)
{

}

void CMarkov::remember(vector<unsigned>& sentence)
{
	unsigned i;
	vector<unsigned> v = sentence;
	v.insert(v.begin(), 0);
	if (sentence.size() > 1)
	{
		for (i = 0; i <= MARKOV_MAXORDER; ++i)
			v.push_back(0);
		for (i = 0; i <= sentence.size(); ++i)
		{
			stringstream query;
			query << " WHERE id1="
				  << v[i  ] << " AND id2="
				  << v[i+1] << " AND id3="
				  << v[i+2] << " AND id4="
				  << v[i+3] << " AND id5="
				  << v[i+4] << " AND id6="
				  << v[i+5] << ";";
			db->Query(string("UPDATE or IGNORE markov SET val=val+1 ") + query.str());
			stringstream qvi;
			qvi << " (" 
				<< v[i  ] << "," << v[i+1] << ","
				<< v[i+2] << "," << v[i+3] << ","
				<< v[i+4] << "," << v[i+5] << ", 1);";
			db->Query(string("INSERT or IGNORE INTO markov VALUES") + qvi.str());
			++internalCount;
		}
	}
}

unsigned CMarkov::LinkStrength(unsigned x, unsigned y, unsigned order)
{
	stringstream query;
	if (order >= MARKOV_MAXORDER) { order = MARKOV_MAXORDER-1; } 
	query << "SELECT sum(val) FROM markov WHERE id1 = " << x
		  << "AND id" << 1+order << " = " << y;
	db->Query(query.str());
	return convert<unsigned>(db->GetLastResult()[0]);
}

unsigned CMarkov::LinkStrength(unsigned x, bool forward, unsigned order)
{
	stringstream query;
	if (order >= MARKOV_MAXORDER) { order = MARKOV_MAXORDER-1; } 
	query << "SELECT sum(val) FROM markov WHERE ";
	if (forward)
	{
		query << "id1 = " << x;
		query << " AND id" << 1+order << "<>0";
	}
	else
	{	
		query << "id" << 1+order << " = " << x;
		query << " AND id1 <> 0";
	}
	query << ";";
	db->Query(query.str());
	return convert<unsigned>(db->GetLastResult()[0]);
}

void CMarkov::savedata(const string& sfile)
{

}

long int CMarkov::readdata(const string& sfile)
{
	Reindex();
	count();
	return internalCount;
}

vector<unsigned> CMarkov::partial(const vector<unsigned>& head, 
								  unsigned end, unsigned method)
{
	unsigned markov_nodes = method + 1;
	if (markov_nodes >= MARKOV_MAXORDER) markov_nodes = MARKOV_MAXORDER-1;
	unsigned real_head_size = std::min((unsigned)head.size(),markov_nodes); 
	
	map<unsigned, bool> blacknodes;
	
	deque<unsigned>		graynodes;
	deque<unsigned>::iterator next;
	
	TNodeLinks previousnode;
	
	// Filling up some previous nodes upto real head size.
	for (unsigned i = head.size(); i > head.size() - real_head_size; --i)
	{
		// if its the first node in the head, it "has no previous"
		if (i != 1)
			previousnode[head[i-1]] = head[i-2];
	}
	//previousnode.erase(0); // zero has no previous when forward-connecting.
	bool finished = false;
	
	unsigned currentnode = head[head.size() - 1];
	graynodes.push_back(currentnode);
	unsigned querycount = 0;
	while (!finished)
	{
		next = graynodes.begin();
		if (next != graynodes.end())
		{
			currentnode = *next;
			blacknodes[currentnode] = true;
		}
		else { break; }
		stringstream querytwo; querytwo << " WHERE 1=1 ";
		unsigned backtrack_node = currentnode;
		unsigned nSelect = 0;
		while (true) { 
			++nSelect; 
			if (nSelect >= markov_nodes) break;
			TNodeLinks::iterator backtrack_nodex 
				= previousnode.find(backtrack_node);
			if (backtrack_nodex == previousnode.end()) break;
			backtrack_node = backtrack_nodex->second;
		}
		backtrack_node = currentnode;
		unsigned i;
		for (i = 0; i < nSelect; ++i)
		{
			querytwo << " AND id" << (nSelect-i) << "=" << backtrack_node;
			TNodeLinks::iterator backtrack_nodex 
				= previousnode.find(backtrack_node);
			if (backtrack_nodex == previousnode.end()) break;
			backtrack_node = backtrack_nodex->second;
		}
		querytwo << ";";
		stringstream queryone;
		queryone << "SELECT id" << (nSelect + 1) << " FROM markov ";
		db->Query(queryone.str() + querytwo.str()); //Get all connected nodes.
		vector<string> v;
		while (( (v = db->GetNextResult()).size() )) 
		{
			unsigned unode = convert<unsigned>(v[0]);
			if ((blacknodes.find(unode) == blacknodes.end()) // node not black
				&& (previousnode.find(unode) == previousnode.end()))
			{
				graynodes.push_back(unode);  //add to explorable list
				previousnode[unode] = currentnode;
				if (unode == end) finished = true;
			}
		}
		if (!graynodes.size()) break;
		graynodes.pop_front();
		if (++querycount > TRIP_AI_MAXNODES) break;
	}
	//exit(1);
	if (finished) { // we have a path
		vector<unsigned> result;
		unsigned backtrack_node = end;
		while (backtrack_node !=  head[head.size() - 1])
		{
			result.push_back(backtrack_node);
			backtrack_node = previousnode[backtrack_node];
		}
		reverse(result.begin(), result.end());
		result.insert(result.begin(), head.begin(), head.end());
		return result;
	}
	else { // we ran out of nodes to explore
		vector<unsigned> result;
		result.insert(result.begin(), head.begin(), head.end());
		result.push_back(0);
		result.push_back(end);
		return result;
	}
}


vector<unsigned> CMarkov::partialreverse(unsigned head, 
										 unsigned end, unsigned method)
{
	unsigned markov_nodes = method + 1;
	if (markov_nodes >= MARKOV_MAXORDER) markov_nodes = MARKOV_MAXORDER-1;
	map<unsigned, bool> blacknodes;
	
	deque<unsigned>		graynodes;
	deque<unsigned>::iterator next;
	
	TNodeLinks next_node;
	
	unsigned currentnode = end;
	graynodes.push_back(currentnode);
	unsigned querycount = 0;
	bool finished = false;
	while (!finished)
	{
		next = graynodes.begin();
		if (next != graynodes.end())
		{
			currentnode = *next;
			blacknodes[currentnode] = true;
		}
		else { break; }
		stringstream querytwo; querytwo << " WHERE 1=1 ";
		unsigned backtrack_node = currentnode;
		unsigned nSelect = 0;
		while (true) { 
			++nSelect; 
			if (nSelect >= markov_nodes) break;
			querytwo << " AND id" << (nSelect+1) << "=" << backtrack_node;
			TNodeLinks::iterator backtrack_nodex = 
				next_node.find(backtrack_node);
			if (backtrack_nodex == next_node.end()) break;
			backtrack_node = backtrack_nodex->second;
		}
		querytwo << ";";
		stringstream queryone;
		queryone << "SELECT id1 FROM markov ";
		db->Query(queryone.str() + querytwo.str()); //Get all connected nodes.
		vector<string> v;
		while (( (v = db->GetNextResult()).size() )) 
		{
			unsigned unode = convert<unsigned>(v[0]);
			if ((blacknodes.find(unode) == blacknodes.end()) // node not black
				&& (next_node.find(unode) == next_node.end()))
			{
				graynodes.push_back(unode);  //add to explorable list
				next_node[unode] = currentnode;
				if (unode == head) 
				{
					finished = true;
					break;
				}
			}
		}
		if (!graynodes.size()) break;
		graynodes.pop_front();
		if (++querycount > TRIP_AI_MAXNODES) break;
	}
	if (finished) { // we have a path
		vector<unsigned> result;
		unsigned backtrack_node = head;
		while (backtrack_node != end)
		{
			result.push_back(backtrack_node);
			backtrack_node = next_node[backtrack_node];
		}
		result.push_back(end);
		return result;
	}
	else { // we ran out of nodes to explore
		vector<unsigned> result;
		result.push_back(head);
		result.push_back(0);
		result.push_back(end);
		return result;
	}
}

vector<vector<unsigned> > CMarkov::connect(const vector<unsigned>& keywords, 
										  unsigned method,
										  long long perm_begin,
										  long long perm_end)
{
	vector<vector<unsigned> > permutations;
	if (keywords.size() <= 1) { 
		permutations.push_back(keywords); 
		return permutations; 
	}
	vector<unsigned> permutation = keywords;
	sort(permutation.begin(), permutation.end());
	if (perm_begin <= 0) permutations.push_back(permutation);
	long long perm_id = 1;
	while (std::next_permutation(permutation.begin(), permutation.end()))
	{
		if (((perm_end < 0) || (perm_id < perm_end)) && (perm_id > perm_begin))
		permutations.push_back(permutation);
		++perm_id;
	}
	all(permutations, method);
	return permutations;
}



vector<vector<unsigned> > CMarkov::dconnect(const vector<unsigned>& keywords, 
											unsigned method, 
											unsigned MAXPERMS)
{
	vector<vector<unsigned> > permutations;
	if (keywords.size() <= 1) { 
		permutations.push_back(keywords); 
		return permutations; 
	}
	vector<unsigned> permutation;
	map<unsigned, bool> used;
	for (unsigned i = 0; i < MAXPERMS; ++i)
	{
		permutation.clear(); used.clear();		
		
		for (unsigned j = 0; j < keywords.size(); ++j)
		{
			unsigned k = unsigned(uniform_deviate(rand()) * keywords.size());
			unsigned element = keywords[k];
			if (used.find(element) == used.end())
			{
				used[element] = true;
				permutation.push_back(element);
			}
		}
		permutations.push_back(permutation);
	}
	all(permutations, method);
	return permutations;
}

void CMarkov::all(vector<vector<unsigned> >& permutations, const unsigned& method)
{
#ifdef PARTIAL_CACHING
	map<vector<unsigned>, vector<unsigned> > headcache;
 	map<vector<unsigned>, vector<unsigned> >::iterator headsearch;
#endif

	vector<vector<unsigned> > extras;
	vector<unsigned> extra;
	//vector<unsigned> permutation;	
	unsigned permusize = permutations.size(); // size of original permutations
	for (unsigned i = 0; i < permusize; ++i)
	{ // for each permutation
		vector<unsigned>& permutation = permutations[i];
		permutation.insert(permutation.begin(), 0);
		permutation.push_back(0);
		vector<unsigned> head; head.push_back(permutation[0]);
		for (unsigned j = 1; j < permutation.size(); ++j)
		{ // connect it to the end
#ifdef PARTIAL_CACHING
			headsearch = headcache.end();
			vector<unsigned> locator;
			
			if (j < permutation.size() - 2)
			{
				locator.assign(permutation.begin(), 
							   permutation.begin()+j+1);
				headsearch = headcache.find(locator);
			}
			
			if (headsearch != headcache.end()) // already cached partial connector
			{
				head = headsearch->second;
			}
			else
			{
#endif
				if (j > 1)
				{
					head = partial(head,permutation[j],method);
					// extra answers?
					if (permutation[j] != 0)
					{
						extra = partial(head,0,method);
						if (extra.size() > 1)
						{
							for (vector<unsigned>::iterator ek = extra.begin() + 1;
								 ek != extra.end(); ++ek)
							{
								if (*ek == 0)
									extra.erase(ek + 1, extra.end());
							}
						}
						extras.push_back(extra);
					}
				}
				else
				{
					head = partialreverse(head[0],permutation[j],method);
					//extras.push_back(partial(head,0,method));
				}
#ifdef PARTIAL_CACHING
				if (j < permutation.size() - 2)
					headcache[locator] = head;
			}
#endif			
		}
		if (head.size() > 1)
		{
			for (vector<unsigned>::iterator ek = head.begin() + 1;
		 		ek != head.end(); ++ek)
			{
			if (*ek == 0)
				head.erase(ek + 1, head.end());
			}
		}
		permutations[i] = head; // and add it to results
	}
	permutations.insert(permutations.begin(), extras.begin(), extras.end());
}

unsigned CMarkov::count() {
	db->Query("SELECT COUNT(val) FROM markov;");
	internalCount = convert<unsigned>(db->GetLastResult()[0]);
	return internalCount; 
}

void CMarkov::BeginTransaction()
{
	db->BeginTransaction();
}


void CMarkov::EndTransaction()
{
	db->EndTransaction();
}

void CMarkov::ClearAll()
{
	db->Query("DELETE FROM markov;");
}

void CMarkov::AddRow(const string& row)
{
	db->Query(string("INSERT or REPLACE INTO markov VALUES (") + row + ")");
}

