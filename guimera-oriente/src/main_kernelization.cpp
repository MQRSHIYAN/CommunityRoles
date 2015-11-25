// File: main_convert.cpp
// -- conversion of a graph from ascii to binary, sample main file
//-----------------------------------------------------------------------------
// Community detection 
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// This program must not be distributed without agreement of the above mentionned authors.
//-----------------------------------------------------------------------------
// Author   : E. Lefebvre, adapted by J.-L. Guillaume
// Email    : jean-loup.guillaume@lip6.fr
// Location : Paris, France
// Time	    : February 2008
//-----------------------------------------------------------------------------
// see readme.txt for more details

using namespace std;
#include <omp.h>
#include <queue>
#include "../include/graph.h"


char *infile   = NULL;
char *outfile  = NULL;
char *outfile_w  = NULL;
int type       = UNWEIGHTED;
bool do_renumber = false;
bool do_kernel = false;
bool do_bfs = false;
bool do_print = false;
unsigned int sommet;

void
usage(char *prog_name, const char *more) {
  cerr << more;
  cerr << "usage: " << prog_name << " -i input_file -s sommet" << endl << endl;
  cerr << "-r to renumber the graph." << endl;
  cerr << "-k to kernelize the graph." << endl;
  cerr << "-p to print the deleted file." << endl;
  cerr << "-h\tshow this usage message." << endl;
  exit(0);
}

void
parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'i':
	if (i==argc-1)
	  usage(argv[0], "Infile missing\n");
	infile = argv[i+1];
	i++;
	break;
      case 'o':
	if (i==argc-1)
	  usage(argv[0], "Outfile missing\n");
        outfile = argv[i+1];
	i++;
	break;
      case 's':
        if (i==argc-1)
          usage(argv[0], "vertice missing\n");
        sommet = atoi(argv[i+1]);
	do_bfs=true;
        i++;
        break;
      case 'k' :
	do_kernel=true;
	break;
      case 'r' :
	do_renumber=true;
	break;
      case 'p' :
	do_print=true;
	break;
      default:
	usage(argv[0], "Unknown option\n");
      }
    } else {
      usage(argv[0], "More than one filename\n");
    }
  }
  if (infile==NULL || outfile==NULL)
    usage(argv[0], "In or outfile missing\n");
}

void 
delVertices(Graph& g, queue<unsigned int>& q, bool print, bool bfs) {
	ofstream foutput;
	unsigned int i;
	vector<pair<unsigned int,float> >::iterator j;
	//Pour imprimer les sommets supprimés
	if (print) {
		string filename;
		if (bfs)
			filename="not_connected";
		else
			filename="sinks_or_sources";
  		foutput.open(filename.c_str(), fstream::out | fstream::app);
	}
	//La, on va en chier, on supprime ces sommets de la structure du graphe
	while (!q.empty()) {
		if (print) {
			foutput << q.front() << endl;
		}
		//Si le sommet qu'on veut supprimer a des voisins entrants
		//Alors on va supprimer dans la liste d'adjacence sortante de chacun de ses sommets le sommet à supprimer
		if (g.links_in[q.front()].size() != 0) {
			#pragma omp parallel for private(i, j)
			for(i=0;i<g.links_in[q.front()].size();i++) {
				//cout << "g.links_in[q.front()][i].first " << g.links_in[q.front()][i].first << endl;
				for(j = g.links_out[g.links_in[q.front()][i].first].begin(); j != g.links_out[g.links_in[q.front()][i].first].end(); j++) {
					//cout << "j.first " << (*j).first << endl;
					if ((*j).first == q.front()) {
						//cout << "size" <<g.links_out[g.links_in[q.front()][i].first].size() << endl;
						g.links_out[g.links_in[q.front()][i].first].erase(j);
						//cout << "size" <<g.links_out[g.links_in[q.front()][i].first].size() << endl;
						//cout << "delete " << endl;
						break;
					}
				} 
			} 
			g.links_in[q.front()].clear();
		}
		//Si le sommet qu'on veut supprimer a des voisins sortants
		//Alors on va supprimer dans la liste d'adjacence entrante de chacun de ses sommets le sommet à supprimer
		if (g.links_out[q.front()].size() != 0) {
			#pragma omp parallel for private(i, j)
			for(i=0;i<g.links_out[q.front()].size();i++) {
				//cout << "g.links_out[q.front()][i].first " << g.links_out[q.front()][i].first << endl;
				for(j = g.links_in[g.links_out[q.front()][i].first].begin(); j != g.links_in[g.links_out[q.front()][i].first].end(); j++) 					{
					//cout << "j.first " << (*j).first << endl;
					if ((*j).first == q.front()) {
						//cout << "size" <<g.links_in[g.links_out[q.front()][i].first].size() << endl;
						g.links_in[g.links_out[q.front()][i].first].erase(j);
						//cout << "size" <<g.links_in[g.links_out[q.front()][i].first].size() << endl;
						//cout << "delete " << endl;
						break;
					}
				} 
			} 
			g.links_out[q.front()].clear();
		}
		q.pop();
	}
	if (print) {
  		foutput.close();
	}

}

