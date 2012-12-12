/*
 * command.c
 *
 *  Created on: Nov 23, 2012
 *      Author: vjd215
 */

#include <stdio.h>
#include <string.h>
#include "generate.h"




int main(int argc, char *argv[]) {

	if (argc < 2 && argc > 3) {
		// remodel [remodelfile] and optional [target] should be arguments
		printf("Please type correct syntax\n");
	} else {

		int pathSize = strlen(argv[0]);
		char *relStart = argv[0];
		char *fName = strrchr(argv[0], '/');
		fName++;
		int fNameSize = strlen(fName);
		char *fPath = (char*) malloc(pathSize + 1);
		memset(fPath, 0, pathSize);
		memcpy(fPath, relStart, pathSize - fNameSize);
		*(fPath + pathSize - fNameSize) = 0;

		if (strcmp(fName, "remodel") != 0) {
			//Looking for command remodel
			printf("Looking for command remodel space remodelfile\n");

		}

		//Create graph and hash data. Optional to pass new target
		if (readFile(argv[1], argv[2], fPath) ==0)
			printf("Failed");

	}
			printf("Process complete");
			return 0;
}

