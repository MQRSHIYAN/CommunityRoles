#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "../include/graph_binary.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

//Les voisins des capitalistes sociaux sont ils des capitalistes sociaux ?
int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* pcc = NULL;
	cout << "lecture..." << argv[1] << endl;
	pcc = new Graph(argv[1], NULL, UNWEIGHTED, false);
  	(*pcc).cap_sociaux_in(argv[2]);
	return 0;

}
