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
