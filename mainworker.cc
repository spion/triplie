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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


#include "protocol/triproto.h"
#include "ai/ai.hpp"



void forktobg() {
	int i;
	i=fork();
	if (i<0) exit(1); /* fork error */
	if (i>0) exit(0); /* parent exits */
	setsid();
	for (i=getdtablesize();i>=0;--i) close(i);
	i=open("/dev/null",O_RDWR); /* open stdin */
	int k = dup(i); /* stdout */
	k = dup(i); /* stderr */
	#ifdef linux
	int lfp; 
	char fstr[10];
	umask(027);
	lfp=open("triplie.lock",O_RDWR|O_CREAT,0640);
	if (lfp<0) exit(1);
	if (lockf(lfp,F_TLOCK,0)<0) exit(0);
	sprintf(fstr,"%d\n",getpid());
	k = write(lfp,fstr,strlen(fstr));
	#endif
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
}


int main(int argc, char** argv)
{
	forktobg();
	srand(time(0));
	int ProtoPort = PROTO_PORT;
	if (argc > 1) {
		ProtoPort = convert<int>(argv[1]);
	}
	AI tai("botdata/triplie.db");
	tai.readalldata();
	tai.maxpermute(TRIP_AI_MAXPERMUTATIONS/2);
	while (true)
	{
		TriplieWorkProto* twp = new TriplieWorkProto(&tai, ProtoPort);
		delete twp;
		sleep(1);
	}

}
