/*
 *  Copyright (C) Gorgi Kosev a.k.a. Spion
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string>
#include <sqlite3.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <typeinfo>

#ifndef _SQLITE_CLASS_H
#define _SQLITE_CLASS_H

#ifndef SQLITE_DEFAULT_TIMEOUT
#define SQLITE_DEFAULT_TIMEOUT 10000
#endif

using std::string;
using std::vector;
using std::stringstream;

template <typename To, typename From>
To convert(From f) { 
	To t; stringstream s; s.precision(5); s << f; s >> t; return t; 
} 

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
	bool InsideTransaction;
	string dbf;
	
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
			retres = sqlite3_prepare(db, m_Query.c_str(), -1, &ppVm, &zTail);
			if (ppVm == NULL) 
			{
				throw SQLiteException(m_Query + " -- At InternalQuery:" + sqlite3_errmsg(db));
			}
			colCount = sqlite3_column_count(ppVm);
			isDone = false;
		}
		else
		{
			throw SQLiteException("DB not initialized\n");
		}
		if (ppVm != NULL)
			GetNextResult(); // will put first result into row, (or just exec query w/o results)
	}

	public:
	void OpenDB(string sqdb = "")
	{
		if (sqdb.size() > 0) dbf = sqdb;
		sqlite3_enable_shared_cache(1);
		int sqstate = sqlite3_open(dbf.c_str(),  &db);
		if (sqstate != SQLITE_OK) 
			throw SQLiteException(sqlite3_errmsg(db) + string(" (at OpenDB)"));
		/*
		sqlite3_enable_load_extension(db,1);
		if (sqlite3_load_extension(db,"./libsqlitefunctions.so",0,0))
		{
			if (sqlite3_load_extension(db,"./ai/libsqlitefunctions.so",0,0))
			{
				//throw SQLiteException("Extension load failed");
			}
		}*/
		sqlite3_busy_timeout(db, SQLITE_DEFAULT_TIMEOUT);
		isDone = true;
		colCount = 0;
	}	
	void CloseDB()
	{
		cleanUp();
		if (db)
		{
			sqlite3_close(db);
		}
	}

	long int GetCounter() { return counter; }
	SQLite(string sqdb)
	{
		OpenDB(sqdb);
		InsideTransaction = false;
	}

	~SQLite()
	{
		CloseDB();	
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
			if (( (retres = sqlite3_step(ppVm)) == SQLITE_BUSY ))
			{
				throw SQLiteException(m_Query + " -- At GetNextResult(1): " + sqlite3_errmsg(db));
			}
			if (retres == SQLITE_ROW) {
				row.reserve(colCount+1);
				for (int i = 0; i < colCount; ++i)
				{ 
					const unsigned char * t = sqlite3_column_text(ppVm,i);
					if (t) row.push_back((char *)t);
					else { 
						row.push_back(convert<string>(sqlite3_column_double(ppVm,i))); 
					}		
				}
			}
			else if (retres == SQLITE_DONE) { 
				cleanUp();
				if (zTail) { m_Query = string(zTail); }
				else { m_Query = ""; }
				if (m_Query.find_first_not_of("\r\n\t ") != string::npos) { internalQuery(); }
			}
			else { throw SQLiteException(m_Query + " -- At GetNextResult(2): " + sqlite3_errmsg(db)); } 
		}
		return rrow;
	}
	vector<string> GetLastResult() { return row; }
	void BeginTransaction() { 
		if (!InsideTransaction)
		{
			Query("BEGIN;"); 
			InsideTransaction = true;
		}
	}
	void EndTransaction() {
		if (InsideTransaction)
		{
			Query("END;");
			InsideTransaction = false;
		}
	}
};

#endif

