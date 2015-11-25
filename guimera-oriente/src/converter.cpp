// File: graph.cpp
// -- simple graph handling source file
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

#include<string.h>
#include "../include/converter.h"

using namespace std;
Converter::Converter(char *filename, int type, bool do_renumber) {
  ifstream finput;
  finput.open(filename,fstream::in); 
  if(do_renumber) {	
  
  unsigned int src, dest, cpt;
  cpt = 0;

	unsigned int src_prec, dest_prec;
        src_prec = -1;
        dest_prec = -1;
        
  int nb_links=0;
  double weight = 1.;
  ofstream foutput;
	char* tmp = new char[strlen(filename) + 6];
	strcat(tmp, filename);
	strcat(tmp, "_renum");
  foutput.open(tmp, fstream::out);

		cout << "Renumerotation begins..." << endl;
		
		correspondance.resize(0);

	/* Find minimum and maximum value of ids */
      unsigned int min, maxi;

        if(type == WEIGHTED)
                finput >> src >> dest >> weight;
        else
                finput >> src >> dest;

        min = src;
        maxi = dest;
	cout << "Debut" << endl;
        while(!finput.eof()) {
		nb_links++;
                finput >> src >> dest;

                if(min > src)
                        min = src;
                if(min > dest)
                        min = dest;

                if(maxi < src)
                        maxi = src;
                if(maxi < dest)
                        maxi = dest;
		//if(nb_links % 1000 == 0) cout << "10000000 lus" << endl;

        }

        finput.close();
	cout << "Maximum found : " << maxi << endl;

        /* Creating a bunch of maps */
        vector<map<unsigned int, unsigned int> > corres;
        int interval = 1000;

	corres.resize((maxi/interval)+1);
        unsigned int nb_interval_src, nb_interval_dest;

        finput.open(filename,fstream::in);

        if(finput) {

        while(!finput.eof()) {

			if(finput.eof()) 

				break;

                        if(type == WEIGHTED)
                                finput >> src >> dest >> weight;
                        else
                                finput >> src >> dest;

			 			if(src_prec == src && dest_prec == dest)

                                continue;

			nb_interval_src = src / interval;
                        if(corres[nb_interval_src].find(src) == corres[nb_interval_src].end()) {
				corres[nb_interval_src].insert(make_pair(src, cpt));
                                correspondance.resize(correspondance.size()+1);
                                correspondance[cpt] = src;
				foutput << cpt << " " << src << endl;
                                cpt++;
                        }

			nb_interval_dest = dest / interval;
                        if(corres[nb_interval_dest].find(dest) == corres[nb_interval_dest].end()) {
				corres[nb_interval_dest].insert(make_pair(dest, cpt));
                                correspondance.resize(correspondance.size()+1);
                                correspondance[cpt] = dest;
				foutput << cpt << " " << dest << endl;
                                cpt++;
                        }
                        
					
					      nb_links++;
	
		// TODO: count nb_links while parsing for the maximum, and pourcent the progression 			
		if(nb_links % 10000000 == 0) cout << "10000000 ecrits" << endl;


                     	src_prec = src;
                        dest_prec = dest;

                }

        }
	finput.close();

	cout << "Renumerotation ends..." << endl;
	foutput.close();


  }
    
  finput.close();

}
