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

	if (argc < 3 && argc > 4) {
		// remodel [-g] [-c] [remodelfile] and optional [target] should be arguments
		printf("Please type correct syntax\n");
	} else {

		int pathSize = strlen(argv[0]);
		char *relStart = argv[0];
		char *fName = strrchr(argv[0], '/');
		*fName++;
		int fNameSize = strlen(fName);
		char *fPath = (char*)malloc(pathSize + 1);
		memcpy(fPath, relStart, pathSize -fNameSize);
		*(fPath + pathSize -fNameSize) = 0;


		if (strcmp(fName, "remodel") != 0) {
			//Looking for command remodel
			printf("Wrong command\n");

		}

		if (strcmp(argv[1], "-g") == 0) {

			//Create graph and hash data. Optional to pass new target
			readFile(argv[2], argv[3], fPath);

		} else if (strcmp(argv[1], "-c") == 0) {

			//Compile code and check value
			//readFile(argv[2], argv[3]);

		} else {
			printf("Wrong flag entered\n");
		}

	}

}

