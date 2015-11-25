#!/bin/bash

CC=g++
CFLAGS= -ansi -O5 -Wall -fopenmp
LDFLAGS= -ansi -lm -Wall -fopenmp
EXEC=community convert hierarchy guimera
OBJ1= graph_binary.o community.o
OBJ2= graph.o
OBJ3 = graph_binary.o graph.o
all: $(EXEC)

community : $(OBJ1) main_community.o
	$(CC) -o $@ $^ $(LDFLAGS)

convert : $(OBJ2) main_convert.o
	$(CC) -o $@ $^ $(LDFLAGS)

guimera : $(OBJ3) main_guimera.o
	$(CC) -o $@ $^ $(LDFLAGS)	

hierarchy : main_hierarchy.o
	$(CC) -o $@ $^ $(LDFLAGS)

##########################################
# Generic rules
##########################################

%.o: %.cpp %.h
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o *~ $(EXEC)
