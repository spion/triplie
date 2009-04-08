#include "../ai/sqlite_class.h"

#include <fstream>
#include <iostream>

using std::ifstream;
using std::cout; using std::endl;

template <typename From, typename To> To convert(From f) { To t; stringstream s; s << f; s >> t; return t; } 

int main(int argc, char** argv)
{
	SQLite db("triplie.db");
	db.QueryExec("CREATE TABLE if not exists markov (id1, id2, id3, id4, id5, id6, val, PRIMARY KEY (id1,id2,id3,id4,id5,id6));");
	db.QueryExec("CREATE TABLE if not exists dict (id INTEGER PRIMARY KEY, word, wcount);");
	db.QueryExec("CREATE TABLE if not exists assoc (id1, id2, val, PRIMARY KEY (id1, id2));");
	//db.QueryExec("DELETE FROM markov; DELETE FROM dict; DELETE FROM assoc; VACUUM;");
	int k = 0;
	
	
	ifstream f("rels");
	k = 0;
	cout << "Inserting into markov table..." << endl;
	db.QueryExec("BEGIN;");
	while (f && !f.eof())
	{
		++k;
		int w1, w2, w3, w4, w5, w6, val;
		f >> w1 >> w2 >> w3 >> w4 >> w5 >> w6 >> val;
		db.QueryExec(string("INSERT or REPLACE INTO markov VALUES (") 
					+ convert<int,string>(w1) + ","
					+ convert<int,string>(w2) + ","
					+ convert<int,string>(w3) + ","
					+ convert<int,string>(w4) + ","
					+ convert<int,string>(w5) + ","
					+ convert<int,string>(w6) + ","
					+ convert<int,string>(val) + ");");
		if (k%1000 == 0) cout << k << " inserts in markov table so far..." << endl; 
	}	
	db.QueryExec("END;");
	
	cout << "Markov table done, inserting into assoc table..." << endl;
	ifstream fv("relsv.dat");
	k = 0;
	db.QueryExec("BEGIN;");
	while (fv && !fv.eof())
	{
		++k;
		int w1, w2, val;
		fv >> w1 >> w2 >> val;
		db.QueryExec(string("INSERT or REPLACE INTO assoc VALUES (")
					+ convert<int,string>(w1) + ","
					+ convert<int,string>(w2) + ","
					+ convert<int,string>(val) + ");");
		if (k%1000 == 0) cout << k << " inserts in assoc table so far..." << endl; 
	}
	db.QueryExec("END;");

	cout << "Assoc table done, inserting into dict table..." << endl;
	
	db.QueryExec("BEGIN;");
	ifstream fd("words.dat");	
	k = 0;
	while (fd && !fd.eof())
	{
		++k;
		int cnt; string wrd;
		fd >> wrd >> cnt;
		string::size_type posq = wrd.find_first_of("'");
		while (posq != std::string::npos)
		{
			wrd.insert(posq,"'");
			posq += 2;
			posq = wrd.find_first_of("'",posq);
		}
		db.QueryExec(string("INSERT or REPLACE INTO dict VALUES (")
				+ convert<int,string>(k) + ", "
				+ "'" + wrd + "', "
				+ convert<int,string>(cnt) + ");");	
		if (k%1000 == 0) cout << k << " inserts in dictionary table so far..." << endl;
	}
	db.QueryExec("END;");	
	cout << "Creating word index...." << endl;
	db.QueryExec("CREATE INDEX IF NOT EXISTS wordindex ON dict (word);");
	if (argc < 2)
	{
		cout << "Creating assoc one index...." << endl;
		db.QueryExec("CREATE INDEX if not exists associndex_one ON assoc(id1);");
		cout << "Creating assoc two index...." << endl;
		db.QueryExec("CREATE INDEX if not exists associndex_two ON assoc(id2);");
		cout << "Creating all markov indeces...." << endl;
		db.QueryExec(" \
		create index if not exists markov_i_1_2 on markov(id1,id2); \
		create index if not exists markov_i_2_3 on markov(id2,id3); \
		");
	}
	else
	{
		cout << "Skipped index creation" << endl;
	}
	cout << "All done!" << endl;
	return 0;
}
