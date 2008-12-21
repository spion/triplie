/*
 *      dictionary.cc
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

#include "dictionary.h"
#include <fstream>

using std::ifstream;
using std::ofstream;



int minimum(int x, int y, int z) 
{
	return x < y ? (x < z ? x : z) : (y < z ? y : z);
}

double leven(string s, string t)
{
	int ocost = 1;
   // d is a table with m+1 rows and n+1 columns
	int d[MAX_WSIZE+1][MAX_WSIZE+1];
	int i,j;
	int m = s.length()+1 < MAX_WSIZE ? s.length()+1 : MAX_WSIZE;
	int n = t.length()+1 < MAX_WSIZE ? t.length()+1 : MAX_WSIZE;
	  
   for (i = 0; i <= m; ++i)
       d[i][0] = i;
   for (i = 0; i <= n; ++i)
       d[0][i] = i;
 	for (i=1; i <= m; ++i)
 	{
		for (j=1; j <= n; ++j)
		{

			int cost = 0;
			if (s[i-1] == t[j-1]) { cost = 0; }
						else cost = ocost*2;
           d[i][j] = minimum(
                                d[i-1][j] + ocost,     // deletion
                                d[i][j-1] + ocost,     // insertion
                                d[i-1][j-1] + cost   // substitution
                            );
                            
			//cout << (int)d[i][j] << '\t';
       
		}
		//cout << endl;
	}
 
   return ((double) d[m][n] / (double) (m + n));
}


void CDictionary::CDictionaryInit(SQLite* dbf) 
{ 
	db = dbf; 
	db->Query("PRAGMA cache_size = 25000; PRAGMA temp_store = MEMORY;");
	db->Query("PRAGMA read_uncommited = True;");
	totaloccurances = 0; 
	occurances(); 
}

const string escapestring(const string& s)
{
	string wrd = s;
	string::size_type posq = wrd.find_first_of("'");
	while (posq != std::string::npos)
	{
		wrd.insert(posq,"'");
		posq += 2;
		posq = wrd.find_first_of("'",posq);
	}
	return wrd;
}

unsigned CDictionary::GetKey(const string& word) {
	db->Query(string("SELECT id FROM dict WHERE word='") + escapestring(word) + "'");
	vector<string> v = db->GetNextResult();
	if (v.size() > 0) { return convert<int>(v[0]); }
	else { return 0; }
}

void CDictionary::AddWord(const string& word, const unsigned& howmany) { 
	db->Query(string("SELECT id FROM dict WHERE word='") 
			+ escapestring(word) + "';");
	vector<string> v = db->GetNextResult();
	if (v.size() < 1) {
		db->Query(string("INSERT INTO dict (word, wcount) VALUES ('")
						+ escapestring(word) + "', 1);"); 
	}
	else
	{
		db->Query(string("UPDATE dict SET wcount = wcount + ") 
				 + convert<string>(howmany) + " WHERE id = " + v[0] + ";");
	}
	totaloccurances += howmany;
}


void CDictionary::AddWord(const unsigned& word, const unsigned& howmany, bool noInject) 
{
	string widstr = convert<string>(word); 
	if (noInject)
	{
		db->Query(string("SELECT id FROM dict WHERE id='") 
				+ widstr + "';");
		vector<string> v = db->GetNextResult();
		if (v.size() < 1) {
			db->Query(string("INSERT INTO dict (id, word, wcount) VALUES (") +
							widstr + ", '', 1);"); 
		}
		else
		{
			db->Query(string("UPDATE dict SET wcount = wcount + ") 
				 + convert<string>(howmany) + " WHERE id = " + widstr + ";");
		}
		totaloccurances += howmany;
	}
	else 
	{
		db->Query(string("INSERT or REPLACE INTO dict (id, word, wcount) VALUES (")
				 + widstr + ", '', " + convert<string>(howmany) + ");"); 
	}
}


unsigned CDictionary::count() {
	db->Query("SELECT count(id) FROM dict;");
	vector<string> v = db->GetNextResult();
	return convert<unsigned>(v[0]); 
}

unsigned CDictionary::occurances(unsigned wrd) { 
	db->Query(string("SELECT wcount FROM dict WHERE id=") 
			 + convert<string>(wrd) + ";");
	if (db->GetLastResult().size() > 0)
		return convert<unsigned>(db->GetLastResult()[0]);
	else return 0;
}

unsigned CDictionary::occurances(const string& wrd)
{
	db->Query(string("SELECT wcount FROM dict WHERE word='") 
			 + escapestring(wrd) + "';");
	if (db->GetLastResult().size() > 0)
		return convert<unsigned>(db->GetLastResult()[0]);
	else return 0;
}
unsigned CDictionary::occurances()
{
	if (totaloccurances == 0) {
		db->Query("SELECT SUM(wcount) FROM dict;");
		totaloccurances = convert<unsigned>(db->GetLastResult()[0]);
	}
	return totaloccurances;
}


unsigned int CDictionary::readwords(string wordsfile = "") {
	db->Query("SELECT COUNT(id) FROM dict;");
	return convert<unsigned>(db->GetLastResult()[0]);
}

void CDictionary::savewords(string wordsfile = "") {
}

const string CDictionary::GetWord(unsigned key) {
	db->Query(string("SELECT word FROM dict WHERE id = ") 
			 + convert<string>(key) + ";");
	if (db->GetLastResult().size() > 0) 
			return db->GetLastResult()[0];
	else { return ""; }
}


map<unsigned, map<unsigned,string> > CDictionary::FindSimilarWords(const vector<unsigned>& wordlist)
{
	db->Query("SELECT id,word FROM dict;");
	vector<string> v;
	vector<string> w_strings;
	map<unsigned, string> full_list;
	map<string, unsigned> w_list;
	map<unsigned, map<unsigned, string> > groups;
	//pass one, divide the words between in-the-list and not-in-the-list
	while (( (v = db->GetNextResult()).size() > 1 ))
	{
		unsigned index = convert<unsigned>(v[0]);
		bool alreadylisted = false;
		for (unsigned i=0; i < wordlist.size(); ++i)
		{
			if (wordlist[i] == index)
			{
				if (v[1].size() > 3)
				{
					w_strings.push_back(v[1]);
					w_list[v[1]] = index;
				}
				alreadylisted = true;
				break;
			}
		}
		if (!alreadylisted) full_list[index] = v[1];
	}
	//pass two, compare levenstein distance between all words in the list
	//with those that are in the full list. If some are not close enough
	//remove them from the full list.
	for (map<unsigned,string>::iterator it = full_list.begin();
		 it != full_list.end();)
	{
		bool close_enough = false;
		for (unsigned i=0; i < w_strings.size(); ++i)
		{
			if ((leven(it->second,w_strings[i]) < LEVEN_MAGIC_LIMIT))
			{
				close_enough = true;
				groups[w_list[w_strings[i]]][it->first] = it->second;
				break;
			}
		}
		if ((!close_enough) || (it->second.size() < 4))
		{
			full_list.erase(it++);
		}
		else { ++it; }
	}
	return groups;
}

void CDictionary::BeginTransaction()
{
	db->BeginTransaction();
}


void CDictionary::EndTransaction()
{
	db->EndTransaction();
}

void CDictionary::ClearAll() { db->Query("DELETE FROM dict;"); }


