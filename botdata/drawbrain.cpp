#define NDEBUG 1

#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <map>
#include "zpr.h"
#include "sqlite_class.h"

#define pi M_PI


#define mod_date 20081218

#define szPoint 4.0

using namespace std;

typedef map<unsigned, map<unsigned, unsigned> > Graph;
typedef map<unsigned, unsigned> GraphLink;


unsigned maxN;
unsigned maxV;
unsigned avgV;
unsigned argvmax;

int sgn(double x) { return x < 0?-1:1; }

void drawCircle(float x, float y, float z, float r)
{
/*
 	
	glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(r, 5, 5);
    glPopMatrix();
*/
}

void drawDot(float x, float y, float z)
{
	glVertex3f(x,y,z);

//	drawCircle(x,y,z, 0.002);
}

void drawbrain();

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawbrain();
    glutSwapBuffers();
}



void CompileList(Graph& rels)
{
	double rBegin;
	Graph::iterator it;
	GraphLink::iterator itl;

	double alpha;
	double beta;
	double rSize = 0.25;

	double rSlices = maxN / log(1.0*maxN);
	int angleSlices = (int)(sqrt(rSlices));

	double rSlice = 0.7 / (maxN/rSlices);
	double alphaSlice = 2*pi / (angleSlices + 1);
	double betaSlice = 2*pi / (angleSlices + 1);
	
	glNewList(123, GL_COMPILE);
	glBegin(GL_LINES);
	for (it = rels.begin(); it != rels.end(); ++it)
	{
		for (itl = it->second.begin(); itl != it->second.end(); ++itl)
		{
			if ((itl->first > argvmax) || (it->first > argvmax)) continue;
			double ratio = (itl->second > 1 ? 0.3 : 0.1);
			ratio = (itl->second > 2 ? 1.0 : 0.5);
			glColor3f(0.9 * ratio, 0.2 + 0.8 * itl->first / argvmax, 0.2 + 0.8 * it->first / argvmax );

			int i = it->first;
			alpha = (i % angleSlices + 1) * alphaSlice;
			beta =  ((i / angleSlices) % angleSlices + 1) * betaSlice;
			rBegin = (i / rSlices) * rSlice + rSize;

			glVertex3f(rBegin * sin(beta) * cos(alpha), rBegin * sin(beta) * sin(alpha), rBegin * cos(beta));


			i = itl->first;
			alpha = (i % angleSlices + 1) * alphaSlice;
			beta =  ((i / angleSlices) % angleSlices + 1) * betaSlice;
			rBegin = (i / rSlices) * rSlice + rSize;

			glVertex3f(rBegin * sin(beta) * cos(alpha), rBegin * sin(beta) * sin(alpha), rBegin * cos(beta));


		}
	}
	glEnd();
	
    glColor3f(0.8, 0.9, 1);

	
	glPointSize(szPoint);
	glBegin(GL_POINTS);
	for (unsigned i = 1; i <= maxN; ++i)
	{
		if (i > argvmax) break;
		alpha = (i % angleSlices + 1) * alphaSlice;
		beta =  ((i / angleSlices) % angleSlices + 1) * betaSlice;
		rBegin = (i / rSlices) * rSlice + rSize;
		double x = rBegin * sin(beta) * cos(alpha);
		double y = rBegin * sin(beta) * sin(alpha);
		double z = rBegin * cos(beta);
		drawDot(x,y,z);
		//cout << "Word at radius: (" << sgn(x) <<","<< sgn(y) <<","<< sgn(z) <<") "<< sqrt (x*x + y*y + z*z) << endl;
	}
	glEnd();
	glEndList();

}

void drawbrain()
{
	glCallList(123);
	//srand(time(0));
	
}


int main(int argc, char* argv[])
{
	maxN = 0;
	maxV = 0;
	avgV = 0;
	argvmax = 200000;
	if (argc > 1) argvmax = atol(argv[1]);
	string strmax = argv[1];
	SQLite db("triplie.db");
	db.Query("SELECT id1,id2,val FROM assoc WHERE (id1 < " + strmax + ") AND (id2 < " + strmax + ");");
    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Triplie's Brain");
    glutDisplayFunc(display);
    glClearColor (0, 0, 0.1, 0.1);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D(-1, 1, -1, 1);
	zprInit();
	zprSelectionFunc(drawbrain);
	glEnable(GL_POINT_SMOOTH | GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	
	{
	vector<string> v;
	Graph rels;
	cout << "Reading graph..." << endl;
	while ((v = db.GetNextResult()).size() > 2)
	{
		unsigned r1, r2, val;
		r1 = convert<unsigned>(v[0]);
		r2 = convert<unsigned>(v[1]);
		val = convert<unsigned>(v[2]);
		if ((r1 !=0) && (r2 != 0) && (val != 0))
		{
			rels[r1][r2] += val;
			if (r1 > maxN) maxN = r1;
			if (r2 > maxN) maxN = r2;
			if (val > maxV) maxV = val;
			avgV += val;
		}
	}
	avgV /= maxN;
	if (argvmax < maxN) maxN = argvmax;
	cout << "Compiling GL list..." << endl;
	CompileList(rels);
	}
    glutMainLoop();
    return 0;
}

