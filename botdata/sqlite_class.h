#include <iostream>
#include <string>
#include <sqlite3.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <vector>

using std::string;
using std::vector;
using std::stringstream;
using std::cout;
using std::endl;

template <typename To, typename From>
To convert(From f) { To t; stringstream s; s << f; s >> t; return t; } 

struct SQLiteException : public std::runtime_error
{
	SQLiteException(std::string error_message): std::runtime_error(error_message) {}
};

class SQLite
{
	sqlite3 *db;
	sqlite3_stmt *ppVm;
	const char *zTail;
	string m_Query;
	bool isDone; // basically, ready for query

	int colCount;
	long int counter; // current row number for current query

	vector<string> row;

	void cleanUp() {
		if (!isDone) {
			//char *errmsg;
			sqlite3_finalize(ppVm);
			isDone = true;
		}
	}
	void internalQuery()
	{
		colCount = 0;
		cleanUp();
		if (db)
		{
			counter = 0;
			int retres;
			retres = sqlite3_prepare_v2(db, m_Query.c_str(), -1, &ppVm, &zTail);
			if (ppVm == NULL) 
			{
				//throw SQLiteException(sqlite3_errmsg(db));
			}
			colCount = sqlite3_column_count(ppVm);
			isDone = false;
		}
		else
		{
			//throw SQLiteException("DB not initialized\n");
		}
		if (ppVm != NULL)
			GetNextResult(); // will put first result into row, (or just exec query w/o results)
	
	}
	public:
	
	long int GetCounter() { return counter; }
	SQLite(string sqdb)
	{
		int sqstate = sqlite3_open(sqdb.c_str(),  &db);
		if (sqstate != SQLITE_OK) { }// throw SQLiteException(sqlite3_errmsg(db));
		isDone = true;
		colCount = 0;
	}

	~SQLite()
	{
		cleanUp();	
	}
	
	void Query(string sql) 
	{
		m_Query = sql;
		internalQuery();
	}
	
	void QueryExec(string sql) 
	{
		Query(sql);
	}
	
	vector<string> GetNextResult()
	{
		vector<string> rrow;
		rrow.swap(row);
		if (!isDone)
		{
			++counter;
			int retres;
			while (( (retres = sqlite3_step(ppVm)) == SQLITE_BUSY ))
			{
				usleep(1);
			}	
			if (retres == SQLITE_ROW) { 
				row.reserve(colCount+1);
				for (int i = 0; i < colCount; ++i)
				{ 
					row.push_back((char *)sqlite3_column_text(ppVm,i));		
				}
			}
			else if (retres == SQLITE_DONE) { 
				cleanUp();
				m_Query = string(zTail);
				if (m_Query.find_first_not_of("\r\n\t ") != string::npos) { internalQuery(); }
			}
			//else if (retres == SQLITE_ERROR) throw SQLiteException(string("SQLITE_ERROR\n") + sqlite3_errmsg(db)); }		
			//else { }// throw SQLiteException(string("SQLITE_MISUSE\n") + sqlite3_errmsg(db)); }
		}
		return rrow;
	}
};


