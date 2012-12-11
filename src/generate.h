/*
 * generate.h
 *
 *  Created on: Nov 29, 2012
 *      Author: vjd215
 */

#ifndef GENERATE_H_
#define GENERATE_H_



struct Node
{
    char *nodeName;
    char *MD5HashValue;
    char *statusFlag;
    struct Node *parentNode;
    struct Node *child;
    struct Node *next;
    char *command;
};

int system(const char *);

struct Node* readFile(const char *, const char *, const char *);

#endif /* GENERATE_H_ */
