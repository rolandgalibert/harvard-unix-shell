#
# File:        	Makefile for smsh
# Author:      	Roland L. Galibert
#               Problem Set 5
#              	For Harvard Extension course CSCI E-28 Unix Systems Programming
# Date:        	April 18, 2015
# 
# This make file contains instructions for creating the smsh executable
#
# Cleanup rules # are also provided for removing executables and object files
# in order to faciliate debugging and recompilation.

# Compilation options
CC = gcc
CFLAGS += -Wall

all:  smsh

# smsh
smsh: builtin.o controlflow.o execute2.o flexstr.o process2.o smsh.o splitline.o varlib.o
	$(CC) $(CFLAGS) builtin.o controlflow.o execute2.o flexstr.o process2.o smsh.o splitline.o varlib.o -o smsh
	
# builtin
builtin.o: builtin.c
	$(CC) $(CFLAGS) -c builtin.c
	
# controlflow
controlflow.o: controlflow.c
	$(CC) $(CFLAGS) -c controlflow.c
	
# execute2
execute2.o: execute2.c
	$(CC) $(CFLAGS) -c execute2.c
	
# flexstr
flexstr.o: flexstr.c
	$(CC) $(CFLAGS) -c flexstr.c
	
# process2
process2.o: process2.c
	$(CC) $(CFLAGS) -c process2.c
	
# smsh
smsh.o: smsh.c
	$(CC) $(CFLAGS) -c smsh.c
	
# splitline
splitline.o: splitline.c
	$(CC) $(CFLAGS) -c splitline.c
	
# varlib
varlib: varlib.c
	$(CC) $(CFLAGS) -c varlib.c
	
# Cleanup
clean:
	rm -f smsh *.o