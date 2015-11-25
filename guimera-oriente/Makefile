#!/bin/bash

CC=g++

SRCDIR=src
HEADDIR=include
LIBDIR=obj
BINDIR=bin

CFLAGS= -ansi -O5 -Wall  -fopenmp
LDFLAGS= -ansi -lm -Wall  -fopenmp
EXEC=kernel pscc convert guimera clustering_coefficient k_neighbors who_neighbors overlap

SRC= $(wildcard $(SRCDIR)/*.cpp)
OBJ1= $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/graph_binary.o)
OBJ2= $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/graph.o)
OBJ3 = $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/graph.o) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/graph_binary.o)

all: $(EXEC)

kernel : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_kernelization.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

pscc : $(OBJ1) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_pcc.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

convert : $(OBJ2) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_convert.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

guimera : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_guimera.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

guimera_classic : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_guimera_classic.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

clustering_coefficient : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/clustering_coefficient.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

k_neighbors : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/k_neighbors.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

who_neighbors : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/who_neighbors.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

overlap : $(OBJ3) $(SRC:$(SRCDIR)/%.cpp=$(LIBDIR)/main_overlap.o)
	$(CC) -g -o $(BINDIR)/$@ $^ $(LDFLAGS)

##########################################
# Generic rules
##########################################

$(LIBDIR)/%.o: $(SRCDIR)/%%.cpp $(HEADDIR)/%.h
	$(CC) -g -o $@ -c $< $(CFLAGS)

$(LIBDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -g -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(LIBDIR)/*.o $(LIBDIR)/*~ $(SRCDIR)/*~ 
