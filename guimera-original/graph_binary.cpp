// File: graph_binary.cpp
// -- graph handling source
//-----------------------------------------------------------------------------
// Community detection 
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// This file is part of Louvain algorithm.
// 
// Louvain algorithm is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Louvain algorithm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Louvain algorithm.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------
// Author   : E. Lefebvre, adapted by J.-L. Guillaume
// Email    : jean-loup.guillaume@lip6.fr
// Location : Paris, France
// Time	    : February 2008
//-----------------------------------------------------------------------------
// see readme.txt for more details

#include <sys/mman.h>
#include <fstream>
#include "graph_binary.h"
#include "math.h"
#include <limits>
#include<omp.h>

Graph::Graph() {
  nb_nodes     = 0;
  nb_links     = 0;
  total_weight = 0;
}

Graph::Graph(char *filename, char *filename_w, int type) {
  ifstream finput;
  finput.open(filename,fstream::in | fstream::binary);

  // Read number of nodes on 4 bytes
  finput.read((char *)&nb_nodes, 4);
  assert(finput.rdstate() == ios::goodbit);

  // Read cumulative degree sequence: 8 bytes for each node
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.
  degrees.resize(nb_nodes);
  finput.read((char *)&degrees[0], nb_nodes*8);

  // Read links: 4 bytes for each link (each link is counted twice)
  nb_links=degrees[nb_nodes-1];
  links.resize(nb_links);
  finput.read((char *)(&links[0]), (long)nb_links*8);  

  // IF WEIGHTED : read weights: 4 bytes for each link (each link is counted twice)
  weights.resize(0);
  total_weight=0;
  if (type==WEIGHTED) {
    ifstream finput_w;
    finput_w.open(filename_w,fstream::in | fstream::binary);
    weights.resize(nb_links);
    finput_w.read((char *)&weights[0], (long)nb_links*4);  
}
  // Compute total weight
  for (unsigned int i=0 ; i<nb_nodes ; i++) {
    total_weight += (double)weighted_degree(i);
  }
}

Graph::Graph(int n, int m, double t, int *d, int *l, float *w) {
/*  nb_nodes     = n;
  nb_links     = m;
  total_weight = t;
  degrees      = d;
  links        = l;
  weights      = w;*/
}


void
Graph::display() {
/*  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<double>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors(node) ; i++) {
      if (node<=*(p.first+i)) {
	if (weights.size()!=0)
	  cout << node << " " << *(p.first+i) << " " << *(p.second+i) << endl;
	else
	  cout << node << " " << *(p.first+i) << endl;
      }
    }   
  }*/
  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<double>::iterator > p = neighbors(node);
    cout << node << ":" ;
    for (unsigned int i=0 ; i<nb_neighbors(node) ; i++) {
      if (true) {
	if (weights.size()!=0)
	  cout << " (" << *(p.first+i) << " " << *(p.second+i) << ")";
	else
	  cout << " " << *(p.first+i);
      }
    }
    cout << endl;
  }
}

void
Graph::display_reverse() {
  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<double>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors(node) ; i++) {
      if (node>*(p.first+i)) {
	if (weights.size()!=0)
	  cout << *(p.first+i) << " " << node << " " << *(p.second+i) << endl;
	else
	  cout << *(p.first+i) << " " << node << endl;
      }
    }   
  }
}


bool
Graph::check_symmetry() {
  int error=0;
  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<double>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors(node) ; i++) {
      unsigned int neigh = *(p.first+i);
      double weight = *(p.second+i);
      
      pair<vector<unsigned int>::iterator, vector<double>::iterator > p_neigh = neighbors(neigh);
      for (unsigned int j=0 ; j<nb_neighbors(neigh) ; j++) {
	unsigned int neigh_neigh = *(p_neigh.first+j);
	double neigh_weight = *(p_neigh.second+j);

	if (node==neigh_neigh && weight!=neigh_weight) {
	  cout << node << " " << neigh << " " << weight << " " << neigh_weight << endl;
	  if (error++==10)
	    exit(0);
	}
      }
    }
  }
  return (error==0);
}


void
Graph::display_binary(char *outfile) {
  ofstream foutput;
  foutput.open(outfile ,fstream::out | fstream::binary);

  foutput.write((char *)(&nb_nodes),4);
  foutput.write((char *)(&degrees[0]),4*nb_nodes);
  foutput.write((char *)(&links[0]),8*nb_links);
}

