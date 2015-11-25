// File: graph_binary.cpp
// -- graph handling source
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

#include <sys/mman.h>
#include <fstream>
#include<sstream>
#include <string>
#include <stack>
#include <set>
#include<string.h>
#include "../include/graph_binary.h"
#include "math.h"
#include <omp.h>
#include <limits>
#include <algorithm>

using namespace std;

Graph::Graph() {
  nb_nodes     = 0;
  nb_links_out     = 0;
  nb_links_in     = 0;
  total_weight = 0;
  total_weight_in = 0;
  total_weight_out = 0;
}

Graph::Graph(char *filename, char *filename_w, int type, bool renumbered) {
  ifstream finput;
  finput.open(filename,fstream::in | fstream::binary);
  
  cout << "number of nodes" << endl;
  // Read number of nodes on 4 bytes
  finput.read((char *)&nb_nodes, sizeof(int));
  assert(finput.rdstate() == ios::goodbit);
  cout << "done: " << nb_nodes << endl;
  
  // Read cumulative out degree sequence: 8 bytes for each node
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.
	cout << "degrees out" << endl;
  degrees_out.resize(nb_nodes);
  finput.read((char *)&degrees_out[0], nb_nodes*sizeof(long));
	cout << "done : " << degrees_out[nb_nodes - 1] << endl;

// Read links_out: 4 bytes for each link 
	cout << "links_out" << endl;
  nb_links_out=degrees_out[nb_nodes-1];
  links.resize(nb_links_out);
  finput.read((char *)(&links[0]), (long)nb_links_out*sizeof(unsigned int)); 
	cout << "done" << endl;
  
  // Read cumulative in degree sequence: 8 bytes for each node
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.
	cout << "degrees in" << endl;
  degrees_in.resize(nb_nodes);
  finput.read((char *)&degrees_in[0], nb_nodes*sizeof(long));
	cout << "done : " << degrees_in[nb_nodes - 1] << endl;

 // Read links_in: 4 bytes for each link 
	cout << "links in" << endl;
  nb_links_in=degrees_in[nb_nodes-1];
  links_in.resize(nb_links_in);
  finput.read((char *)(&links_in[0]), (long)nb_links_in*sizeof(unsigned int)); 
	cout << "done" << endl; 
 
  // Read correspondance of labels
  if(renumbered) {
	cout << "correspondance" << endl;
  correspondance.resize(nb_nodes);
  finput.read((char *)(&correspondance[0]), nb_nodes*sizeof(unsigned long long int)); 
  }
  cout << "done" << endl;
 
  // IF WEIGHTED : read weights: 4 bytes for each link (each link is counted twice)
  weights.resize(0);
  weights_in.resize(0);
	
  total_weight=0;
  if (type==WEIGHTED) {
    ifstream finput_w;
    finput_w.open(filename_w,fstream::in | fstream::binary);
    weights.resize(nb_links_out);
    finput_w.read((char *)&weights[0], (long)nb_links_out*sizeof(int));  
  }    

  // Compute total weight
  for (unsigned int i=0 ; i<nb_nodes ; i++) {
    total_weight += out_weighted_degree(i);
  }
}

Graph::Graph(int n, int m, double t, int *d, int *l, float *w) {
/*  nb_nodes     = n;
  nb_links     = m;
  total_weight = t;
  degrees_out      = d;
  links        = l;
  weights      = w;*/
}

void
Graph::display() {
/*  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<float>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors_out(node) ; i++) {
      if (node<=*(p.first+i)) {
	if (weights.size()!=0)
	  cout << node << " " << *(p.first+i) << " " << *(p.second+i) << endl;
	else
	  cout << node << " " << *(p.first+i) << endl;
      }
    }   
  }*/
  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<float>::iterator > p = neighbors(node);
    cout << correspondance[node] << ":" ;
    for (unsigned int i=0 ; i<nb_neighbors_out(node) ; i++) {
      if (true) {
	if (weights.size()!=0)
	  cout << " (" << correspondance[*(p.first+i)] << " " << *(p.second+i) << ")";
	else
	  cout << " " << correspondance[*(p.first+i)];
      }
    }
    cout << endl;
  }
}

/* Methode de reecriture du fichier */ 
void
Graph::writeFile(string outNeighbors, string inNeighbors) {

	ofstream foutput;
	foutput.open(outNeighbors.c_str() ,fstream::out | fstream::binary);
	
	cout << "Nombre de noeuds : " << nb_nodes << endl; 

	/* On recupere les voisins sortants */ 
	for(unsigned int node=0; node < nb_nodes; node++) {
	
		pair<vector<unsigned int>::iterator, vector<float>::iterator > p = neighbors(node);
	
		for(unsigned int i = 0; i < nb_neighbors_out(node); i++) {
			
			foutput << correspondance[node] << " " << correspondance[*(p.first+i)] << endl;
			
		}

	}

	foutput.close();

	ofstream foutputIn;
        foutputIn.open(inNeighbors.c_str(), fstream::out | fstream::binary);

	/* On recupere les voisins entrants */
	for(unsigned int node=0; node < nb_nodes; node++) {
	
		pair<vector<unsigned int>::iterator, vector<float>::iterator > p1 = in_neighbors(node);

		for(unsigned int i = 0; i < nb_neighbors_in(node); i++) {

				foutputIn << correspondance[node] << " " << correspondance[*(p1.first+i)] << endl;
		
		}

	}

}

