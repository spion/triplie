
#ifndef _NEWMARKOV_H
#define _NEWMARKOV_H

#include <map>
#include <vector>

using std::map;
using std::vector;
using std::string;

#define MARKOV_MAXORDER 6

template<unsigned N> struct newmarkov
{ 
    typedef map<unsigned, typename newmarkov<N-1>::model> model; 
}; 
template<> struct newmarkov<0> 
{ 
    typedef unsigned model; 
};


class CMarkov
{
	private:
		newmarkov<MARKOV_MAXORDER>::model mdata;
		unsigned order; 			//order of model
		unsigned internalCount;
		bool CheckIfLinked(vector<unsigned>& words);
	public:
		CMarkov() { internalCount = 0; }
		~CMarkov() { }
		void setOrder(unsigned N) { }
		void remember(vector<unsigned>& sentence);
	
		vector<unsigned> connect (vector<unsigned>& keywords, unsigned method=0);
		vector<unsigned> dconnect(vector<unsigned>& keywords, unsigned method=0);
	
		void savedata(const string&);
		long readdata(const string&);
		unsigned count() { return internalCount; }
};

#endif
