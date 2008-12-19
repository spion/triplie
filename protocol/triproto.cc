#include "triproto.h"
#include <iostream>

using std::cout; using std::endl;

TriplieWorkProto::TriplieWorkProto(AI* ai, int ProtoPort)
{
	tai = ai;
	doWork = true;
	gotResponse = false;
	sckListen = new ServerSocket(ProtoPort);
	sckAccept = new ServerSocket();
	//sckListen->set_non_blocking(true);
	//sckAccept->set_non_blocking(true);
	sckListen->accept(*sckAccept);
	waitrequests();
}
TriplieWorkProto::~TriplieWorkProto()
{
	delete sckListen;
	delete sckAccept;
}

void TriplieWorkProto::sendcmd(string cmd)
{
	sckAccept->send(cmd);
	gotResponse = false;
}
void TriplieWorkProto::waitrequests()
{
	bool StillRunning = true;
	while (StillRunning)
	{
	usleep(50000);
	string req;
	if (sckAccept->recv(req) < 0) break;
	if ((sckAccept->get_state() == ECONNREFUSED)
		|| (sckAccept->get_state() == ENOTCONN)) { break; }
	if (!(sckAccept->is_valid())) { break; }
	if (sckAccept->get_state()) { cout << sckAccept->get_state() << endl; } 
		//cout << req << endl;
	vector<string> tokens;
	tokenize(req, tokens, " ");
	if (tokens.size() > 0)
	{
		int cmd = convert<string,int>(tokens[0]);
		int len;
		string response;
		switch (cmd)
		{
			case 1:	
				// LearnMarkov : 01 <N> <id1> <id2> <id3> ... <idN>
				if (tokens.size() > 2)
				{
					len = convert<string, int>(tokens[1]);
					doSyncDB(subtokstring(tokens, 2, len, " "));
					sendcmd("02 \n");
				}
				else {
					sendcmd("05 \n");
				}
				break;
			case 3: 
				// Request best answer : 03 <id1> <id2> <idN> - Connect "id1 id2" to idN
				if (tokens.size() > 3)
				{
					unsigned i = 0;
					vector<string> keywordlist;
					while (++i < tokens.size())
					{
						keywordlist.push_back(tokens[i]);
					}
					string response = getMarkovConnecture(keywordlist);
					response = string("04") + response + " \n";
					sendcmd(response);
				}
				else {
					sendcmd("05 \n");
				}			
				break;
			case 6: // Die
				doDie();
				sendcmd("02 \n"); //OK
				StillRunning = false;
				break;
			case 7: // Inject Markov Row
					// 07 <id1> <id2> <id3> <id4> <id5> <id6> <value>
				if (tokens.size() > 6)
				tai->InjectMarkov(subtokstring(tokens,1,tokens.size(),","));
				sendcmd("02 \n"); //OK
				break;
			case 8: // Begin Markov Transaction
				tai->BeginMarkovTransaction();
				break;
			case 9: // End Markov Transaction
				tai->EndMarkovTransaction();
			case 10: // Inject Dictionary Row
				// 10 <id> <wcount>
				if (tokens.size() > 2)
				tai->InjectWord(
							 convert<string,unsigned>(tokens[1]), 
							 convert<string,unsigned>(tokens[2])
							);
				sendcmd("02 \n"); //OK
				
			case 11: // Begin Dictionary Transacion
				tai->BeginDictionaryTransaction();
				sendcmd("02 \n"); //OK
				break;
			case 12: // End Dictionary Transactrion
				tai->EndDictionaryTransaction();
				sendcmd("02 \n"); //OK
				break;
			case 20: // BootstrapDB
				tai->BootstrapDB();
				sendcmd("02 \n"); //OK
				break;
			case 30: // TODO: settings
				break;
		}
	}
		
	}
}

string TriplieWorkProto::getMarkovConnecture(const vector<string>& temp)
{
	tai->setnumericdatastring(temp);
	tai->connectkeywords(1); 
	string answer = tai->getnumericdatastring();
	return answer;
}

void TriplieWorkProto::doSyncDB(const string& syncdata) 
{
	vector<string> numdata;
	tokenize(syncdata,numdata," ");
	tai->setnumericdatastring(numdata);
	tai->learnonlymarkov();
}

