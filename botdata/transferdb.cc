// (C) Gorgi Kosev a.k.a. Spion, John Peterson. License GNU GPL 3.
#include <fstream>
#include <iostream>
#include <getopt.h>
#include "../ai/sqlite_class.h"
#include "../common.h"
using std::ifstream;
using std::cout; using std::endl;

template <typename From, typename To> To convert(From f) { To t; stringstream s; s << f; s >> t; return t; } 

string d = "triplie.db";
int opt[] = {'h', 'V', 'd'};
string args = "hVd:";
string optl[] = {"help", "version", "database"};
void usage() {
	fprintf(stdout, "Usage: tdb [-h] [-V] [-d database]\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay this message.\n"
		"\t\033[1m-%c, --%s\033[0m\tdisplay version.\n"
		"\t\033[1m-%c, --%s\033[0m\tdatabase file. \033[1;30mex: '%s' (default).\033[0m\n",		
		opt[0], optl[0].c_str(),
		opt[1], optl[1].c_str(),
		opt[2], optl[2].c_str(), d.c_str());
}
bool get_arg(int argc, char** argv) {
	u32 i = 0;
	struct option longopts[] = {
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	no_argument,		NULL,	opt[i++]},
		{optl[i].c_str(),	required_argument ,	NULL,	opt[i++]},
		{NULL,				0,					NULL,	0}
	};
	int c;
	vector<string> v_tmp;
	while ((c = getopt_long(argc, argv, args.c_str(), longopts, 0)) != -1) {
		switch (c) {
		case 'V':
			log("%s %s\n", VERDATE, VER);
			return true;
		case 'h':
			usage();
			return true;
		case 'd':
			d = optarg;
			break;
		default:
			fprintf(stderr, "unknown option: %c\n", c);
			break;
		}
	}
	return false;
}

int main(int argc, char** argv)
{
	if (get_arg(argc, argv)) return 0;
	cout << "Using " << d << endl;
	SQLite db(d);
	db.QueryExec("CREATE TABLE if not exists markov (id1, id2, id3, id4, id5, id6, val, PRIMARY KEY  (id1,id2,id3,id4,id5,id6))");
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
	db.QueryExec("CREATE INDEX IF NOT EXISTS wordindex ON dict (word);");
	if (argc < 2)
	{
		cout << "Creating all markov indeces...." << endl;
		db.QueryExec("create index if not exists markov_i_2_3 on markov(id2,id3);");
	}
	else
	{
		cout << "Skipped index creation" << endl;
	}
	cout << "All done!" << endl;
	return 0;
}
