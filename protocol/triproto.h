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

#include "ServerSocket.h"
#include <string>
#include <sstream>
#include "../ai/tokens.h"
#include "../ai/ai.hpp"

using std::string;

#define PROTO_PORT 6991

template<typename From, typename To> 
To convert(const From& f) { 
	stringstream s; 
	To t; 
	s << f; 
	s >> t; 
	return t; 
} 

class TriplieWorkProto
{
	private:
		AI* tai;
		ServerSocket* sckListen;
		ServerSocket* sckAccept;
		bool doWork, gotResponse;
		void sendcmd(string cmd);
		void waitrequests();
	
		void doSyncDB(const string& syncdata);
		string getMarkovConnecture(const vector<string>&) ;
		void doSave() { }
		void doDie() { }
		
	public:
		TriplieWorkProto(AI* xai, int);
		~TriplieWorkProto();

};
