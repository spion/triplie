#include "sqlite_class.h"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iostream>

//using std::rand;
using std::stringstream;
using std::cout;
using std::endl;
using std::cin;

template<typename T>
string toStr(T from) {
    stringstream s;
    s << from;
    return s.str();
}

int main() {
    SQLite all("../botdata/triplie.db");
    all.QueryExec("PRAGMA synchronous = OFF; PRAGMA cache_size = 6000; PRAGMA parser_trace = off;");
    string query;

    while (1) {
        string s;
        cin >> s;
        query = query + s + " ";
        if (query.find_first_of(";") != string::npos) {
            all.Query(query);
            vector<string> v = all.GetNextResult();
            while (v.size() > 0) {
                for (unsigned i = 0; i < v.size(); ++i)
                    cout << v[i] << "\t";
                cout << endl;
                v = all.GetNextResult();
            }
            cout << "Query finished" << endl;
            query = "";
        }
    }
    return 0;
}
