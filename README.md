remodel_prj
===========

Command line structure remodel [remodelfile] optional [new target value]
example: remodel remodelfile 
 or	 remodel remodelfile target

This program is a make like utility that provides the capabilities to read
in a remodelfile (makefile structure) parse it and create a dependency graph
from it.  It will run the compiler commands in parallel for each child of 
the tree starting at the leaves and working towards the root. A md5 hash is
created for each node of the tree and stored in md5.txt, which is automatically
created if one isn't present. The md5 function is run in parallel also using
pthreads under a Linux operating system.

On the first execution of the program a directory will be create called .remodel
and each target will be compiled along with a md5 hash. A md5.txt file will be
generated and put in the /.remodel/md5.txt. Upon the second execution of the 
program the md5 program will verify each of the file to determine if any have 
changed. If any of the files has changed a status flag will be updated for, each
file that has changed a parent will get changed as well and each one walking the 
tree to root. Only those files that receive a status change will be executed. A 
complete run of the md5 function will be run to determine if any other file were 
effected.

In order to run the utility in new state, delete all files created from current 
directory including the md5.txt, directory .remodel e.g. foo.o, baz

A jpeg image is provided to show the program flow logic called remodel.jpg