void 
Graph::computeOverlap(string fileName) {

	ofstream foutput; 
	foutput.open(fileName.c_str(), fstream::out | fstream::binary);

	for(unsigned int node = 0; node < nb_nodes; node++) {

		int deg;

		if(node == 0) deg = 0; else deg = node - 1;

		unsigned int* outNeighbors = &links[degrees_out[deg]];
		unsigned int* inNeighbors = &links_in[degrees_in[deg]];

		vector<int>* inter = NULL; 
		inter = new vector<int>(max(nb_neighbors_out(node), nb_neighbors_in(node)));
		vector<int>::iterator it;

		sort(outNeighbors, outNeighbors + nb_neighbors_out(node));
		sort(inNeighbors, inNeighbors + nb_neighbors_in(node));

		it = set_intersection(outNeighbors, outNeighbors + nb_neighbors_out(node), inNeighbors, inNeighbors + nb_neighbors_in(node), (*inter).begin());
		(*inter).resize(it-(*inter).begin());

		float overlap;

		if(nb_neighbors_out(node) == 0 || nb_neighbors_in(node) == 0) 

			overlap = 0;

		else 

			overlap = max((*inter).size()/(float)nb_neighbors_out(node), (*inter).size()/(float)nb_neighbors_in(node)); 

		foutput << node << ";" << nb_neighbors_out(node) << ";" << nb_neighbors_in(node) << ";" <<  overlap << endl;  

		//delete[] outNeighbors; 
		//delete[] inNeighbors; 
		//delete inter;

	}

	foutput.close();

}

