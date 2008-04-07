/*
 *      markov.cc
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

#include "markov.h"
#include <math.h>
#include <iostream>
using std::cout;
using std::endl;

#define TRIP_AI_MAXDEPTH 32000

string IntToStr(int x)
{
	std::stringstream ss;
	std::string str;
	ss << x;
	ss >> str;
	return str;
}


template<typename T> bool myabs(T a)
{
	return a >= 0 ? a : 0-a;
}

CMarkov::CMarkov()
{
}

CMarkov::~CMarkov()
{
}

void CMarkov::setOrder(unsigned N)
{
	order = N;
}

void CMarkov::remember(vector<unsigned>& sentence)
{
	long i,j;
	for (i = sentence.size()-1; i >= 0; --i)
	{
		for (j = 1; j <= (long)order; ++j)
		{
			if (i >= j) //remaining backwards greater then order
			{
				mdata[j].IncLink(sentence[i-j],sentence[i]);
				++internalCount;
			}
		}
	}

}

void CMarkov::setLink(unsigned x, unsigned y, unsigned val, unsigned r)
{
	mdata[r].AddLink(x,y,val);
}

/*
 * BFS based connect
 */

vector<unsigned> CMarkov::connect(vector<unsigned>& keywords,
								  unsigned method)
{
	vector<unsigned>::iterator it;
	unsigned markov_order = method + 1;
	if (markov_order > order) markov_order = order;
	map<unsigned, bool> foundnodes; //marks found nodes from the keywords
	map<unsigned, bool> deadnodes;  //non-expandable nodes.
	map<unsigned, bool> blacknodes; //explored nodes
	deque<unsigned>     graynodes;  //nodes to be explored.
	deque<unsigned>::iterator next; //next gray node in list.
	
	unsigned keynode; // current keynode from which exploring began.
	
	TNodeLinks previousnode;
	
	vector<unsigned> results;
	
	unsigned currentnode;
	if (keywords.size() > 0) {
		currentnode = keywords[0];
		foundnodes[currentnode] = true;
	}
	else {
		return results;
	}

	bool finished = false;
	bool nextassigned = true;
	unsigned nodecount;
	nodecount = 0;
	graynodes.push_back(currentnode);
	previousnode[currentnode] = 0;
	previousnode[0] = 0;
	keynode = currentnode;
	while (!finished)
	{
		if (graynodes.size() > 0) 
			graynodes.pop_front();
		if (blacknodes.find(currentnode) == blacknodes.end()) 
		{ //node not black
			blacknodes[currentnode] = true; // mark as black
			TGraphH::iterator links = mdata[1].GetFwdLinks(currentnode);
			if (links != mdata[1].NonExistant())
			{	//if there are links to other nodes from this node
				TNodeLinks::iterator node;
				for (node = links->second.begin(); 
				 node != links->second.end(); ++node)
				{ //for each node that is linked
					if (blacknodes.find(node->first) == blacknodes.end())
					{	//node not black
						unsigned k = 2;
						unsigned previous = previousnode[currentnode];
						bool nth_order_connected = true;
						while (k <= markov_order)
						{
							if ( (!mdata[k].CheckLink(previous, node->first)) 
								&& (previous) ) //both previous nonzero, and 
												//no links from it to node
							{
								nth_order_connected = false;
								break;
							}
							previous = previousnode[previous];
							k++;
						} //check if its fully n-th order connected.
						if (nth_order_connected)
						{ //add to gray nodes
							graynodes.push_back(node->first);
							previousnode[node->first] = currentnode;
						}
					
					} //if linked node not blacknode 
				} //for all nodes linked to the current
			} //if not nonexistant.
		} //if currentnode not black
		next = graynodes.begin(); //prepare next gray node to explore.
		nodecount++;
		//if there is such a node
		if ((next != graynodes.end()) && (nodecount <= TRIP_AI_MAXDEPTH))
		{
			currentnode = *next;
			//check if we have come to any of the other keyword nodes.
			for (it = keywords.begin(); it != keywords.end(); ++it)
			{
				
				if ((currentnode == *it) && 
					(foundnodes.find(*it) == foundnodes.end()))
				{ // if we arrived to a previously not found node.
					unsigned pathnode = currentnode;
					TNodeLinks::iterator node;
					
					vector<unsigned>::size_type revOffset = results.size();
					while ( 
						   (( (node = previousnode.find(pathnode)) 
							!= previousnode.end() ))
						  && (foundnodes.find(pathnode) == foundnodes.end())
						   )
					{
						results.push_back(node->second); //push previous of
						                                 //pathnode 
						pathnode=node->second;
					}
					foundnodes[currentnode] = true; // mark node as found
					blacknodes.clear(); // clear all black nodes.
					graynodes.clear(); // clear all gray nodes
					// partial reverse.
					reverse(results.begin() + revOffset, results.end()); 
					results.push_back(currentnode); // push last node :P
					if (foundnodes.size() >= keywords.size()) 
					{
						finished = true;
					}
					//otherwise, we are not finished, begin exploring again
					nodecount = 0; //reset nodecount
					keynode = currentnode;
					graynodes.push_back(keynode);
					break; //break the for loop. current node is setup right.
				}
			} //for each keyword	
		}
		else
		{ //fully expanded tree, but no next keyword found, 
		  //or expanded past limit
			graynodes.clear();
			blacknodes.clear();
			//results.push_back(keynode); //push keynode anyway
			if ((previousnode.find(keynode))->second == 0) //if nobody arrived 
				foundnodes.erase(keynode); //to it before, unmark found
			
			deadnodes[keynode] = true; // mark as dead (non-expandable)
			results.push_back(0); // add a zero to signify lack of connector
			
			// pick any node not in foundnodes as the next node to resume.
			for (it = keywords.begin(); it != keywords.end(); ++it)
			{
				if (   (foundnodes.find(*it) == foundnodes.end())
					&& (deadnodes.find(*it) == deadnodes.end())  )
				{ //node not found and not dead...
					keynode = *it; //next key node from which we begin
					foundnodes[keynode] = true;
					currentnode = keynode;
					previousnode[keynode] = 0;
					graynodes.push_back(keynode);
					nextassigned = true;
					nodecount = 0;
					break;
				}
			}
		} //assigning next node done.
		
		if ((foundnodes.size() >= keywords.size()) || (graynodes.size() == 0))
			finished = true;
	} // done finding the path (or a partial path)
	  //return whatever results found.
	return results;
}


