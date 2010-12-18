#include "../ai/sqlite_class.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

int main()
{
	SQLite db("triplie.db");
	map<unsigned, unsigned> assoc;
	map<unsigned, string> dict;
	map<unsigned, double> dictfreq;
	db.Query("select id, word, wcount from dict order by wcount desc limit 0,3000");
	vector<string> v;
	while ((v = db.GetNextResult()).size() > 2)
	{
		// v[0], v[1], v[2]
		unsigned id = convert<unsigned>(v[0]);
		dict[id] = v[1];
		dictfreq[id] = convert<double>(v[2]);
	}

	db.Query("select id1,id2, val from markov;");
	while ( ((v = db.GetNextResult()).size() > 2)) 
	{
		unsigned id1(convert<unsigned>(v[0])), 
				 id2(convert<unsigned>(v[1])),
				 val(convert<unsigned>(v[2]));
		
		if (dict.find(id1) != dict.end() && dict.find(id2) != dict.end())
		{
			assoc[id1] += val;
			assoc[id2] += val;
		}
		/*
		if (dict.find(id2) != dict.end())
		{
			assoc[id2] += val; //convert<unsigned>(v[2]);
		}
		*/
	}
	for (map<unsigned,double>::iterator it = dictfreq.begin();
		 it != dictfreq.end(); ++it)
	{
		it->second = 1.0 * assoc[it->first] / it->second;
		if (it->second < 2.0)
		{
			cout << it->first << "\t" << it->second << "\t" << dict[it->first] << endl;
		}
	}
}
