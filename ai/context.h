/*
 *      context.h
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

#include <string>
#include <vector>
#include <deque>
#include "graph.h"
#include <math.h>

#define TRIP_CONTEXT_TIMEOUT 120
//#define TRIP_MAXKEY 6



template<typename T> T abs(T n) { return n<0?0-n:n; }

using std::vector;
using std::string;
using std::deque;
using std::map;

struct CContext
{
	vector<unsigned> keywords;
	string nick;
	time_t addtime;
};


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

class CContextQueue
{
	private:
		deque<CContext> context;
		map<string, bool> conNicks;
		unsigned conMax;
		unsigned conLearn;
		unsigned conCount;
		CGraph * vertical;
		//context functions
	public:
		vector<unsigned> my_dellayed_context;
		CContextQueue() {
			conMax=10;
			conLearn=2;
			conCount=0;
		}
		void setVertical(CGraph * v) { vertical = v; }
		void push(const string& bywho, vector<unsigned>& keywords);
		void learn();
};