class CGrayLink
{
	public:
		unsigned node;
		double pathlength;
		
		CGrayLink(unsigned nodeto, double plength):
		node(nodeto), pathlength(plength) { }
		
		bool operator > (const CGrayLink& x1) const
		{
			return (bool)(this->pathlength < x1.pathlength);
		}
		bool operator < (const CGrayLink& x1) const
		{
			return (bool)(this->pathlength > x1.pathlength);
		}
		
		bool operator == (const CGrayLink& x1) const
		{
			return (bool)(this->node == x1.node);
		}
};

/*
 * dijkstra search based connect
 */
vector<unsigned> CMarkov::dconnect(vector<unsigned>& keywords,
								  unsigned method)
{
	vector<unsigned>::iterator it;
	unsigned markov_order = method + 1;
	if (markov_order > order) markov_order = order;
	map<unsigned, bool> foundnodes; //marks found nodes from the keywords
	map<unsigned, bool> deadnodes;  //non-expandable nodes.
	map<unsigned, bool> blacknodes; //explored nodes
	
	map<unsigned, double> pathlengths; //subjective path lengths
	
	priority_queue<CGrayLink>      graynodes; //nodes to be explored
	CGrayLink next(0,0.0);

	
	unsigned keynode; // current keynode from which exploring began.
	
	TNodeLinks previousnode;
	
	vector<unsigned> results;
	
	unsigned currentnode;
	if (keywords.size() > 0) {
		currentnode = keywords[0];
		foundnodes[currentnode] = true;
	}
	else {
		return results;
	}

	bool finished = false;
	bool nextassigned = true;
	unsigned nodecount;
	nodecount = 0;
	graynodes.push(CGrayLink(currentnode,0.0));
	previousnode[currentnode] = 0;
	previousnode[0] = 0;
	keynode = currentnode;
	while (!finished)
	{
		if (graynodes.size() > 0) 
			graynodes.pop();
		if (blacknodes.find(currentnode) == blacknodes.end()) 
		{ //node not black
			blacknodes[currentnode] = true; // mark as black
			TGraphH::iterator links = mdata[1].GetFwdLinks(currentnode);
			if (links != mdata[1].NonExistant())
			{	//if there are links to other nodes from this node
				TNodeLinks::iterator node;
				for (node = links->second.begin(); 
				 node != links->second.end(); ++node)
				{ //for each node that is linked
					if (blacknodes.find(node->first) == blacknodes.end())
					{	//node not black
						unsigned k = 2;
						unsigned previous = previousnode[currentnode];
						bool nth_order_connected = true;
						double mutualinformation = 0.0;
						while (k <= markov_order)
						{
							if ( (!mdata[k].CheckLink(previous, node->first)) 
								&& (previous) ) //both previous nonzero, and 
												//no links from it to node
							{
								nth_order_connected = false;
								break;
							}
							mutualinformation +=
							1.0 * node->second / mdata[k].count
				  			* 
				  			logf(
					   		 (1.0 * node->second) * mdata[k].count
					   		 /
					   		 (
					   		 1.0 * mdata[k].CountFwdLinksStrength(previous)
						    	 * mdata[k].CountBckLinksStrength(node->first)
					   		 )
					  		);
			
					  		previous = previousnode[previous];		
							k++;
						} //check if its fully n-th order connected.
						double distance = 1.0 / mutualinformation;
						if (nth_order_connected)
						{ //add to gray nodes
							
							graynodes.push(
							  CGrayLink(
								node->first,							
								(distance) + pathlengths[currentnode]
							  )
							);
							if (pathlengths.find(node->first)
								!= pathlengths.end()) 
							{ // already found node, update min path?
								if ((distance + pathlengths[currentnode])
									< pathlengths[node->first])
								{
									pathlengths[node->first] = distance
										+ pathlengths[currentnode];
								}
							}
							else { // a brand new node 
								pathlengths[node->first] = distance
									+ pathlengths[currentnode];
							}
							previousnode[node->first] = currentnode;
						}
					
					} //if linked node not blacknode 
				} //for all nodes linked to the current
			} //if not nonexistant.
		} //if currentnode not black
		//
		
		if (graynodes.size()) {
			//prepare next gray node to explore.
			next = graynodes.top();	
			//ignore suboptimal lengths.
			while (myabs(next.pathlength - pathlengths[next.node])
					> myabs(0.001 * next.pathlength))
			{
				if (graynodes.size()>1) {
					graynodes.pop();
					next = graynodes.top();
				}
				else { break; }
			}
			nodecount++;
		}
		
		//if there is such a node
		if ((graynodes.size()) && (nodecount <= TRIP_AI_MAXDEPTH))
		{
			currentnode = next.node;
			//check if we have come to any of the other keyword nodes.
			for (it = keywords.begin(); it != keywords.end(); ++it)
			{
				
				if ((currentnode == *it) && 
					(foundnodes.find(*it) == foundnodes.end()))
				{ // if we arrived to a previously not found node.
					unsigned pathnode = currentnode;
					TNodeLinks::iterator node;
					
					vector<unsigned>::size_type revOffset = results.size();
					while ( 
						   (( (node = previousnode.find(pathnode)) 
							!= previousnode.end() ))
						  && (foundnodes.find(pathnode) == foundnodes.end())
						   )
					{
						results.push_back(node->second); //push previous of
						                                 //pathnode 
						pathnode=node->second;
					}
					foundnodes[currentnode] = true; // mark node as found
					blacknodes.clear(); // clear all black nodes.
					while (graynodes.size()) graynodes.pop();
					// partial reverse.
					reverse(results.begin() + revOffset, results.end()); 
					results.push_back(currentnode); // push last node :P
					if (foundnodes.size() >= keywords.size()) 
					{
						finished = true;
					}
					//otherwise, we are not finished, begin exploring again
					nodecount = 0; //reset nodecount
					keynode = currentnode;
					graynodes.push(CGrayLink(keynode,0.0));
					break; //break the for loop. current node is setup right.
				}
			} //for each keyword	
		}
		else
		{ //fully expanded tree, but no next keyword found, 
		  //or expanded past limit
			while (graynodes.size()) graynodes.pop();
			blacknodes.clear();
			//results.push_back(keynode); //push keynode anyway
			if ((previousnode.find(keynode))->second == 0) //if nobody arrived 
				foundnodes.erase(keynode); //to it before, unmark found
			
			deadnodes[keynode] = true; // mark as dead (non-expandable)
			results.push_back(0); // add a zero to signify lack of connector
			
			// pick any node not in foundnodes as the next node to resume.
			for (it = keywords.begin(); it != keywords.end(); ++it)
			{
				if (   (foundnodes.find(*it) == foundnodes.end())
					&& (deadnodes.find(*it) == deadnodes.end())  )
				{ //node not found and not dead...
					keynode = *it; //next key node from which we begin
					foundnodes[keynode] = true;
					currentnode = keynode;
					previousnode[keynode] = 0;
					graynodes.push(CGrayLink(keynode,0.0));
					nextassigned = true;
					nodecount = 0;
					break;
				}
			}
		} //assigning next node done.
		
		if ((foundnodes.size() >= keywords.size()) || (graynodes.size() == 0))
			finished = true;
	} // done finding the path (or a partial path)
	  //return whatever results found.
	return results;
}



