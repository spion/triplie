/*
 *      markov.h
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

#ifndef _MARKOV_H
#define _MARKOV_H

#include "graph.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <sstream>


using std::vector;
using std::deque;
using std::map;
using std::string;
using std::stringstream;

class CMarkov
{
	private:
		map< unsigned, CGraph > mdata; //n-th order markov model
		unsigned order; 			//order of model
		unsigned internalCount;
	public:
		CMarkov();
		~CMarkov();
		void setOrder(unsigned N);
		void remember(vector<unsigned>& sentence);
		void setLink (unsigned x, unsigned y, unsigned val, unsigned r);
	
		vector<unsigned> connect(vector<unsigned>& keywords, unsigned method=0);
	
		void savedata(const string&);
		long readdata(const string&);
		long CountLinks(unsigned x, unsigned r = 0);
		long CountLinksStrength(unsigned x);
		long CountLinksStrength(unsigned x, unsigned r);
		long CheckLink(unsigned x, unsigned y, unsigned r);
		unsigned count() { return internalCount; }
};

#endif
