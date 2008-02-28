/*
 *      graph.cc
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

#include "graph.h"

void CGraph::AddLink(unsigned x, unsigned y, unsigned val)
{
	forward[x][y] = val;
	backward[y][x] = val;
	count += val;
}

void CGraph::DelLink(unsigned x, unsigned y)
{
	forward[x][y] = backward[y][x] = 0;
}

void CGraph::IncLink(unsigned x, unsigned y)
{
	++forward[x][y];
	++backward[y][x];
	++count;
}

unsigned CGraph::CheckLink(unsigned x, unsigned y)
{
	unsigned result = 0;
	TGraphH::iterator links = forward.find(x);
	if (links != forward.end())
	{
		TNodeLinks::iterator node = links->second.find(y);
		if (node != links->second.end())
			result += node->second;
	}
	return result;
}

unsigned CGraph::CountFwdLinks(unsigned x)
{
	unsigned result = 0;
	TGraphH::iterator links = forward.find(x);
	if (links != forward.end())
	{
		result += links->second.size();
	}	
	return result;	
}

unsigned CGraph::CountBckLinks(unsigned x)
{
	unsigned result = 0;
	TGraphH::iterator links = backward.find(x);
	if (links != backward.end())
	{
		result += links->second.size();
	}	
	return result;	
}

unsigned CGraph::CountLinks(unsigned x)
{
	return CountFwdLinks(x) + CountBckLinks(x);
}

unsigned CGraph::CountBckLinksStrength(unsigned x)
{
	unsigned result = 0;
	TNodeLinks::iterator node;
	TGraphH::iterator links = backward.find(x);
	if (links != backward.end())
	{
		for (node = links->second.begin(); node != links->second.end(); ++node)
		{
			result += node->second;
		}
	}
	return result;	
}

unsigned CGraph::CountFwdLinksStrength(unsigned x)
{
	unsigned result = 0;
	TNodeLinks::iterator node;
	TGraphH::iterator links = forward.find(x);
	if (links != forward.end())
	{
		for (node = links->second.begin(); node != links->second.end(); ++node)
		{
			result += node->second;
		}
	}
	return result;	
}

unsigned CGraph::CountLinksStrength(unsigned x)
{
	return CountFwdLinksStrength(x) + CountBckLinksStrength(x);
}

TGraphH::iterator CGraph::GetFwdLinks(unsigned x)
{
	TGraphH::iterator it = forward.find(x);
	return it;
}

TGraphH::iterator CGraph::GetBckLinks(unsigned x)
{
	TGraphH::iterator it = backward.find(x);
	return it;
}

TGraphH::iterator CGraph::NonExistant()
{
	return forward.end();
}

TGraphH::iterator CGraph::NonExistantBck()
{
	return backward.end();
}


void CGraph::SaveLinks(const string& sfile)
{
	ofstream myfile (sfile.c_str());
	TGraphH::iterator iter;
	TNodeLinks::iterator subit;
  	if (myfile.is_open()) {
		for (iter=forward.begin();iter!=forward.end();iter++)
		{
			for(subit=iter->second.begin();subit!=iter->second.end();subit++)
			{
				myfile << iter->first << " "  
					   << subit->first << " " 
					   << subit->second << endl;

			}

		}
	}
	myfile.close();
}

long int CGraph::ReadLinks(const string& sfile)
{
	long int retval; unsigned int l1; unsigned int l2; unsigned int l3;
	ifstream myfile (sfile.c_str());
	retval=0;
	//relcount=0;
	if (myfile.is_open()) {
		while (! myfile.eof() ) {
			l1=0; l2=0; l3=0;
			myfile >> l1 >> l2 >> l3;
			if ((l1>0) && (l2>0) && (l3>0)) {
				forward[l1][l2]=l3;
				backward[l2][l1]=l3;
				retval+=l3;
			}
		}
		myfile.close();
	}
	count = (unsigned)retval;
	return retval;
}
