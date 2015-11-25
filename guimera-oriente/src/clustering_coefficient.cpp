#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "../include/graph_binary.h"

/* Convertit un fichier a l'envers, puis compare au fichier existant */

int main(int argc, char** argv) {

	/* Extraction du fichier normal depuis le fichier binaire */
	Graph* pcc = NULL;
	cout << "lecture..." << argv[1] << endl;
	pcc = new Graph(argv[1], NULL, UNWEIGHTED, true);
  	//Lire liste de sommets
	//Calculer leur coefficient et l'écrire
	std::ifstream fichier( argv[2] );

	if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
	{
		std::string ligne; // variable contenant chaque ligne lue

		// cette boucle s'arrête dès qu'une erreur de lecture survient
		while ( std::getline( fichier, ligne ) )
		{
		    // afficher la ligne à l'écran
		    std::cout << ligne << std::endl;
		    std::cout << (*pcc).c_coefficient(atoi(ligne.c_str())) << std::endl;
		}
	}
	fichier.close();
	
	return 0;

}
