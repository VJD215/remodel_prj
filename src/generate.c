/*
 * generate.c
 *
 *  Created on: Nov 23, 2012
 *      Author: vjd215
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generate.h"

int readLine(FILE *file, char* buf, int bufSize) {
	//read one line at a time into buffer

	char* retVal = NULL;
	//while((retVal = fgets(c,BUF,f)) != NULL && *retVal != '\n') for all lines in file
	retVal = fgets(buf, bufSize, file);
	if (retVal == NULL) {
		*buf = 0;
		return 1;
	}

	return 0;
}

int parse(char *buffer, char **target, char ***dependency, int *numChild, char **command) {
	/* parse file to get this structure
	 * target '<-' dependency (':' '"' command '"")
	 * Here's an example that builds the program "baz" from two source files,
	 * "foo.cpp" and "bar.cpp".
	 *
	 * DEFAULT <- baz
	 * baz <- foo.o, bar.o: "g++ foo.o bar.o -o baz"
	 * foo.o <- foo.cpp : "g++ -c foo.cpp -o foo.o"
	 * bar.o <- bar.cpp: "g++ -c bar.cpp -o bar.o"
	 */
	*numChild = 0;
	char *ptr;
	ptr = buffer;

	while (*ptr != 0 && (*ptr == ' ' || *ptr == '\t')) {
		ptr++;
	}
	char* start = ptr;
	while (*ptr != 0 && *ptr != ' ') {
		ptr++;
	}

	int size = ptr - start;
	if (size == 0)
		return 1;

	*target = malloc(size + 1);
	memcpy(*target, start, size);
	*(*target + size) = 0;

	while (*ptr != 0 && *ptr != '-') {
		ptr++;
	}

	while (*ptr != 0 && (*ptr == ' ' || *ptr == '\t')) {
		ptr++;
	}
	start = ptr;

	int depCnt = 0;
	while (*ptr != 0 && *ptr != ':') {
		if (*ptr == ',')
			depCnt++;
		ptr++;
	}
	depCnt++;
	if (depCnt == 0)
		return 1;

	*numChild = depCnt;
	*dependency = malloc(sizeof(char **) * depCnt);
	memset(*dependency, 0, sizeof(char **) * depCnt);
	// get the individual children from dependency field
	ptr = start;
	depCnt = 0;
	while (*ptr != 0 && *ptr != ':') {
		if (*ptr == ',') {
			size = ptr - start;
			*(*dependency + depCnt) = malloc(size + 1);
			memcpy(*(*dependency + depCnt), start, size);
			*(*(*dependency + depCnt) + size) = 0;
			depCnt++;
			start = ptr + 1;
		}

		ptr++;
	}

	if (*ptr != ':')
		return 1;

	size = ptr - start;
	*(*dependency + depCnt) = malloc(size + 1);
	memcpy(*(*dependency + depCnt), start, size);
	*(*(*dependency + depCnt) + size) = 0;
	ptr++;

	int cmdSize = strlen(ptr);
	if (cmdSize == 0)
		return 1;

	*command = malloc(cmdSize + 1);
	memcpy(*command, ptr, cmdSize);
	*(*command + cmdSize) = 0;
	//printf("target= %c, dependency = %c, command= %c\n", *target, *dependency, *command);
	return 0;
}
struct Node* createSubTree(FILE* file) {
	//From the dependencies or children create a tree from parent to child or children
	char lineBuf[2048];
	if (readLine(file, lineBuf, sizeof(lineBuf)) != 0)
		return NULL;

	char *target;
	char **dependency;
	int numChild;
	char *command;
	if (parse(lineBuf, &target, &dependency, &numChild, &command) != 0)
		return NULL;

	struct Node *subRoot = malloc(sizeof(struct Node));
	subRoot->nodeName = target;
	subRoot->next = NULL;
	subRoot->child = NULL;
	subRoot->parentNode = NULL;
	subRoot->command = NULL;

	//Create each child
	int i;
	struct Node *last = NULL;
	for (i = 0; i < numChild; i++) {
		struct Node *childNode = malloc(sizeof(struct Node));
		childNode->nodeName = *dependency + i;
		childNode->next = NULL;
		childNode->child = NULL;
		childNode->parentNode = subRoot;
		if (last == NULL)
			subRoot->child = childNode;
		else
			last->next = childNode;

		last = childNode;
	}

	return subRoot;
}

struct Node* readFile(const char *filename, const char *target) {
	//Read in remodelfile and check for new target


	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Could not open file\n");
		abort();
	} else {

		struct Node* subRoot;
		subRoot = NULL;
		while (1) {
			struct Node* ptr = createSubTree(file);
			if (ptr == NULL)
				break;

			if (subRoot == NULL)
				subRoot = ptr;

		}

	}
	fclose(file);

}

