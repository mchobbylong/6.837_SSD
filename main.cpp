#ifdef WIN32
#include <windows.h>
#endif

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>

#ifdef WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif
#include <vecmath.h>

#include "modelerapp.h"
#include "ModelerView.h"

using namespace std;

int main( int argc, char* argv[] )
{
	if( argc < 2 )
	{
		cout << "Usage: " << argv[ 0 ] << " PREFIX1 PREFIX2 ..." << endl;
		cout << "For example, if you're trying to load data/cheb.skel, data/cheb.obj, and data/cheb.attach, run with: " << argv[ 0 ] << " data/cheb" << endl;
		return -1;
	}

	vector<string> jointNames = {
		"Root (Translation)",
		"Root",
		"Chest",
		"Waist",
		"Neck",
		"Right hip",
		"Right leg",
		"Right knee",
		"Right foot",
		"Left hip",
		"Left leg",
		"Left knee",
		"Left foot",
		"Right collarbone",
		"Right shoulder",
		"Right elbow",
		"Left collarbone",
		"Left shoulder",
		"Left elbow"
	};

	ModelerApplication::Instance()->Init(argc, argv, jointNames);

	// Run the modeler application.
	int ret = ModelerApplication::Instance()->Run();

	// This line is reached when you close the program.
	delete ModelerApplication::Instance();

	return ret;
}
