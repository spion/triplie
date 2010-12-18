#include "ai/tokens.cc"
#include "ai/sqlite_class.h"


#define MAX_PROP 10

#define MAX_EPOCH 10



#define LRATE 0.01 // Learning rate parameter
#define K 0.001 // Regularization parameter used to minimize over-fitting


#define INIT 0.1

#define IDMAX 1500


using namespace std;

class proplist
{
	public:
		double prop[MAX_PROP];
	proplist()
	{
		for (unsigned k = 0; k < MAX_PROP; ++k)
		{
			prop[k] = INIT;
		}
	}
};

typedef map<unsigned, double> udmap;
typedef map<unsigned, udmap> uudmap;

double predictValue(const proplist& p1, const proplist& p2)
{
	double pres = 0.0;
	for (unsigned k = 0; k < MAX_PROP; ++k)
	{
		pres += p1.prop[k] * p2.prop[k];
	}
	return pres;
}

void trainFeatures(proplist& p1, proplist& p2)
{
}

int main()
{
	SQLite db("botdata/triplie.db");
	map<unsigned,proplist> properties;
	map<unsigned, string> words;
	uudmap assoc;

	cerr << "Reading words..." << endl;
	db.Query("SELECT * FROM dict;");
	vector<string> res;
	while ((res = db.GetNextResult()).size() > 1)
	{
		unsigned id = convert<unsigned>(res[0]);
		if (id < IDMAX)
		{
			words[id] = res[1];
		}
	}
	cerr << "Reading associations..." << endl;
	db.Query("SELECT * from assoc WHERE id1 < " + convert<string>(IDMAX) + " and id2 < " + convert<string>(IDMAX) + ";");
	while ((res = db.GetNextResult()).size() > 2)
	{
		unsigned id1 = convert<unsigned>(res[0]);
		unsigned id2 = convert<unsigned>(res[0]);
		if (id1 < IDMAX && id2 < IDMAX)
		{
			assoc[convert<unsigned>(res[0])][convert<unsigned>(res[1])] = convert<double>(res[2]);
		}
	}
	cerr << "Training SVD vectors..." << endl;
	
	for (int k = 0; k < MAX_PROP; ++k)
	{
		cerr << "Property " << k << endl;
		for (int ie = 0; ie < MAX_EPOCH; ++ie)
		{
			for (uudmap::iterator it = assoc.begin(); it != assoc.end(); ++it)
			{
				for (udmap::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
				{
					proplist& firstWordProps = properties[it->first];
					proplist& secondWordProps = properties[jt->first];
					double expectedValue = jt->second;
					double predictedValue = predictValue(firstWordProps, secondWordProps);
					double error = expectedValue - predictedValue;
					double propFirst = firstWordProps.prop[k];
					double propSecond = secondWordProps.prop[k];
					firstWordProps.prop[k] += LRATE * (error * propFirst - K * propSecond);
					secondWordProps.prop[k] += LRATE * (error * propSecond - K * propFirst);
				}
			}
		}
	}
	for (map<unsigned, proplist>::iterator it = properties.begin();
			it != properties.end(); ++it)
	{
		cout << words[it->first] << " ";
		for (int k = 0; k < MAX_PROP; ++k)
		{
			cout << it->second.prop[k] << " ";
		}
		cout << endl;
	}
	return 0;
}

