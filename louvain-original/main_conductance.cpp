#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>

#include "../graph_binary.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* g = NULL;
	g = new Graph(argv[1], NULL, UNWEIGHTED, false);
	cout << "Loaded" << endl;
	//cout << "Computing conductance..." << endl;
	(*g).readCommunity(argv[2]);
	float conductance = (*g).conductance();
	//cout << "Conductance sortante " << out_conductance << endl;
	//cout << "Conductance sortante " << in_conductance << endl;
	//cout << "Conductance " << conductance_totale << endl;
	float int_d= (*g).internal_density();
  	float cro= (*g).cut_ratio();
	ofstream foutput;
	foutput.open(argv[3], fstream::out | std::fstream::app);
	foutput << argv[4] << ";" << argv[5] << ";" << argv[6] << ";" << argv[7] << ";" << conductance << ";" << int_d  << ";" << cro << endl;
	foutput.close();
	cout << "done" << endl;

//	delete guimera;

	return 0;

}
