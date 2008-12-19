//           triprotomaster.h
//  Mon Nov 24 04:32:18 2008
//  Copyright  2008  Gorgi Kosev
//  <gorgi.kosev@gmail.com>

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef _TRIPROTOMASTER_H
#define _TRIPROTOMASTER_H

#include <string>
#include <vector>
#include <sstream>
#include "ClientSocket.h"

enum RequestStatus { Req_InProgress, Req_Success, Req_Failed, Req_Idle, Req_Dead };

using std::string;
using std::vector;
using std::stringstream;

template <typename To, typename From>
To conv(From f) { 
	To t; stringstream s; s.precision(5); s << f; s >> t; return t; 
} 

class TriplieMasterProto
{
	private:
		RequestStatus m_Status;
		string m_Buffer;
		Socket sckConn;
		string host; int port;
	public:
		TriplieMasterProto();
		void SetHostPort(const string& h, int p) { host = h, port = p; }
		bool Connect(string, int);
		bool Connect() { return Connect(host, port); }
		bool IsConnected() { return sckConn.is_valid(); }
		bool SendRequest(string);
		bool RequestComplete();
		string ReplyData();


};

namespace TripMaster {
	string PrepareLearnRequest(const vector<unsigned>& words);
	string PrepareReplyRequest(const vector<unsigned>& words);
	bool VerifyAnswer(const string& answer); 
	vector<unsigned> ConvertAnswerToVector(const string& answer);
}

#endif
