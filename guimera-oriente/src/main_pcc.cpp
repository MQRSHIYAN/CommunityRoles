#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>

#include "../include/graph_binary.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* pcc = NULL;
	cout << "lecture..." << argv[1] << endl;
	pcc = new Graph(argv[1], NULL, UNWEIGHTED, true);
  	(*pcc).p_scc_all(atoi(argv[2]), atoi(argv[3]), argv[4]);
	
	return 0;

}