void CMarkov::savedata(const string& basefile)
{
	for (unsigned i = 1; i <= order; ++i)
	{
		mdata[i].SaveLinks(basefile + "." + IntToStr(i) + ".dat");
	}
}

long CMarkov::readdata(const string& basefile)
{
	long sum = 0;
	for (unsigned i = 1; i <= order; ++i)
	{
		sum += mdata[i].ReadLinks(basefile + "." + IntToStr(i) + ".dat");
	}
	internalCount = sum;
	return sum;
}

long CMarkov::CountLinks(unsigned x, unsigned r)
{
	long sum=0;
	if (r) return mdata[r].CountLinks(x);
	else
	{
		for (unsigned i = 1; i <=order; ++i)
		{
			sum += mdata[i].CountLinks(x);
		}
	}
	return sum;
}

long CMarkov::CountLinksStrength(unsigned x)
{
	long sum = 0;
	for (unsigned i = 1; i <= order; ++i)
	{
		sum += mdata[i].CountLinksStrength(x);
	}
	return sum;
}

long CMarkov::CountLinksStrength(unsigned x, unsigned r)
{
	return mdata[r].CountLinksStrength(x);
}

long CMarkov::CheckLink(unsigned x, unsigned y, unsigned r)
{
	return mdata[r].CheckLink(x,y);
}
