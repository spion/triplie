//DISCARDED#3
		float tmpscore = 0;
		/*if (it>0) {
			
			tmpscore += markov.CountLinks(keywords[it-1],1) * 
						markov.CheckLink(keywords[it-1],keywords[it],1)
					   / (1.0
						  + markov.CountLinksStrength(keywords[it-1],1)
						 );
		}*/
		/*if (it>1) {
			tmpscore += markov.CountLinks(keywords[it-1],2) * 
						markov.CheckLink(keywords[it-1],keywords[it],2)
					   / (1.0
						  + markov.CountLinksStrength(keywords[it-1],2)
						 );
		}*/

/*--------------------------------------*/

void AI::prune_vertical_nonkeywords()
{
	symbolmap::iterator it;
	symbolnode::iterator jt;
	unsigned cnt = 0;
	for (it = rel_vert.begin(); it != rel_vert.end(); ++it)
	{
		if (scorekeyword(dictionary.GetWord(it->first)) < 0)
		{
			rel_vert.erase(it);
			++cnt;
		}
		else
		{
			for (jt = it->second.begin(); jt != it->second.end(); ++jt)
			{
				if (scorekeyword(dictionary.GetWord(jt->first)) < 0)
				{
					it->second.erase(jt);
					++cnt;
				}
			}
		}
	}
	cout << "Pruned " << cnt << " words from vertical relations" << endl;
}




//REFACTOR: Move to CMarkovLang. Will work with indexes and markov level.
//Rewrite to handle indexes and lower level internal CMarkovLang functions.
//NOTE: will need a rewrite to sdijkstra, where the normal dijkstra is sdijkst
//of markov level 1.
//NOTE: evaltype will probably become a function of CAIStatistics ??? 
//const double& ?
const string AI::dijkstra(const string& keywrd1,const string& keywrd2, 
						  const unsigned int& deep, const int& evaltype) {
	unsigned cw1 = dictionary.GetKey(keywrd1);
	unsigned cw2 = dictionary.GetKey(keywrd2);
	long int depthleft=deep;

	if ((cw1) && (cw2)) {
		bool cloop = true;
		map<unsigned int, map<unsigned int, unsigned int> >::iterator mp1;
		map<unsigned int, unsigned int>::iterator it2;
		map<unsigned int, short int> nblack; //unsigned int marked as black.
		map<unsigned int, unsigned long int> ngraypath; //subjective path length
		map<unsigned int, unsigned long int>::iterator temppathsize;
		map<unsigned int, unsigned int>::iterator ngpathiter;
		list<long> ngray; // gray FIFO queue
		list<long> ngraytmp; //unsorted temp fifo
		list<long>::iterator gonext,gomin;
		string resultstr="";
		map<unsigned int, unsigned int> nprev; // this will be used to check if
		                    			// something is already marked as grey
		ngray.clear();
		ngray.push_back(cw1); // start with a gray w1 node.
		ngraypath[cw1] = 0;
		gonext = ngray.begin();


		while ((cloop) && (depthleft>0))
		{
			if (nblack.find(cw1)==nblack.end()) // if the node not marked black
			{
				mp1 = rel_next.find(cw1); // find nodes attached to cw1 and
									  	  // put them in the map mp1->second
				nblack[cw1]=1;	 //mark as black
				if (mp1->second.size()>0) // if the node is not a dead end
				{
					// clear temporary queue.
					ngraytmp.clear();
					for (it2=mp1->second.begin();it2!=mp1->second.end(); it2++)
					{

						//mark all nodes it goes to as gray and show that they
						//expand from cw1. Unless they are black already.
						//Relax the paths if needed
						if (nblack.find(it2->first)==nblack.end())
						{ // it2->first is the next word.
							temppathsize = ngraypath.find(it2->first);
							if ( temppathsize != ngraypath.end())
							{
								if (temppathsize->second > (4+ ngraypath[cw1]+ 
															it2->second))
								{   //we found smaller path size for it2->first.
									temppathsize->second = 4+ ngraypath[cw1]+ 
														it2->second;
									// tell the gray temp world.
									ngraytmp.push_back(it2->first); 
									nprev[it2->first]=cw1; // update backpath.
								}
							}
							else { // this is the first path.
								ngraypath[it2->first] = 4+ ngraypath[cw1]+ 
													it2->second;
								ngraytmp.push_back(it2->first);
								nprev[it2->first]=cw1;

							}
						}
						//otherwise, no need to grey a black.
					} // end of marking.

					ngraytmp.sort(); //we need to sort temp gray nodes HERE.

				} // node not dead-end
				--depthleft; //we just expanded and blacked one.
			} //node not black - end.
			// node is black or not, but is still in gray. Remove.
			ngray.erase(gonext);
			//add our new grays.
			ngray.merge(ngraytmp);
			ngray.unique(); //remove duplicate grays.
			//now set-up the next node to expand.
			//Unlike in BFS, we must find the node with the minimal path!
			gonext=ngray.begin(); gomin=gonext;
			while (gonext!=ngray.end()) {
				//cout << backdict[*gonext] << "," ;
				if (ngraypath[*gomin] > ngraypath[*gonext]) { gomin=gonext; }
				++gonext;
			}
			//cout << endl;
			gonext=gomin; //found the node with minimal path.
			//Setup to explore it,
			//unless there are no more nodes of course.
			if (gonext!=ngray.end())
			{
				cw1=*gonext;
				//Maybe the next node to expand is cw2?!
				//In that case there is no need to expand it. END.
				if (cw1==cw2) { cloop=false; }
			}
			else { depthleft=0; }
		}

		//we are finished with our markings, now lets see if we have
		//a path.
		if (cloop) resultstr = "";
		else {
			while (( (it2=nprev.find(cw2)) != nprev.end() ))
			{
				resultstr=dictionary.GetWord(cw2)+" "+resultstr;
				cw2=it2->second;
				//note that for cw1 there is no nprev element
			}
		}
		return resultstr;
	}
	else { return ""; } // words missing
}
	//return log (s
	//	logf(relcount * 1.0 / relationoccurances(wrd))
	//	/
	//	(logf(2.0) * logf(dictind - 0.9))
	//);