void
Graph::display_reverse() {
  for (unsigned int node=0 ; node<nb_nodes ; node++) {
    pair<vector<unsigned int>::iterator, vector<float>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors_out(node) ; i++) {
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
    pair<vector<unsigned int>::iterator, vector<float>::iterator > p = neighbors(node);
    for (unsigned int i=0 ; i<nb_neighbors_out(node) ; i++) {
      unsigned int neigh = *(p.first+i);
      float weight = *(p.second+i);
      
      pair<vector<unsigned int>::iterator, vector<float>::iterator > p_neigh = neighbors(neigh);
      for (unsigned int j=0 ; j<nb_neighbors_out(neigh) ; j++) {
	unsigned int neigh_neigh = *(p_neigh.first+j);
	float neigh_weight = *(p_neigh.second+j);

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
  foutput.write((char *)(&degrees_out[0]),4*nb_nodes);
  foutput.write((char *)(&links[0]),4*nb_links_out);
  foutput.write((char *)(&degrees_in[0]),4*nb_nodes);
  foutput.write((char *)(&links_in[0]),4*nb_links_in);
}

void
Graph::p_scc(int s, int p, set<int>& composante) {
	ofstream foutput;
	/*if (s == 0) {
	string filename = "composante1";
	foutput.open(filename.c_str() ,fstream::out);
	}
	else {
	string filename = "composanteAutre";
	foutput.open(filename.c_str() ,fstream::out);
	}*/
	stack<pair<int,int> > astack;
	stack<int> vpath, vpath_temp;
	int w, z, temp;
	pair<int, int> a;
	int c =(p + 2) /2;
	composante.insert(s);
	vpath.push(s);
	int debut, fin;
	if (s == 0)
		debut =0;
	else
		debut = degrees_out[s -1];
	fin = degrees_out[s];

	for (int i = debut; i < fin; i++) {
		astack.push(make_pair(s, links[i]));
		//foutput << "push de " << correspondance[links[i]] << endl;
	}
	while (!astack.empty()) {
		a = astack.top();

		astack.pop();
		w = vpath.top();
		//foutput << "a : " << a.first << ", "<< a.second << endl;
		//foutput << "w : " << w << endl;
		//if (a.first != w) {
			while (a.first != w) {
				vpath.pop();
				w=vpath.top();				
			}
	//		vpath.push(w);
	//	}
		//foutput << "w : " << w << endl;
		z = a.second;
		//foutput << "z : " << z << endl;
		if (z == s) {
			//foutput << "z == s " << z << endl;
			while (!vpath.empty()) {
				temp = vpath.top();
				vpath_temp.push(temp);
				composante.insert(temp);
				vpath.pop();
			}
			while (!vpath_temp.empty()) {
				temp = vpath_temp.top();
				vpath.push(temp);  
				vpath_temp.pop();
			}
		}
		else {
			//foutput << "z != s " << z << endl;
			if (vpath.size() < c) {
				if (z == 0)
					debut =0;
				else
					debut = degrees_out[z -1];
				fin = degrees_out[z];
				for (int i= debut; i < fin; i++) {
					astack.push(make_pair(z,links[i]));
				}
				vpath.push(z);
			}
		}
	}
	foutput.close();
}


void
Graph::p_scc_all(int p, int taille_min, string filename) {
	
	ofstream foutput;
	foutput.open(filename.c_str() ,fstream::out);
	ofstream foutput_log;
	foutput_log.open((filename + "_log").c_str() ,fstream::out);
	foutput_log << endl << "-------P-SCC-------" << endl << endl;
	cout << endl << "-------P-SCC -> Debut ! -------" << endl << endl;
	//ofstream foutputbin;
	//foutputbin.open((filename+"_bin").c_str() ,fstream::out | fstream::binary);
	
	int deg_cum =0;
	set<int>::iterator it;
	vector<set<int> > composantes;

	unsigned int nb_pscc= 0;
	composantes.resize(nb_nodes);
	//On ajoute toutes les composantes connexes dans un vecteur
	//Les composantes connexes sont representees par les set
	#pragma omp parallel for
	for (int k=0; k < nb_nodes; k++) {
		//foutput << correspondance[i] << " - ";
		//cout << k << endl;
		set<int> composante;
		p_scc(k, p, composante);
		//cout << k << "fin"<< endl;
		//foutput_log << "p-scc du sommet " << correspondance[k] << " : " << endl;
		if (composante.size() >= taille_min) {
			//composante.clear();
			composantes[k]=composante;
			/*for (it = composante.begin(); it != composante.end(); it++) {
				foutput_log << correspondance[(*it)] << " ";
			}
			foutput_log << endl;*/
			nb_pscc++;
		}
		else
			composante.clear();
		//composante.clear();
		//cout << k << " done" << endl;
	}
	cout << endl << "-------Tri -------" << endl << endl;
	//On trie le set en fonction de la taille des composantes
	sort(composantes.begin(), composantes.end(), setcomp());

	foutput_log << endl << "-------P-SCC triees-------" << endl << endl;
	for (int i=0; i < nb_pscc; i++) {
		//foutput_log << "pscc " << i << " : ";
		foutput_log << "pscc " << i << " : " << "[";
		for (it = composantes[i].begin(); it != composantes[i].end(); it++) {
			foutput_log << correspondance[(*it)] << ", ";

		}
		foutput_log << "]"<< endl;
		//composante.clear();
	}

	foutput_log << endl << "-------COMMUNITIES-------" << endl << endl;

	map<int, int> communities;
	std::map<int, int>::iterator itVertices;
	
	bool intersect;
	string toPrint;
	//On selectionne pour être des coeurs de communaute les composantes dans l'ordre de leur taille
	//Si une composante a une intersection avec une autre composante alors on la supprime
	int cpt_communities = 0;
	cout << endl << "-------Selection des coeurs -------" << endl << endl;
	for (int i=0; i < nb_pscc; i++) {
		intersect = false;
		
		for (it = composantes[i].begin(); it != composantes[i].end(); it++) {
			itVertices = communities.find(*it);
			if (itVertices != communities.end()) {
				intersect = true;
				/*for (itVertices = communities.begin(); itVertices != communities.end(); ++itVertices){
					cout << correspondance[(*itVertices).first] << " " << (*itVertices).second << endl;
				}*/
				//cout << "p-scc " << i  << " -> Intersection, sommet " << correspondance[(*it)]<< endl;
				break;
			}
		}
		if (!intersect) {
			for (it = composantes[i].begin(); it != composantes[i].end(); it++) {
				communities.insert(std::pair<int,int>((*it), cpt_communities));
			}
			cpt_communities++;
		}
		//composantes.erase(composantes.begin()+ i);
		/*if (!intersect) {
			for (it = composantes[i].begin(); it != composantes[i].end(); it++) {
				foutput << correspondance[(*it)] << " ";
				vertices.insert(*it);
			}
			foutput << endl;
		}
		else {
			composantes.erase(myvector.begin()+ i);
		}*/
	}
	composantes.clear();
	foutput_log << endl << "-------Selection des coeurs -------" << endl << endl;
	/*for(itVertices = communities.begin(); itVertices != communities.end(); ++itVertices){
		foutput_log << correspondance[(*itVertices).first] << " " << (*itVertices).second << endl;
	}*/
	foutput_log << endl << "-------Sommets isoles -------" << endl << endl;
	cout << endl << "-------Ajout des sommets isoles -------" << endl << endl;
	//On ajoute les sommets qui n'appartiennent pas encore à une communaute
	itVertices = communities.find(0);
	bool found;
	if (itVertices == communities.end()) {
		found = false;
		for (int i = 0; i < degrees_out[0]; i++) {
			itVertices = communities.find(links[i]);
			if (itVertices != communities.end()) {
				found = true;
				communities.insert(std::pair<int,int>(0, (*itVertices).second));
				break;
			}
		}
		if (!found) {
			for (int i = 0; i < degrees_in[0]; i++) {
				itVertices = communities.find(links_in[i]);
				if (itVertices != communities.end()) {
					found = true;
					communities.insert(std::pair<int,int>(0, (*itVertices).second));
					break;
				}
			}
		}		
	}
	bool clustering_ended = false;
	int limit = 10;
	while (!clustering_ended && (limit > 0)) {
		clustering_ended = true;
		for (int j = 1; j < nb_nodes; j++) {
			itVertices = communities.find(j);
			if (itVertices == communities.end()) {
				found = false;
				clustering_ended = false;
				for (int i = degrees_out[j -1]; i < degrees_out[j]; i++) {
					itVertices = communities.find(links[i]);
					if (itVertices != communities.end()) {
						found = true;
						communities.insert(std::pair<int,int>(j, (*itVertices).second));
						break;
					}
				}
				if (!found) {
					for (int i = degrees_in[j -1]; i < degrees_in[j]; i++) {
						itVertices = communities.find(links_in[i]);
						if (itVertices != communities.end()) {
							found = true;
							communities.insert(std::pair<int,int>(j, (*itVertices).second));
							break;
						}
					}
				}		
			}
		}
		limit --;
	}
	
	for(itVertices = communities.begin(); itVertices != communities.end(); ++itVertices){
		foutput << correspondance[(*itVertices).first] << " " << (*itVertices).second << endl;
	}
	cout << nb_nodes;
	foutput.close();
	foutput_log.close();
	/*foutputbin.write((char *)(&nb_nodes),4);
	for (int i=0; i < nb_nodes; i++) {
		deg_cum+=composantes[i].size();
		foutputbin.write((char *)(&deg_cum),4);
		cout << deg_cum << " ";
	}
	cout << endl;
	int c;
	for (int i=0; i < nb_nodes; i++) {
		for (it = composantes[i].begin(); it != composantes[i].end(); it++) {
			c= *it;
			cout << *it << " ";
			foutputbin.write((char *)(&c),4);
		}
		cout << endl;
	}

	foutputbin.close();*/

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
	Graph::avg_degree_in_comm(int comm) {

		long degree_comm_global = 0;

		for(unsigned int i = 0; i < communities[comm].size(); i++) {

			degree_comm_global += degree_in_com[communities[comm][i]];//degree_in_comm(communities[comm][i], comm);

		}

		return  degree_comm_global / communities[comm].size();
	}

	float
	Graph::avg_degree_out_comm(int comm) {

		long degree_comm_global = 0;

		for(unsigned int i = 0; i < communities[comm].size(); i++) {

		        degree_comm_global += degree_out_com[communities[comm][i]];

		}

		return  degree_comm_global / communities[comm].size();
	}


	float
	Graph::degree_out_comm(int node, int comm) {

		unsigned int pos_out, pos_out_end, cpt = 0;
		 
		if(node == 0) pos_out = 0; else pos_out = degrees_out[node - 1];
		pos_out_end=degrees_out[node];

		for (unsigned int i=pos_out; i < pos_out_end; i++) {
			if (map_communities[links[i]] == comm)
				cpt++;
		}    
		return cpt;
		/*unsigned int* outNeighbors = &links[pos_out];

		 Gives the position of the degree of the node in the cumulative sequence 

	      
		 We get its out- and in-neighbors and the vertices of the community 
		vector<int> community_vect = communities[comm];

		 We compute the intersection of out- and in- with C 
		vector<int> inter(max(nb_neighbors_out(node), (unsigned int)(community_vect.size())));
		vector<int>::iterator it;

		sort(outNeighbors, outNeighbors + nb_neighbors_out(node));
		sort(&community_vect[0], &community_vect[0] + (community_vect).size());

		it = set_intersection(outNeighbors, outNeighbors + nb_neighbors_out(node), &community_vect[0], &community_vect[0] + (community_vect).size(), inter.begin());
		inter.resize(it-inter.begin());

		return inter.size();*/

	}

	float
	Graph::degree_in_comm(int node, int comm) {


		unsigned int pos_out, pos_out_end, cpt = 0;
		 
		if(node == 0) pos_out = 0; else pos_out = degrees_in[node - 1];
		pos_out_end=degrees_in[node];
		    

		for (unsigned int i=pos_out; i < pos_out_end; i++) {
			if (map_communities[links_in[i]] == comm)
				cpt++;
		}    
		return cpt;

	}

	double
	Graph::variane_out(int comm) {

		double mean = avg_com_out[comm];
		/* TODO: do we calculate it here? Or before? */

		double tmp = 0;
		double degree_node = 0;

		for(unsigned int i = 0; i < communities[comm].size(); i++) {
	
			degree_node = degree_out_com[communities[comm][i]];
			tmp += (degree_node - mean) * (degree_node - mean);

		}

		return tmp / (double)communities[comm].size();

	}

	double
	Graph::variane_in(int comm) {

		double mean = avg_com_in[comm];
		/* TODO: do we calculate it here? Or before? */

		double tmp = 0;
		double tmp_bis = 0;
		double degree_node = 0;

		for(unsigned int i = 0; i < communities[comm].size(); i++) {

			degree_node = degree_in_com[communities[comm][i]];
			tmp_bis = degree_node - mean;
		        tmp += tmp_bis * tmp_bis;

		}

		return tmp / (double)communities[comm].size();

	}

	double 
	Graph::standard_deviation_out(int comm) {

		return sqrt(variane_out(comm));

	}

	double
	Graph::standard_deviation_in(int comm) {

		return sqrt(variane_in(comm));

	}

	double
	Graph::z_score_out(int node, int comm) {

		if(std_com_out[comm] == 0)

			return std::numeric_limits<double>::max();

		else

			return ((degree_out_com[node] - avg_com_out[comm]) / std_com_out[comm]);

	}

	double
	Graph::z_score_in(int node, int comm) {

		if(std_com_in[comm] == 0) 

			return numeric_limits<double>::max();

		else
	
			return ((degree_in_com[node] - avg_com_in[comm]) /  std_com_in[comm]);

	}

	double
	Graph::participation_out(int node) {

		double sum = 0;
		double tmp = 0;

		for (unsigned int i = 0; i < communities.size(); i++) {
		
			if(nb_neighbors_out(node) > 0) {

				tmp = (degree_out_comm(node, i) / nb_neighbors_out(node)); 
				sum += tmp * tmp;

			}

		}

		return (1 - sum);

	}

	double
	Graph::participation_in(int node) {

		double sum = 0;
		double tmp = 0;

		for(unsigned int i = 0; i < communities.size(); i++) {

			if(nb_neighbors_in(node) >0) {

				tmp = (degree_in_comm(node, i) / nb_neighbors_in(node));
				sum += tmp * tmp;

			}

		}

		return (1 - sum);

	}
	
	unsigned int
	Graph::diversite(int node, bool out) {
		int com_node = map_communities[node];
		set<unsigned int> comm_ext;
		int com;
		if (out) {
			int pos_out, pos_out_end = degrees_out[node];
			if(node == 0) pos_out = 0; else pos_out = degrees_out[node - 1];
			//cout << "degre : " << pos_out_end - pos_out << endl;
			for (int i = pos_out; i < pos_out_end; i++) {
			//	cout << "BIIIIIIIM "<< endl;
				com = map_communities[links[i]];
			//	cout << links[i] << " dans communaute " << com << endl;
				if (com != com_node)
					comm_ext.insert(com);
			}
			//cout << "Diversite : " << comm_ext.size() << endl;
		}
		else {
			int pos_in, pos_in_end = degrees_in[node];
			if(node == 0) pos_in = 0; else pos_in = degrees_in[node - 1];
			for (int i = pos_in; i < pos_in_end; i++) {
				com = map_communities[links_in[i]];
				if (com != com_node)
					comm_ext.insert(com);
			}
		}
		
		return comm_ext.size();
	}

	float
	Graph::avg_diversite(int comm, bool out) {

		long diversite_comm_global = 0;
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
			diversite_comm_global += diversite(communities[comm][i], out);
		}
		return  (float)diversite_comm_global / (float)communities[comm].size();
	}

	float
	Graph::variance_diversite(int comm, bool out) {
		//float mean = avg_diversite(comm, out);
		float mean;
		if (out)
			mean = avg_com_out[comm];
		else
			mean = avg_com_in[comm];
		/* TODO: do we calculate it here? Or before? */

		float tmp = 0;
		float diversite_node = 0;
		
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
	
			diversite_node = diversite(communities[comm][i], out);
			tmp += (diversite_node - mean) * (diversite_node - mean);

		}

		return tmp / (float)communities[comm].size();
	}

	float
	Graph::standard_deviation_diversite(int comm, bool out) {

		return sqrt(variance_diversite(comm, out));

	}

	float
	Graph::z_diversite(int node, bool out) {
		int com_node = map_communities[node];
		float std;
		if (out)
			std = std_com_out[com_node];
		else 
			std = std_com_in[com_node];
		
		if(std == 0) 
			return numeric_limits<double>::max();
		else {
			if (out)
				return (((float)diversite(node, out) - avg_com_out[com_node]) / std_com_out[com_node]);
			else
				return (((float)diversite(node, out) - avg_com_in[com_node]) / std_com_in[com_node]);
		}	
			
			//return (((float)diversite(node, out) - avg_diversite(com_node, out)) / std);
		
	}


	unsigned int
	Graph::intensite(int node, bool out) {
		int com_node = map_communities[node];
		set<unsigned int> comm_ext;
		int cpt = 0;
		
		if (out) {
			int pos_out, pos_out_end = degrees_out[node];
			if(node == 0) pos_out = 0; else pos_out = degrees_out[node - 1];
			for (int i = pos_out; i < pos_out_end; i++) {
				if (map_communities[links[i]] != com_node)
					cpt++;
			}
		}
		else {
			int pos_in, pos_in_end = degrees_in[node];
			if(node == 0) pos_in = 0; else pos_in = degrees_in[node - 1];
			for (int i = pos_in; i < pos_in_end; i++) {
				if (map_communities[links_in[i]] != com_node)
					cpt++;
			}
		}
		return cpt;
	}

	float
	Graph::avg_intensite(int comm, bool out) {
		//cout << "calcul moyenne " << endl;
		long intensite_comm_global = 0;
		//cout << "com : " << comm << "size : " << communities[comm].size() <<endl;
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
		//	cout << "intensite(" << communities[comm][i] <<", " << out<< ")";
			intensite_comm_global += intensite(communities[comm][i], out);
		//	cout << "ientesite " << intensite_comm_global << endl;
		}
		//cout << "taille " << communities[comm].size() << endl;
		return  (float)intensite_comm_global / (float)communities[comm].size();
	}

	float
	Graph::variance_intensite(int comm, bool out) {
		//cout << "laaaaa" << endl;
		float mean;
		if (out)
			mean = avg_com_out[comm];
		else
			mean = avg_com_in[comm];
		/* TODO: do we calculate it here? Or before? */
		//cout << "mean : " << mean << endl;
		float tmp = 0;
		float intensite_node = 0;
		//cout << "Ici" << endl;
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
	
			intensite_node = intensite(communities[comm][i], out);
		//	cout << "Noeud " << communities[comm][i] <<" -> Intensite : "<< intensite_node << endl;
			tmp += (intensite_node - mean) * (intensite_node - mean);

		}

		return tmp / (float)communities[comm].size();
	}

	float
	Graph::standard_deviation_intensite(int comm, bool out) {
		//cout << variance_intensite(comm, out) << endl;
		return sqrt(variance_intensite(comm, out));

	}

	float
	Graph::z_intensite(int node, bool out) {
		int com_node = map_communities[node];
		float std;
		if (out)
			std = std_com_out[com_node];
		else 
			std = std_com_in[com_node];
		
		if(std == 0) 
			return numeric_limits<double>::max();
		else {
			if (out)
				return (((float)intensite(node, out) - avg_com_out[com_node]) / std_com_out[com_node]);
			else
				return (((float)intensite(node, out) - avg_com_in[com_node]) / std_com_in[com_node]);
		}	
		
	}

	float
	Graph::homogeneite(int node, bool out) {
		int com_node = map_communities[node];
		//Un vecteur de taille égale au nombre de communautés, rempli de 0
		vector<unsigned int> communities_count(communities.size(),0);
		unsigned int value, communaute;
		unsigned int nb_com= 0, cpt = 0;
		if (out) {
			unsigned int pos_out, pos_out_end = degrees_out[node];
			if(node == 0) pos_out = 0; else pos_out = degrees_out[node - 1];
			for (unsigned int i = pos_out; i < pos_out_end; i++) {
				communaute = map_communities[links[i]];
				//cout << links[i] << " dans communaute " << communaute << endl;
				if (communaute != com_node) {
					cpt++;
					if (communities_count[communaute] == 0)
						nb_com++;
					communities_count[communaute]++;
				}
			}
		}
		else {
			unsigned int pos_in, pos_in_end = degrees_in[node];
			if(node == 0) pos_in = 0; else pos_in = degrees_in[node - 1];
			for (unsigned int i = pos_in; i < pos_in_end; i++) {
				communaute = map_communities[links_in[i]];
				if (communaute != com_node) {
					cpt++;
					if (communities_count[communaute] == 0)
						nb_com++;
					communities_count[communaute]++;
				}
			}
		}
		//cout << "nb_com " << nb_com << endl;
		if (nb_com == 0)
			return 0;
		float mean = (float)cpt / (float)nb_com;
		float tmp = 0, tmp_bis = 0;


		for (unsigned int i = 0 ; i < communities.size(); i++) {
			if (communities_count[i] != 0) {
				tmp_bis = (communities_count[i] - mean);
				tmp +=  tmp_bis * tmp_bis;
			}
			
		}
		//cout << tmp << endl;
		return sqrt(tmp / (float)nb_com);
		/*int com_node = map_communities[node];
		map<unsigned int, unsigned int> communities_count;
		map<unsigned int, unsigned int>::iterator it;
		int cpt = 0;
		int nb_com=0;
		if (out) {
			int pos_out, pos_out_end = degrees_out[node];
			int value, communaute;
			if(node == 0) pos_out = 0; else pos_out = degrees_out[node - 1];
			for (int i = pos_out; i < pos_out_end; i++) {
				communaute = map_communities[links[i]];
				//cout << links[i] << " dans communaute " << communaute << endl;
				if (communaute != com_node) {
					cpt++;
					it=communities_count.find(communaute);
					if (it == communities_count.end()) {
						//cout << "premiere fois" << endl;
						nb_com++;
						communities_count.insert(pair<int, int>(communaute, 1));
					}
					else {
						//cout << "autre fois" << endl;
						value= it->second;
						communities_count.erase(it);
						communities_count.insert(pair<int, int>(communaute, value + 1));
					}
				}
			}
		}
		else {
			int pos_in, pos_in_end = degrees_in[node];
			int value, communaute;
			if(node == 0) pos_in = 0; else pos_in = degrees_in[node - 1];
			for (int i = pos_in; i < pos_in_end; i++) {
				communaute = map_communities[links_in[i]];
				if (communaute != com_node) {
					it=communities_count.find(communaute);
					cpt++;
					if (it == communities_count.end()) {
						nb_com++;
						communities_count.insert(pair<int, int>(communaute, 1));
					}
					else {
						value= it->second;
						communities_count.erase(it);
						communities_count.insert(pair<int, int>(communaute, value + 1));
					}
				}
			}
		}
		//cout << "ici" << endl;
		if (nb_com == 0)
			return 0;
		float mean = (float)cpt / (float)nb_com;
		float tmp = 0;

		for (it = communities_count.begin(); it != communities_count.end(); it++) {
		//	cout << "it->second" << it->second;
			tmp += (it->second - mean) * (it->second - mean);
		}
		//cout << "mean : " << mean << "variance : " << tmp << endl;
		//cout << sqrt(tmp / (float)nb_com) << endl;
		return sqrt(tmp / (float)nb_com);*/
	}

	float
	Graph::avg_homogeneite(int comm, bool out) {

		float homogeneite_comm_global = 0;
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
			homogeneite_comm_global += homogeneite(communities[comm][i], out);
		}
		return  (float)homogeneite_comm_global / (float)communities[comm].size();
	}

	float
	Graph::variance_homogeneite(int comm, bool out) {
		float mean;
		if (out)
			mean = avg_com_out[comm];
		else
			mean = avg_com_in[comm];
		/* TODO: do we calculate it here? Or before? */

		float tmp = 0;
		float homogeneite_node = 0;
		
		for(unsigned int i = 0; i < communities[comm].size(); i++) {
			
			homogeneite_node = homogeneite(communities[comm][i], out);
			//cout << "homogeneite : " <<homogeneite_node << endl;
			tmp += (homogeneite_node - mean) * (homogeneite_node - mean);

		}
		//cout << "tmp  : "<< tmp <<endl;
		//cout << "size  : "<< communities[comm].size()<<endl;
		return tmp / (float)communities[comm].size();
	}

	float
	Graph::standard_deviation_homogeneite(int comm, bool out) {
		//cout << variance_homogeneite(comm, out) <<endl;
		return sqrt(variance_homogeneite(comm, out));

	}
	
	float
	Graph::z_homogeneite(int node, bool out) {
		int com_node = map_communities[node];
		float std;
		if (out)
			std = std_com_out[com_node];
		else 
			std = std_com_in[com_node];
		cout << "std dev : " << std << endl;
		if (out)
			cout << "avg : " << avg_com_out[com_node] << endl;
		else
			cout << "avg : " << avg_com_in[com_node] << endl;
		if(std == 0) 
			return numeric_limits<double>::max();
		else	{
			if (out)
				return ((homogeneite(node, out) - avg_com_out[com_node]) / std_com_out[com_node]);
			else
				return ((homogeneite(node, out) - avg_com_in[com_node]) / std_com_in[com_node]);
		}	
		
	}


	void
	Graph::writeGuimera(string filename) {

		ofstream foutput;
		

		//foutput << "node;z_score_out;z_score_in;community;k_out;k_in;variane_out;variane_in;d_out;d_in;div_out;div_in;int_out;int_in;hom_out;hom_in" << endl;
		//foutput << "node;community;z_score_out;z_score_in;z_diversite_out;z_diversite_in;z_intensite_out;z_intensite_in;z_homogeneite_out;z_homogeneite_in;"<< endl;			
		
		avg_com_out.resize(communities.size());
		std_com_out.resize(communities.size());
		avg_com_in.resize(communities.size());
		std_com_in.resize(communities.size());
		degree_in_com.resize(nb_nodes);
		degree_out_com.resize(nb_nodes);
		
		
		
		
		unsigned int i, thread_id;

		for (i=0; i < nb_nodes; i++) {
			degree_in_com[i]= degree_in_comm(i, map_communities[i]);
			degree_out_com[i]=degree_out_comm(i,map_communities[i]);
		}

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
				degree_in_com[i]= degree_in_comm(i, map_communities[i]);
				degree_out_com[i]=degree_out_comm(i,map_communities[i]);
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
				avg_com_out[i]= avg_degree_out_comm(i);
				std_com_out[i]= standard_deviation_out(i);
				avg_com_in[i] = avg_degree_in_comm(i);
				std_com_in[i] = standard_deviation_in(i);
				if (cpt % 500 == 0)
					cout << thread_id <<" a traité 500 communautés" << endl; 
				cpt++;
			}
			
		}
		

		foutput.open((filename+"_z_score").c_str(), fstream::out);
		cout << "Calcul du z-score " <<endl;
		for(i = 0; i < nb_nodes; i++) {
			foutput << i <<";" << map_communities[i] << ";";
			foutput << z_score_out(i, map_communities[i]) << ";" << z_score_in(i, map_communities[i]) << ";" << endl;			
			if (i % 1000000 == 0)
				cout << "1 million de noeuds en plus pour le z_score : présence !" << endl;

		}
	
		foutput.close();
		
		degree_in_com.clear();
		degree_out_com.clear();
		
		//D'abord la diversité
		cout << "Calcul des moyennes et ecart type pour la diversité " <<endl;
		#pragma omp parallel private(thread_id, foutput) 
 		{
			thread_id = omp_get_thread_num();
			stringstream ss;
			ss << thread_id;
			string filename2 = filename+"_diversite_" + ss.str();
			int cpt = 1;
			#pragma omp for
			for(i = 0; i < communities.size(); i++) {
				avg_com_out[i]= avg_diversite(i, true);
				std_com_out[i]= standard_deviation_diversite(i, true);
				avg_com_in[i] = avg_diversite(i, false);
				std_com_in[i] = standard_deviation_diversite(i, false);	
				if (cpt % 500 == 0)
					cout << thread_id <<" a traité 500 communautés" << endl; 
				cpt++;
			}
			cpt=1;
			#pragma omp barrier
			#pragma omp master 
			{
				cout << "Calcul de la diversité " <<endl;
			}
			
			//cout << filename2 << endl;
			foutput.open((filename2).c_str(), fstream::out);
			#pragma omp for
			for(i = 0; i < nb_nodes; i++) {
				foutput << i <<";" << map_communities[i] << ";";
				foutput << z_diversite(i, true) << ";" << z_diversite(i, false) << ";" << endl;
				if (cpt % 200000 == 0)
					cout << thread_id <<" a traité 200000 noeuds" << endl; 
				cpt++;	
			}
			foutput.close();
			cpt=1;
			#pragma omp barrier
			#pragma omp master 
			{
				//Maintenant, l'intensité
					cout << "Calcul des moyennes et ecart type pour l'intensité " <<endl;
			}
			#pragma omp for
			for(i = 0; i < communities.size(); i++) {
				avg_com_out[i]= avg_intensite(i, true);
				std_com_out[i]= standard_deviation_intensite(i, true);
				avg_com_in[i] = avg_intensite(i, false);
				std_com_in[i] = standard_deviation_intensite(i, false);
				if (cpt % 500 == 0)
					cout << thread_id <<" a traité 500 communautés" << endl; 
				cpt++;
			}
			cpt=1;
			#pragma omp barrier
			#pragma omp master 
			{
				//Maintenant, l'intensité
					cout << "Calcul de l'intensité " <<endl;
			}
			filename2 = filename+"_intensite_" + ss.str();
			foutput.open((filename2).c_str(), fstream::out);
			#pragma omp for
			for(i = 0; i < nb_nodes; i++) {
			
				foutput << i <<";" << map_communities[i] << ";";
				foutput << z_intensite(i, true) << ";" << z_intensite(i, false) << ";" << endl;
				if (cpt % 200000 == 0)
					cout << thread_id <<" a traité 200000 noeuds" << endl; 
				cpt++;

			}
			foutput.close();
			cpt=1;
			#pragma omp barrier
			#pragma omp master 
			{
				//Maintenant, l'homogénéité
					cout << "Calcul des moyennes et ecart type pour l'homogénéité " <<endl;
			}
			//Maintenant, l'homogénéité
			#pragma omp for
			for(i = 0; i < communities.size(); i++) {
				avg_com_out[i]= avg_homogeneite(i, true);
				std_com_out[i]= standard_deviation_homogeneite(i, true);
				avg_com_in[i] = avg_homogeneite(i, false);
				std_com_in[i] = standard_deviation_homogeneite(i, false);
				if (cpt % 500 == 0)
					cout << thread_id <<" a traité 500 communautés" << endl; 
				cpt++;
	
			}
			cpt=1;
			#pragma omp barrier
			#pragma omp master 
			{
				//Maintenant, l'homogénéité
					cout << "Calcul de l'homogénéité " <<endl;
			}
			filename2 = filename+"_homogeneite_" + ss.str();
			//cout << filename2 << endl;
			foutput.open((filename2).c_str(), fstream::out);
			#pragma omp for
			for(i = 0; i < nb_nodes; i++) {
			
				foutput << i <<";" << map_communities[i] << ";";
				foutput << z_homogeneite(i, true) << ";" << z_homogeneite(i, false) << ";"<<endl;	
				if (cpt % 200000 == 0)
					cout << thread_id <<" a traité 200000 noeuds" << endl; 
				cpt++;

			}
			foutput.close();
		}
	
		

		/*for(unsigned int i = 0; i < nb_nodes; i++) {
			//cout << i << "laaa" << endl;
			foutput << i <<";" << map_communities[i] << ";" << z_score_out(i, map_communities[i]) << ";" << z_score_in(i, map_communities[i]) << ";" <<endl;
			foutput  << "Diversite : " << " - " << diversite(i, true) << ";" << diversite(i, false) << ";" << endl << "Diversite moyenne communaute " << map_communities[i] << " - " << avg_diversite(map_communities[i], true)<< ";" <<avg_diversite(map_communities[i], false)<< endl << "Std deviation diversite : " << standard_deviation_diversite(map_communities[i], true) << ";"<< standard_deviation_diversite(map_communities[i], false) ;
			foutput << endl << "Z-Diversite : "<< z_diversite(i, true) << ";" << z_diversite(i, false) << ";" <<endl;
			foutput  << "Intensite : "<< intensite(i, true) << ";" << intensite(i, false) << ";";
			
			foutput << endl << "Intensite  moyenne communaute : " << map_communities[i] << " - " << avg_intensite(map_communities[i], true) << ";" << avg_intensite(map_communities[i], false) << ";";
			//cout <<"Ecart type de " << i << ", comm : " <<map_communities[i] << endl;
			foutput << endl << "Ecarte type Intensite communaute : " << map_communities[i] << " - " << standard_deviation_intensite(map_communities[i], true) << ";" << standard_deviation_intensite(map_communities[i], false) << ";";
			//cout <<"Z-intensite de " << i << ", comm : " <<map_communities[i] << endl;
			foutput << endl << "Z-Intensite : "<< z_intensite(i, true) << ";" << z_intensite(i, false) << ";"<<endl;

			foutput << "Homogeneite : "<< homogeneite(i, true) << ";" << homogeneite(i, false) << ";";
			foutput << endl << "Homogeneite  moyenne communaute : " << map_communities[i] << " - " << avg_homogeneite(map_communities[i], true) << ";" << avg_homogeneite(map_communities[i], false) << ";";
			foutput << endl << "Ecarte type Homogeneite communaute : " << map_communities[i] << " - " << standard_deviation_homogeneite(map_communities[i], true) << ";" << standard_deviation_homogeneite(map_communities[i], false) << ";";
			foutput << endl << "Z-Homogeneite : "<< z_homogeneite(i, true) << ";" << z_homogeneite(i, false) << ";";

			foutput << endl << endl;
			//cout << "intensite : done" << endl;
			//foutput << homogeneite(i, true) << ";" <<endl;// << homogeneite(i, false) << ";" << endl;
			//cout << "homogeneite : done" << endl;


			
			foutput << i <<";" << map_communities[i] << ";";
			foutput << z_score_out(i, map_communities[i]) << ";" << z_score_in(i, map_communities[i]) << ";";
			foutput << z_diversite(i, true) << ";" << z_diversite(i, false) << ";";
			foutput << z_intensite(i, true) << ";" << z_intensite(i, false) << ";";
			foutput << z_homogeneite(i, true) << ";" << z_homogeneite(i, false) << ";"<<endl;	
			
			if (i % 1000000 == 0)
				cout << "1 million de noeuds en plus : présence !" << endl;

			

		}

		foutput.close();*/


	}

	float
	Graph::c_coefficient(int node, bool out) {
		vector<bool> neighbors (nb_nodes, false);
		unsigned int pos, pos_end, pos_neigh, pos_neigh_end, cpt = 0;
		if (out) {
			pos_end = degrees_out[node];
			if(node == 0) pos = 0; else pos = degrees_out[node - 1];
		}
		else {
			pos_end = degrees_in[node];
			if(node == 0) pos = 0; else pos = degrees_in[node - 1];
		}
		for (int i = pos; i < pos_end; i++) {
			neighbors[i] = true;
		}
		for (int i = pos; i < pos_end; i++) {
			if (out) {
				pos_neigh_end = degrees_out[links[i]];
				if(node == 0) pos_neigh = 0; else pos_neigh = degrees_out[links[i]];
			}
			else  {
				pos_neigh_end = degrees_out[links_in[i]];
				if(node == 0) pos = 0; else pos_neigh = degrees_in[links_in[i]];
			}
			for (int i = pos_neigh; i < pos_neigh_end; i++) {
				if (links[i] || links_in[i])
					cpt++;
			}
		}
		return cpt / (pos_end - pos)* (pos_end - pos - 1);
	}
	float
	Graph::c_coefficient(int node) {
		vector<bool> neighbors (nb_nodes, false);
		set<unsigned int> voisins;
		unsigned int pos, pos_end, pos_neigh, pos_neigh_end, cpt = 0, k;
		//Out
		pos_end = degrees_out[node];
		if(node == 0) pos = 0; else pos = degrees_out[node - 1];
		for (int i = pos; i < pos_end; i++) {
			neighbors[links[i]] = true;
			voisins.insert(links[i]);
		}
		//In
		pos_end = degrees_in[node];
		if(node == 0) pos = 0; else pos = degrees_in[node - 1];
		for (int i = pos; i < pos_end; i++) {
			neighbors[links_in[i]] = true;
			voisins.insert(links_in[i]);
		}
		set<unsigned int>::iterator myIterator;
		for(myIterator = voisins.begin();myIterator != voisins.end();myIterator++) {
			k=*myIterator;
			//cout << k << endl;
			pos_neigh_end = degrees_out[k];
			if(k == 0) pos_neigh = 0; else pos_neigh = degrees_out[k -1];
			for (unsigned int j = pos_neigh; j < pos_neigh_end; j++) {
				//cout << "\t" << links[j] << " : " << neighbors[links[j]]<< endl;
				if (neighbors[links[j]])
					cpt++;
			}
		}
		//cout << "Nb de links trouvés :" << cpt<< endl;
		//cout << "Nb de voisins " << voisins.size()<< endl;
		return (float)cpt / (float)(voisins.size() * (voisins.size() - 1));
	}

	//Pour chaque capitaliste social, savoir combien de voisins sortants capitalistes sociaux il a
	void
	Graph::cap_sociaux_out(string file) {
		vector<bool> neighbors (nb_nodes, false);
		std::ifstream fichier( file.c_str());
		unsigned int pos, pos_end, cpt = 0, node;
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
			    // afficher la ligne à l'écran
			    neighbors[atoi(ligne.c_str())] = true;
			}
		}
		fichier.close();
		fichier.open ( file.c_str());
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
				node =atoi(ligne.c_str());
				cpt = 0;
			  	pos_end = degrees_out[node];
				if(node == 0) pos = 0; else pos = degrees_out[node - 1];
				for (int i = pos; i < pos_end; i++) {
					if (neighbors[links[i]])
						cpt++;
				}
				std::cout << node << ";" << cpt << std::endl;
			}
		}
		fichier.close();
	}

	void
	Graph::cap_sociaux_in(string file) {
		vector<bool> neighbors (nb_nodes, false);
		std::ifstream fichier( file.c_str());
		unsigned int pos, pos_end, cpt = 0, node;
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
			    // afficher la ligne à l'écran
			    neighbors[atoi(ligne.c_str())] = true;
			}
		}
		fichier.close();
		fichier.open ( file.c_str());
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
				node =atoi(ligne.c_str());
				cpt = 0;
			  	pos_end = degrees_in[node];
				if(node == 0) pos = 0; else pos = degrees_in[node - 1];
				for (int i = pos; i < pos_end; i++) {
					if (neighbors[links_in[i]])
						cpt++;
				}
				std::cout << node << ";" << cpt << std::endl;
			}
		}
		fichier.close();
	}

	void
	Graph::cap_sociaux_voisins_count(string file) {
		vector<bool> neighbors (nb_nodes, false);
		vector<unsigned int> voisins (nb_nodes, 0);
		std::ifstream fichier( file.c_str());
		unsigned int pos, pos_end, cpt = 0, node;
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
			    // afficher la ligne à l'écran
			    neighbors[atoi(ligne.c_str())] = true;
			}
		}
		fichier.close();
		fichier.open ( file.c_str());
		if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
		{
			std::string ligne; // variable contenant chaque ligne lue

			// cette boucle s'arrête dès qu'une erreur de lecture survient
			while ( std::getline( fichier, ligne ) )
			{
				node =atoi(ligne.c_str());
				cpt = 0;
			  	pos_end = degrees_in[node];
				if(node == 0) pos = 0; else pos = degrees_in[node - 1];
				for (int i = pos; i < pos_end; i++) {
					if (!neighbors[links_in[i]])
						voisins[links_in[i]]++;
				}
			}
	
			for (unsigned int i = 0; i < nb_nodes; i++) {
				if (voisins[i] > 0)
					cout << i << ";" << voisins[i] << endl;
			}
		}
		fichier.close();
	}


	