/************************************************************************

 ************************************************************************

 *

 *

 *

 *

 *GUIIIIIIIIIIIIMERA STYYYYYYLE

 * 

 *

 *

 *

 ***********************************************************************

 ***********************************************************************

*/





	void

	Graph::readCommunity(string filename) {



		ifstream finput;

		unsigned int node, node_corr, com, corr;



		finput.open(filename.c_str(), fstream::in);

		node = 0; com = 0; corr = 0; 



		map_communities.resize(nb_nodes);

		int cpt = 0;





		cpt = 0;



		while (!finput.eof() && cpt < nb_nodes) {



			/* Strange: reads one line more than expected... */		

			cpt++; 

			if(finput.eof()) 



				break; 



			/* Getting a node and its community */

		        finput >> node_corr >> com;

			//node_corr--;

			/* Is it really needed? Depends on the community algorithm? */		

			//com = com - 1;



			/* We associate the community to the node */

		        map_communities[node_corr] = com; 

	

			if(communities.size() <= com) {



				communities.resize(com+1);

			}

	

			communities[com].push_back(node_corr); 		



		}



		finput.close();

	}





	float

	Graph::degree_comm(int node, int comm) {



		unsigned int pos, pos_end;
		float cpt = 0.;

		 

		if(node == 0) pos = 0; else pos = degrees[node - 1];

		pos_end=degrees[node];



		for (unsigned int i=pos; i < pos_end; i++) {

			if (map_communities[links[i]] == comm)

				cpt++;

		}    

		return cpt;



	}



	float

	Graph::avg_degree_comm(int comm) {



		long degree_comm_global = 0;



		for(unsigned int i = 0; i < communities[comm].size(); i++) {



			degree_comm_global += degree_com[communities[comm][i]];



		}



		return  degree_comm_global / communities[comm].size();

	}



	double

	Graph::variane(int comm) {



		double mean = avg_com[comm];

		/* TODO: do we calculate it here? Or before? */



		double tmp = 0;

		double degree_node = 0;



		for(unsigned int i = 0; i < communities[comm].size(); i++) {

	

			degree_node = degree_com[communities[comm][i]];

			tmp += (degree_node - mean) * (degree_node - mean);



		}



		return tmp / (double)communities[comm].size();



	}



	double 

	Graph::standard_deviation(int comm) {



		return sqrt(variane(comm));



	}



	double

	Graph::z_score(int node, int comm) {



		if(std_com[comm] == 0)



			return std::numeric_limits<double>::max();



		else



			return ((degree_com[node] - avg_com[comm]) / std_com[comm]);



	}

	double
        Graph::participation(int node) {

                double sum = 0;
                double tmp = 0;

                for (unsigned int i = 0; i < communities.size(); i++) {

                        if(nb_neighbors(node) > 0) {

                                tmp = ((degree_comm(node, i)) / nb_neighbors(node));
                                sum += tmp * tmp;

                        }

                }

                return (1 - sum);

        }


	/*double
	Graph::participation(int node) {

		//Un vecteur de taille égale au nombre de communautés, rempli de 0
		vector<unsigned int> communities_count(communities.size(),0);

		unsigned int value, communaute;

		// Parcours du voisinage des noeuds
		unsigned int pos, pos_end;
		if(node == 0) pos = 0; else pos = degrees[node - 1];
		pos_end=degrees[node];

		// A la fin, le vecteur contient le degre du noeud vers chaque communaute
		for (unsigned int i = pos; i < pos_end; i++) {
			// Communaute du noeud
		        communaute = map_communities[links[i]];
		        communities_count[communaute]++;
		}

		double sum = 0.;

		// Parcours du vecteur de communautes pour calculer la participation
		for(unsigned int i = 0; i < communities.size(); i++) {

			// Si le noeud a des voisins dans la communaute
			if(communities_count[i] > 0) {

				sum += ((double)(communities_count[i]) / (double)nb_neighbors(node)) * ((double)(communities_count[i]) / (double)nb_neighbors(node));

			}
		}

		return (1 - sum);

	}*/

	void
        Graph::writeGuimeraClassic(string filename) {

                ofstream foutput;      
                
                avg_com.resize(communities.size());
                std_com.resize(communities.size());
                degree_com.resize(nb_nodes);

		unsigned int i, thread_id;

                //Z_score
                cout << "Calcul des degrés internes au communauté " <<endl;
                #pragma omp parallel private(thread_id)
                { 
                        thread_id = omp_get_thread_num();
                        int cpt = 1;
                        /*stringstream ss;
                        ss << thread_id;
                        string filename2 = filename+"_z_score_" + ss.str();*/
                        #pragma omp for
                        for (i=0; i < nb_nodes; i++) {
                                degree_com[i]=degree_comm(i,map_communities[i]);
                                if (cpt % 500 == 0)
                                        cout << thread_id <<" a traité 500 communautés" << endl; 
                        }
                        #pragma omp barrier
                        #pragma omp master
                        {
                                cout << "Calcul des moyennes et écart-type pour le z-score " <<endl;
                        }
                        cpt=1;
			#pragma omp for
                        for(i = 0; i < communities.size(); i++) {
                                //cout << i << " effectué par " << omp_get_thread_num() << endl;
                                avg_com[i]= avg_degree_comm(i);
                                std_com[i]= standard_deviation(i);
                                if (cpt % 500 == 0)
                                        cout << thread_id <<" a traité 500 communautés" << endl; 
                                cpt++;
                        }
                        
                }
                

                foutput.open((filename+"_z_score").c_str(), fstream::out);
                cout << "Calcul du z-score " <<endl;
                for(i = 0; i < nb_nodes; i++) {
                        foutput << z_score(i, map_communities[i]) << "\t" << participation(i) << endl;
                        if (i % 1000000 == 0)
                                cout << "1 million de noeuds en plus pour le z_score : présence !" << endl;

                }
        
                foutput.close();
                
                }
