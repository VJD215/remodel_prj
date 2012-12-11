/*
 * generate.c
 *
 *  Created on: Nov 23, 2012
 *      Author: vjd215
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "generate.h"

static int counter= 0;

int readLine(FILE* file, char* buf, int bufSize) {
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
		if(p2 != ptr)
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

void resetTarget(const char* targetName, struct Node* root)
{
	//Change target name
	int size = strlen(targetName);
	char* oldName = root->nodeName;
	root->nodeName = (char*)malloc(size + 1);
	memcpy(root->nodeName, targetName, size);
	*(root->nodeName + size) = 0;
	//Change target command
	if (root->command != NULL)
	{
		//Need to find starting point and replace name with new one
		char* p1 = strstr(root->command, oldName);
		if (p1 != NULL)
		{
			int ln1 = strlen(root->command);
			int newSize = ln1 + size - strlen(oldName);
			char* ptr = (char*)malloc(newSize + 1);
			memcpy(ptr, root->command, p1 - root->command);
			memcpy(ptr + (p1 - root->command), targetName, size);
			if (newSize - size - (p1 - root->command) > 1)
				memcpy(ptr + (p1 - root->command) + size, p1 + strlen(oldName), newSize - size - (p1 - root->command));

			*(ptr + newSize) = 0;
			free(root->command);
			root->command = ptr;
		}
	}

	free(oldName);
}

void resetCommand(struct Node* node, const char *fPath) {
	//Add path to file
	//Need to find starting point in g++ -c fileName.cpp
	int pl = strlen(fPath);

	if (node->command != NULL) {
		char *ptr = node->nodeName;
		char *getName = strrchr(node->nodeName, '.');
		int nameSize = getName - ptr;
		char *searchName = (char*) malloc(nameSize);
		memset(searchName, 0, sizeof(nameSize));
		memcpy(searchName, ptr, nameSize);
		//Find starting point in command
		int cl = strlen(node->command);
		char *ptr1 = node->command;
		char *start = ptr1;

		while (*ptr1 != '.') {
			ptr1++;

			if (*ptr1 == 'c')
				ptr1++;
		}
		if (ptr1 != NULL) {

			int newSize = pl + cl;
			char* ptr2 = (char*) malloc(newSize + 1);
			memset(ptr2, 0, sizeof(node->command + 1));
			int cPt = ptr1 - start - nameSize;
			memcpy(ptr2, node->command, cPt);

			char *getName2 = strrchr(ptr2, '"');
			getName2++;

			char *getName1 = strrchr(ptr1, '"');
			getName1--;

			int endl = getName1 - ptr1;
			char* ptr4 = (char*) malloc(endl + 1);
			memset(ptr4, 0, (endl + 1));
			memcpy(ptr4, ptr1, endl + 1);

			int cmdl = strlen(getName) + pl + strlen(searchName) + strlen(ptr4);

			char* ptr3 = (char*) malloc(cmdl + 1);
			memset(ptr3, 0, sizeof(cmdl + 1));

			sprintf(ptr3, "%s%s%s%s", getName2, fPath, searchName, ptr4);

			free(node->command);
			node->command = ptr3;

		}
	}
}


//void* workThread(void* cmdPath) {
void* workThread(void* cmdPath) {
	//Run the command that is given
	/*

	if (system(cmdPath) != 0)
		return 1;
	return 0;
	*/
	FILE *cmd = popen(cmdPath, "r");
    if ( cmd == 0 ) {
        printf( "Could not execute\n" );
        return NULL;
    }
	char result[2048];
	while (fgets(result, sizeof(result), cmd) != NULL)
		printf("%s\n", result);
	pclose(cmd);
}

bool createThread(char* cmdPath, pthread_t *pt) {
	//Create a thread for each call

	if (pthread_create(&pt, NULL, &workThread, (void*) cmdPath) != 0)
		return 1;
	if (pthread_join(pt, NULL) != 0)
		return 1;

	return 0;
	pthread_exit(NULL);
}

int runMD5(struct Node* node, const char *fPath) {
	pthread_t pt;
	if (node->child != NULL)
		runMD5(node->child, fPath);
	//sprintf("/usr/bin/md5sum %s > .remodel/md5.txt", node->nodeName);

	char bufMd5[2048];
	memset(bufMd5, 0, sizeof(bufMd5));
	// build this md5sum filename > .remodel/md5.txt
	if (counter == 0) {
		sprintf(bufMd5,"md5sum %s%s > .remodel/md5.txt", fPath, node->nodeName);
		counter++;
	} else {
		sprintf(bufMd5,"md5sum %s%s >> .remodel/md5.txt", fPath, node->nodeName);
	}

	if (createThread(bufMd5, &pt))
		return 1;

	if (node->next != NULL)
		runMD5(node->next, fPath);
}

void runCmd(struct Node* node, const char *fPath) {
	//Run the commands for each node in parallel
	pthread_t pt;

	if (node->child != NULL)
		runCmd(node->child, fPath);

	if (node->command != NULL) {
		char buf[2048];
		memset(buf, 0, sizeof(buf));

		char *sourceExt = strchr(node->nodeName, '.');
		if (sourceExt != NULL) {

			if ((strcmp(sourceExt, ".cpp") == 0) || (strcmp(sourceExt, ".o")
					== 0))
				resetCommand(node, fPath);
		} else {

			//Need to fix other string and remove ""
			const char *src = strrchr(node->command, 'g');
			if (src != NULL) {
				//src--;
				const char *dsc = strrchr(src, '"');
				int endl = dsc - src;
				char* iptr = (char*) malloc(endl + 1);
				memset(iptr, 0, endl);
				memcpy(iptr, src, endl);
				iptr[endl] = 0;
				node->command = iptr;
			}
		}

		sprintf(buf,"%s", node->command);

		if (createThread(buf, &pt))
			return 1;

		if (node->next != NULL)
			runCmd(node->next, fPath);
	}

}

struct Node* readFile(const char *filename, const char *target,	const char *fPath) {
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
			else {
				addSubTreeToParent(ptr, root);
			}
		}
		//Change target to one given in command argument
		if (target != NULL)
			resetTarget(target, root);


		char *dirName = ".remodel";
		char *fileName = ".remodel/md5.txt";
		struct stat *st;

		st = malloc(sizeof(struct stat));

		if (stat(dirName, &st) == 0) {
			printf(".remodel is present\n");
		} else {
			system("mkdir .remodel");
		}

		if (stat(fileName, &st) == 0) {
			printf(".remodel/md5.txt is present\n");
			FILE *cmd = popen("md5sum -c .remodel/md5.txt", "r");
			char result[2048];
			while (fgets(result, sizeof(result), cmd) != NULL)
				printf("%s\n", result);
			pclose(cmd);

			// run a parser on the array and only look for FAIL results
			// If failed result is found get filename
			// set the statusFlag for each nodeName found

			runCmd(root, fPath);
			runMD5(root, fPath);

		} else {

			runCmd(root, fPath);
			runMD5(root, fPath);

		}
		//free(st);
	}
	fclose(file);
	return root;
}





