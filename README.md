# CommunityRoles

This package was programmed by [Nicolas Dugué] (http://www.univ-orleans.fr/lifo/membres/Nicolas.Dugue/) and [Anthony perez] (http://www.univ-orleans.fr/lifo/membres/Anthony.Perez) to be able to compute the community roles of graph nodes.
To know more about Community roles, please consider the following readings : 
* **[1]** Nicolas Dugué, Vincent Labatut, Anthony Perez. A community role approach to assess social capitalists visibility in the Twitter network. Social Network Analysis and Mining, Springer, 2015, 5, pp.26.
* **[2]** Klimm, F., Borge-Holthoefer, J., Wessel, N., Kurths, J., & Zamora-López, G. (2014). Individual nodeʼs contribution to the mesoscale of complex networks. New Journal of Physics, 16(12), 125006.
* **[3]** Guimera, R., & Amaral, L. A. N. (2005). Functional cartography of complex metabolic networks. Nature, 433(7028), 895-900.

The data structure used to store graphs is the same as in the [Louvain algorithm] (https://sites.google.com/site/findcommunities/) and the [Directed Louvain] (https://github.com/nicolasdugue/DirectedLouvain). 

----------------------------------
**Description**

The folder *guimera-original* implements measures described in **[3]** while in the folder *directed-roles*, measures of **[1]** are coded.
In each folder, use ***make*** to compile the sources and obtain binaries.

--------------------------------
**Parallelism**

To be able to use multi-cores, OpenMP should be installed on your computer. Otherwise, computations would be run on a single core.
To specify the number of cores to use with OpenMp:
export OMP_NUM_THREADS=nb_proc

--------------------------------
**Man**

Let's take the example of a graph **graphes/easygraphe**.

Convert the graph ***easygraph** in a binary format and store it in the *convert* folder :

    ./bin/convert -i graphes/easygraphe -o convert/easygraphe -r

Compute the communities with [Louvain algorithm] (https://sites.google.com/site/findcommunities/) or [Directed Louvain] (https://github.com/nicolasdugue/DirectedLouvain) and store the result as **results/result_louvain**.

Compute the community roles of each node :

    ./bin/guimera convert/easygraphe results/result_louvain results/result_guimera
