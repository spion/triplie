//           triprotomaster.cc
//  Mon Nov 24 04:32:18 2008
//  Copyright  2008  Gorgi Kosev
//  <gorgi.kosev@gmail.com>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA

#include "triprotomaster.h"
#include "../ai/tokens.h"

TriplieMasterProto::TriplieMasterProto() 
{ 
	m_Status = Req_Idle;
	
}


bool TriplieMasterProto::Connect(string hostname, int portnumber)
{
	host = hostname; port = portnumber;
	sckConn.destroy();
	sckConn.create();
	sckConn.connect(host, port);
	return sckConn.is_valid();
}

bool TriplieMasterProto::SendRequest(string request)
{
	m_Buffer = "";
	sckConn.set_non_blocking(false);
	m_Status = Req_InProgress;
	sckConn.send(request);
	return true;
}

bool TriplieMasterProto::RequestComplete()
{
	sckConn.set_non_blocking(true);
	string buf = ""; 
	sckConn.recv(buf);
	m_Buffer += buf;
	if ((sckConn.get_state() == ECONNREFUSED) 
		|| (sckConn.get_state() == ENOTCONN))
	{
		m_Status = Req_Failed;
	}
	if (m_Buffer.find_first_of("\r\n") != string::npos)
	{
		m_Status = Req_Success;
		return true;
	}
	else
	{
		return false;
	}
}

string TriplieMasterProto::ReplyData()
{
	string::size_type wheren = m_Buffer.find_first_of("\r\n");
	if (wheren == string::npos) return "";
	string ret(m_Buffer, 0, wheren);
	wheren = m_Buffer.find_first_not_of("\r\n",wheren);
	if (wheren == string::npos)
	{
		m_Buffer = "";
	}
	else {
		m_Buffer = m_Buffer.substr(wheren);
	}
	return ret;
}



namespace TripMaster
{
	
string PrepareLearnRequest(const vector<unsigned>& words)
{
	string res = "01 "; //learn
	res += conv<string>(words.size()) + " ";
	for (unsigned int i = 0; i < words.size(); ++i)
	{
		res += conv<string>(words[i]) + string(" ");
	}
	res += "\n";
	return res;
}

string PrepareReplyRequest(const vector<unsigned>& words)
{
	string res = "03 "; //NeedReplyFor
	for (unsigned int i = 0; i < words.size(); ++i)
	{
		res += conv<string>(words[i]) + string(" ");
	}
	res += "\n";
	return res;	
}

bool VerifyAnswer(const string& answer)
{
	vector<string> tokens;
	if (answer.size() < 1) return false;
	tokenize(answer, tokens, " ");
	if (tokens.size() < 1) return false;
	unsigned cmd = conv<unsigned>(tokens[0]);
	if (cmd == 2 || cmd == 4) return true;
	return false;
}

vector<unsigned> ConvertAnswerToVector(const string& answer)
{
	vector<unsigned> v;
	vector<string> s;
	tokenize(answer, s, " ");
	//Leave out the reply command code.
	for (unsigned i = 1; i < s.size(); ++i)
	{
		v.push_back(conv<unsigned>(s[i]));
	}
	return v;
}

}


