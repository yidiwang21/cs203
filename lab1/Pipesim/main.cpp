#include <iostream>
#include "unistd.h"
#include "pipeline.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[]) {

	int opt;
	int width = 0;	// forwarding default disabled 
	bool forwarding = false;
	string fileName = "instruction.txt";
	while ((opt = getopt(argc,argv,"fi:")) != EOF)
        switch(opt)
        {
            case 'f': forwarding = true; break;
            case 'i': fileName.assign(optarg); break;
            case '?': fprintf(stderr, "usuage is \n -i fileName : to run input file fileName \n -f [forwarding window width (0 - 2)]: to enable forwarding (disabled by dfl)");
            default: cout<<endl; abort();
        }

	if (forwarding == true) {
		width = atoi(argv[4]);
		switch (width)
		{
			case 0: cout<<"# Forwarding disabled..."<<endl; break;
			case 1: cout<<"# EXEC/MEM -> EXEC forwarding enabled..."<<endl; break; 
			case 2: cout<<"# EXEC/MEM -> EXEC & MEM/WB -> EXEC forwarding enabled..."<<endl; break;
			default: fprintf(stderr, "ERROR: Invalid width value!\n"); abort();
		}
	}
	
	cout << "Loading application..." << fileName << endl;
	Application application;
	application.loadApplication(fileName);
	cout << "Initializing pipeline..." << endl;
	Pipeline pipeline(&application);
	pipeline.forwarding = forwarding;
	/* FORWARDING WINDOW WIDTH */
	pipeline.width = width;

	do {
		pipeline.cycle();
		pipeline.printPipeline();

	} while(!pipeline.done());

	cout << "Completed in " << pipeline.cycleTime - 1 << " cycles" << endl;
	return 0;
}
