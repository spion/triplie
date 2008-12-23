/*
 *      graph.cc
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

#include "graph.h"
#include <sstream>


void CGraph::CGraphInit(SQLite* dbf, string tblname)
{
	db = dbf;
	table_name = tblname;
	stringstream query;
	query << "SELECT count(*) FROM " << table_name << ";";
	db->Query(query.str());
	count = convert<unsigned>(db->GetLastResult()[0]); 
}

void CGraph::AddLink(unsigned x, unsigned y, unsigned val)
{
	stringstream query;
	query << "INSERT or IGNORE INTO " << table_name
		  << " VALUES (" << x << "," << y << "," << val << ");";
	db->Query(query.str());
	//forward[x][y] = val;
	//backward[y][x] = val;
	count += val;
}

void CGraph::DelLink(unsigned x, unsigned y)
{
	stringstream query;
	query << "DELETE FROM " << table_name
		  << " WHERE id1=" << x << " AND id2=" << y << ";";
	db->Query(query.str());
	
	//forward[x][y] = backward[y][x] = 0;
}

void CGraph::IncLink(unsigned x, unsigned y, unsigned val)
{
	stringstream query;
	query << "UPDATE " << table_name << " SET val=val+" << val
		  << " WHERE id1=" << x << " AND id2=" << y << ";";
	db->Query(query.str());
	count += val;
}

unsigned CGraph::CheckLink(unsigned x, unsigned y)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT sum(val) FROM " << table_name 
		  << " WHERE (id1 = " << x << ") AND (id2 = " << y << ");"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);
	return result;
}

unsigned CGraph::CountFwdLinks(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT count(id2) FROM " << table_name 
		  << " WHERE (id1 = " << x << ");"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;	
}

unsigned CGraph::CountBckLinks(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT count(id1) FROM " << table_name 
		  << " WHERE (id2 = " << x << ");"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;	
}

unsigned CGraph::CountLinks(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT (SELECT count(id1) FROM " << table_name 
		  << " WHERE (id2 = " << x << ")) + "
		  << "(SELECT count(id2) FROM " << table_name 
		  << " WHERE (id1 = " << x << "));"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;	
}

unsigned CGraph::CountBckLinksStrength(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT sum(val) FROM " << table_name 
		  << " WHERE (id2 = " << x << ");"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;	
}

unsigned CGraph::CountFwdLinksStrength(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT sum(val) FROM " << table_name 
		  << " WHERE (id1 = " << x << ");"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;	
}

unsigned CGraph::CountLinksStrength(unsigned x)
{
	unsigned result = 0;
	stringstream query;
	query << "SELECT (SELECT sum(val) FROM " << table_name 
		  << " WHERE (id2 = " << x << ")) + "
		  << "(SELECT sum(val) FROM " << table_name 
		  << " WHERE (id1 = " << x << "));"; 
	db->Query(query.str());
	result += convert<unsigned>(db->GetLastResult()[0]);	
	return result;
}

TNodeLinks CGraph::GetFwdLinks(unsigned x)
{
	TNodeLinks n;
	stringstream query;
	query << "SELECT id2, val FROM " << table_name
		  << " WHERE (id1 = " << x << ");";
	db->Query(query.str());
	vector<string> v = db->GetNextResult();
	while (v.size() > 1)
	{
		n[convert<unsigned>(v[0])] = convert<unsigned>(v[1]);
		v = db->GetNextResult();
	}
	return n;
}

TNodeLinks CGraph::GetBckLinks(unsigned x)
{
	TNodeLinks n;
	stringstream query;
	query << "SELECT id1, val FROM " << table_name
		  << " WHERE (id2 = " << x << ");";
	db->Query(query.str());
	vector<string> v = db->GetNextResult();
	while (v.size() > 1)
	{
		n[convert<unsigned>(v[0])] = convert<unsigned>(v[1]);
		v = db->GetNextResult();
	}
	return n;
}



void CGraph::SaveLinks(const string& sfile)
{

}

long int CGraph::ReadLinks(const string& sfile)
{
	return count;
}

void CGraph::BeginTransaction()
{
	db->BeginTransaction();
}


void CGraph::EndTransaction()
{
	db->EndTransaction();
}

