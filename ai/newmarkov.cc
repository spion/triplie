#include "newmarkov.h"
#include "graph.h"
#include <deque>
#include <fstream>

#define TRIP_AI_MAXDEPTH 32000

using std::deque;
using std::ifstream;
using std::ofstream;

void CMarkov::remember(vector<unsigned>& sentence)
{
	unsigned i;
	vector<unsigned> v = sentence;
	if (sentence.size() > 1)
	{
		for (i = 0; i <= MARKOV_MAXORDER; ++i)
			v.push_back(0);
		for (i = 0; i < sentence.size()-1; ++i)
		{
			mdata[v[i]]
		 	 	[v[i+1]]
		 	 	[v[i+2]]
		 	 	[v[i+3]]
		 	 	[v[i+4]]
		 	 	[v[i+5]]+=1;
		 	 	++internalCount;
		}
	}
}

bool CMarkov::CheckIfLinked(vector<unsigned>& words)
{

	if (!words.size()) return true;
	newmarkov<MARKOV_MAXORDER>::model::iterator 
		it = mdata.find(words[0]);
	if (it != mdata.end())
	{
		if (words.size() < 2) return true;
		newmarkov<MARKOV_MAXORDER-1>::model::iterator
			it2 = it->second.find(words[1]);
		if (it2 != it->second.end())
		{
			if (words.size() < 3) return true;
			newmarkov<MARKOV_MAXORDER-2>::model::iterator
				it3 = it2->second.find(words[2]);
			if (it3 != it2->second.end())
			{		
				if (words.size() < 4) return true;
				newmarkov<MARKOV_MAXORDER-3>::model::iterator
					it4 = it3->second.find(words[3]);
				if (it4 != it3->second.end())
				{
					if (words.size() < 5) return true;	
					newmarkov<MARKOV_MAXORDER-4>::model::iterator
						it5 = it4->second.find(words[4]);
					if (it5 != it4->second.end())
					{
						if (words.size() < 6) return true;	
						newmarkov<MARKOV_MAXORDER-5>::model::iterator
							it6 = it5->second.find(words[5]);
						if (it6 != it5->second.end())
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}


vector<unsigned> CMarkov::dconnect(vector<unsigned>& keywords, unsigned method)
{
	return connect(keywords,method);
}

vector<unsigned> CMarkov::connect(vector<unsigned>& keywords, unsigned method)
{
	vector<unsigned>::iterator it;
	unsigned markov_order = method + 1;
	if (markov_order > MARKOV_MAXORDER) markov_order = MARKOV_MAXORDER;
	
	map<unsigned, bool> foundnodes; //found nodes
	map<unsigned, bool> blacknodes; //nodes already explored
	map<unsigned, bool> deadnodes;  //non-expandable nodes.
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
			newmarkov<MARKOV_MAXORDER>::model::iterator 
				links = mdata.find(currentnode);
			if (links != mdata.end())
			{	//if there are links to other nodes from this node
				newmarkov<MARKOV_MAXORDER-1>::model::iterator node;
				for (node = links->second.begin(); 
				 node != links->second.end(); ++node)
				{ //for each node that is linked
					if (blacknodes.find(node->first) == blacknodes.end())
					{	//node not black
						unsigned k = 2;
						unsigned previous = previousnode[currentnode];
						vector<unsigned> linkcheck;
						linkcheck.push_back(node->first);
						linkcheck.push_back(currentnode);
						while (k <= markov_order)
						{
							if (!previous) break;
							linkcheck.push_back(previous);
							previous = previousnode[previous];
							++k;
						}
						std::reverse(linkcheck.begin(),linkcheck.end());
						bool nth_order_connected = 
							CheckIfLinked(linkcheck);
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


void CMarkov::savedata(const string& sfile)
{
	ofstream f(sfile.c_str());
	for (newmarkov<MARKOV_MAXORDER>::model::iterator i1
			= mdata.begin(); i1 != mdata.end(); ++i1)
	for (newmarkov<MARKOV_MAXORDER-1>::model::iterator i2
			= i1->second.begin(); i2 != i1->second.end(); ++i2)
	for (newmarkov<MARKOV_MAXORDER-2>::model::iterator i3
			= i2->second.begin(); i3 != i2->second.end(); ++i3)
	for (newmarkov<MARKOV_MAXORDER-3>::model::iterator i4
			= i3->second.begin(); i4 != i3->second.end(); ++i4)
	for (newmarkov<MARKOV_MAXORDER-4>::model::iterator i5
			= i4->second.begin(); i5 != i4->second.end(); ++i5)
	for (newmarkov<MARKOV_MAXORDER-5>::model::iterator i6
			= i5->second.begin(); i6 != i5->second.end(); ++i6)
		f << i1->first << " " 
		  << i2->first << " "
		  << i3->first << " "
		  << i4->first << " "
		  << i5->first << " " 
		  << i6->first << " "
		  << i6->second << endl;
	f.close();
}

long int CMarkov::readdata(const string& sfile)
{
	ifstream f(sfile.c_str());
	unsigned i1, i2, i3, i4, i5, i6, val;
	if (f.is_open())
	{
		while (!f.eof())
		{
			f >> i1 >> i2 >> i3 >> i4 >> i5 >> i6 >> val;
			mdata[i1][i2][i3][i4][i5][i6] = val;
			internalCount += val;
		}
		f.close();
	}
	return internalCount;
}
