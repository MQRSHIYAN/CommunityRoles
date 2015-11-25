#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "../include/graph_binary.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

//Qui sont les voisins des capitalistes sociaux
int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* pcc = NULL;
	cout << "lecture..." << argv[1] << endl;
	pcc = new Graph(argv[1], NULL, UNWEIGHTED, false);
  	(*pcc).cap_sociaux_voisins_count(argv[2]);
	return 0;

}
