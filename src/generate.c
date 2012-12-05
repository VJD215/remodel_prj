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

int readLine(FILE* file, char* buf, int bufSize)
{
	char* ptr = buf;
	while (bufSize > 1) {
		if (fread(buf, 1, 1, file) == 0) {
			*buf = 0;

			if (buf == ptr)
				return 1;

			return 0;
		}

		if (*buf == '\n')
			break;

		buf++;
	}
	*buf = 0;
	return 0;
	//printf("Line Count= %d\n", linesRead);

	//	char* retVal = NULL;
	//	int i;
		//	while((retVal = fgets(buf, bufSize, file)) != NULL && *retVal != '\n') //for all lines in file
	//	for (i = 0; i < linesRead; i++) {
	//		retVal = fgets(buf, bufSize, file);
	//		printf("Buffer= %s\n", buf);
	//	}
	//	if (retVal == NULL) {
	//		*buf = 0;
	//		return 1;
	//	}
	//	printf("Buffer= %s\n", buf);
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

	*target = (char*) malloc(size + 1);
	memcpy(*target, start, size);
	*(*target + size) = 0;

	while (*ptr != 0 && *ptr != '-') {
		ptr++;
	}
	ptr++;

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
	//setting the array to zero before reading in the children
	*dependency = (char**) malloc(sizeof(char **) * depCnt);
	memset(*dependency, 0, sizeof(char **) * depCnt);

	ptr = start;
	depCnt = 0;
	while (*ptr != 0 && *ptr != ':') {
		if (*ptr == ',') {
			//trim leading spaces
			char* p1 = start;
			while (p1 != ptr && (*p1 == ' ' || *p1 == '\t'))
				p1++;
			//trim post spaces
			char* p2 = ptr;
			//trim before comma
			if (p2 > p1) {
				p2--;
				while (p2 != p1 && (*p2 == ' ' || *p2 == '\t'))
					p2--;

				if (p2 == ptr - 1)
					p2++;
			}

			size = p2 - p1;
			*(*dependency + depCnt) = (char*) malloc(size + 1);
			memcpy(*(*dependency + depCnt), p1, size);
			*(*(*dependency + depCnt) + size) = 0;
			depCnt++;
			start = ptr + 1;
		}

		ptr++;
	}

	if (*ptr != ':')
		return 1;

	//trim leading spaces
	char* p1 = start;
	while (p1 != ptr && (*p1 == ' ' || *p1 == '\t'))
		p1++;

	//trim post spaces
	char* p2 = ptr;
	//trim before comma
	if (p2 > p1) {
		p2--;
		while (p2 != p1 && (*p2 == ' ' || *p2 == '\t'))
			p2--;

		if (p2 == ptr - 1)
			p2++;
	}

	size = p2 - p1;
	*(*dependency + depCnt) = (char*) malloc(size + 1);
	memcpy(*(*dependency + depCnt), p1, size);
	*(*(*dependency + depCnt) + size) = 0;
	ptr++;

	int cmdSize = strlen(ptr);
	if (cmdSize == 0)
		return 1;

	*command = (char*) malloc(cmdSize + 1);
	memcpy(*command, ptr, cmdSize);
	*(*command + cmdSize) = 0;
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

	struct Node *subRoot = (struct Node *) malloc(sizeof(struct Node));
	subRoot->nodeName = target;
	subRoot->next = NULL;
	subRoot->child = NULL;
	subRoot->parentNode = NULL;
	subRoot->command = command;
	//Create the children below and siblings
	int i;
	struct Node *last = NULL;
	for (i = 0; i < numChild; i++) {
		struct Node *childNode = (struct Node *) malloc(sizeof(struct Node));
		childNode->nodeName = dependency[i];
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

int addSubTreeToParent(struct Node *subTree, struct Node *parent)
{
	//First find parent in tree
	//Take subTree and reference children to found parent

	if (strcmp(parent->nodeName, subTree->nodeName) == 0)
	{
		if (parent->child != NULL)
			return 1;

		parent->child = subTree->child;
		parent->command = subTree->command;

		struct Node *ptr = subTree->child;
		while (ptr != NULL)
		{
			ptr->parentNode = parent;
			ptr = ptr->next;
		}

		free(subTree);
		return 0;
	}
	//Attach siblings
	if (parent->next != NULL)
	{
		if (addSubTreeToParent(subTree, parent->next) == 0)
			return 0;
	}
	//Attach children
	if (parent->child != NULL)
	{
		if (addSubTreeToParent(subTree, parent->child) == 0)
			return 0;
	}

	return 1;
}

struct Node* readFile(const char *filename, const char *target) {
	//Read in remodelfile and check for new target
	char lineBuf[2048];
	struct Node* root = NULL;
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Could not open file\n");
		abort();
	} else {
		//Read first line
		if (readLine(file, lineBuf, sizeof(lineBuf)) != 0)
			return NULL;

		while (1) {
			struct Node* ptr = createSubTree(file);
			if (ptr == NULL)
				break;

			if (root == NULL)
				root = ptr;
			else
				addSubTreeToParent(ptr, root);
		}
	}
	fclose(file);
	return root;
}

