/*
 *      graph.h
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

#ifndef _GRAPH_H
#define _GRAPH_H

#include <map>
#include <fstream>
#include <string>
#include "sqlite_class.h"

using std::ifstream;
using std::ofstream;
using std::string;
using std::endl;

typedef std::map<unsigned, unsigned > TNodeLinks;
typedef std::map<unsigned, TNodeLinks > TGraphH;

class CGraph
{
	private:
		string table_name;
		SQLite* db;
		bool InsideTransaction;
	public:
		void CloseDB() { db->CloseDB(); }
		void OpenDB() { db->OpenDB(); }
		unsigned long count;
	
		void CGraphInit(SQLite* dbf, string tblname);
		void AddLink(unsigned x, unsigned y, unsigned val);
		void IncLink(unsigned x, unsigned y);
		void DelLink(unsigned x, unsigned y);
		unsigned CheckLink(unsigned x, unsigned y);
	
		unsigned CountLinks(unsigned x);
		unsigned CountLinksStrength(unsigned x);
		unsigned CountFwdLinks(unsigned x);
		unsigned CountFwdLinksStrength(unsigned x);
		unsigned CountBckLinks(unsigned x);
		unsigned CountBckLinksStrength(unsigned x);
		TNodeLinks GetFwdLinks(unsigned node);
		TNodeLinks GetBckLinks(unsigned node);
		TGraphH::iterator NonExistant();
		TGraphH::iterator NonExistantBck();
	
		void SaveLinks(const string& sfile);
		long int ReadLinks(const string& sfile);

		void BeginTransaction();
		void EndTransaction();
};

#endif