int
main(int argc, char **argv) {
  parse_args(argc, argv);
  Graph* g;

   // renumber(type, (string)infile, (string)infile + "_renum");

    string nameGraph = (string)infile;
    cout << "Loading graph "<< infile << endl;
    g = new Graph((char*)nameGraph.c_str(), type, do_renumber);
    unsigned int i;
    
    
    cout << "End Loading " << endl;
    //(*g).display(type);
	//On effectue un BFS pour conserver uniquement la composante géante

	queue<unsigned int> q;

	if (do_bfs) {
		cout << "-------BFS sommet " << sommet <<"---------- " << endl;
	    	
		unsigned int x,z;
		//Permet de savoir si un noeud i a été vu pendant le BFS
		//La valeur par défaut à la création du vecteur est false dans chaque case
		vector<bool> vu;
		//cout << "blabla " << endl;
		cout << (*g).links_in.size() << endl;
		vu.resize((*g).links_in.size()); 
		//for (i =0; i < (*g).links_in.size(); i++)
		//	vu[i]=false;
		//Le BFS part du sommet 0
		vu[sommet] =true;
		q.push(sommet);
		//int aaa = 0;
		//cout << "Debut push " << endl;
		while (!q.empty()) {
			x=q.front();
			q.pop();
			//cout << (*g).correspondance[x] << endl;
			for (i=0; i < (*g).links_out[x].size(); i++) {
				z= (*g).links_out[x][i].first;
				if (!vu[z])	{
					vu[z] = true;
					q.push(z);
				}
			}
			for (i=0; i < (*g).links_in[x].size(); i++) {
				z= (*g).links_in[x][i].first;
				if (!vu[z])	{
					vu[z] = true;
					q.push(z);
				}
			}
		}
		cout << "Mise en suppression des sommets " << endl;
		//cout << aaa << " sommets vus" << endl;
		//Tous les sommets qui n'ont pas été vus par le BFS sont à supprimer !
		//#pragma omp parallel for private(i) 
		for (i =0; i < (*g).links_in.size(); i++) {
			if (!vu[i]) {
				//cout << i << endl;
				q.push(i);
			}
		}
		vu.clear();
		//On supprime les sommets qui n'ont pas été vus par le BFS
		delVertices((*g), q, do_print, true);
		cout << "Fin du BFS " << endl;
	}
	//(*g).display(type);
	if (do_kernel) {
		cout << "Suppressions des puits et sources de tous niveaux " << endl;
		//On supprime maintenant les puits et els sources
		vector<bool> supprime;
		supprime.resize((*g).links_in.size()); 
		//On supprime à tous les niveaux
		while (true) {
			q.empty();
			//#pragma omp parallel for private(i) 
		    	for (i =0; i < (*g).links_in.size(); i++) {
				if  (((*g).links_in[i].size() == 0 || (*g).links_out[i].size() == 0) && (!supprime[i])) {
					q.push(i);
					supprime[i]= true;
				}
		    	}
			//Si aucun autre puits ou source n'a été détecté, on arrête
			if (q.empty())
				break;
			//Sinon, on supprime ces sommets et on cherchera ensuite les puits et sources de niveau supérieur
			else
		 		delVertices((*g), q, do_print, false);
		}
	    	cout << "Fin de la suppression des puits et sources " << endl;
	}

  
  
  
  //(*g).display(type);
	
  	(*g).displayFile((string)outfile, type, do_renumber);

}
