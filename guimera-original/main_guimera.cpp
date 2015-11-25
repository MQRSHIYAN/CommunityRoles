#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>

#include "graph_binary.h"
#include "graph.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* guimera = NULL;
	guimera = new Graph(argv[1], NULL, UNWEIGHTED);
	cout << "Computing Guimera Classic..." << endl;
	(*guimera).readCommunity(argv[2]);
	(*guimera).writeGuimeraClassic(argv[3]);
	cout << "done" << endl;

	return 0;

}
