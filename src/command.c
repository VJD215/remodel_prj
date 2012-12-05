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
		//char *fname = NULL;
		char *fname = strrchr(argv[0], '/');
		*fname++;

		if (strcmp(fname, "remodel") != 0) {
			//Looking for command remodel
			printf("Wrong command\n");

		}

		if (strcmp(argv[1], "-g") == 0) {

			//Create graph and hash data. Optional to pass new target
			readFile(argv[2], argv[3]);

		} else if (strcmp(argv[1], "-c") == 0) {

			//Compile code and check value
			readFile(argv[2], argv[3]);

		} else {
			printf("Wrong flag entered\n");
		}

	}

}

