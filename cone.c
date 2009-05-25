/*
 *  cone.c
 *  
 *
 *  Created by Nico Van Cleemput on 19/05/09.
 *
 */

#include "cone.h"
#include "pseudoconvex.h"
#include "pseudoconvexuser.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//set to 1 when only the number of structures needs to be counted
int onlyCount = 0;

//should ipr be used?
int ipr = 0;

//
int mirror = 0;

char outputType = 'p';

int structureCounter = 0;

/*
The minimum length of the (shortest) side for which there exists a
canonical cone patch.
*/
int symmetricMinima[5] = {0,1,1,2,5};
int symmetricMinimaIPR[5] = {0,1,2,4,9};
int nearsymmetricMinima[3] = {0,1,2};
int nearsymmetricMinimaIPR[3] = {1,2,4};

void exportStructure(INNERSPIRAL *is){
	structureCounter++;
	if(onlyCount) return;
	switch(outputType) {
		case 's':
			exportInnerSpiral(is);
			break;
		case 'p':
			exportPlanarGraphCode(is);
			break;
		case 't':
			exportPlanarGraphCode(is);
			break;
	}
}

/*
print a usage message. name is the name of the current program.
*/
void usage(char *name) {
	fprintf(stderr, "Usage: %s [options] (# pentagons) (shortest 'side') (n/s) [(# hexagon layers)]\n", name);
	fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
print a help message. name is the name of the current program.
*/
void help(char *name) {
	fprintf(stderr, "The program %s calculates canonical conepatches.\n", name);
	fprintf(stderr, "Usage: %s [options] (# pentagons) (shortest 'side') (n/s) [(# hexagon layers)] \n\n", name);
	fprintf(stderr, "Valid options:\n");
	fprintf(stderr, "  -h          : Print this help and return.\n");
	fprintf(stderr, "  -m          : Mirror-images are considered nonisomorphic.\n");
	fprintf(stderr, "  -i          : Use IPR-rule.\n");
	fprintf(stderr, "  -c          : Only count structures don't export them.\n");
	fprintf(stderr, "  -e c        : Specifies the export format where c is one of\n");
	fprintf(stderr, "                p    planar code (default)\n");
	fprintf(stderr, "                t    adjacency lists in tabular format\n");
	fprintf(stderr, "                s    inner spirals\n");
}

/**
 * Validates the parameters and writes an error message if they are not valid.
 */
boolean validate(int pentagons, int sside, int hexagonLayers, boolean symmetric){
	if(hexagonLayers < 0){
		fprintf(stderr, "The number of hexagon layers needs to be a non-negative number.\n");
		return 0;
	}

	if(pentagons < 1 || pentagons > 5){
		fprintf(stderr, "A cone needs to have between 1 and 5 pentagons.\n");
		return 0;
	}

	if(sside < 0){
		fprintf(stderr, "The shortest side needs to have a positive length.\n");
		return 0;
	}

	if(!symmetric && (pentagons ==1 || pentagons == 5)){
		fprintf(stderr, "A cone with 1 or 5 pentagons cannot be nearsymmetric.\n");
		return 0;
	}
	
	return 1;
}

int main(int argc, char *argv[]) {

	/*=========== commandline parsing ===========*/

	int c, i;
	char *name = argv[0];
	
	int pentagons, sside, hexagonLayers;
	boolean symmetric;
  
	while ((c = getopt(argc, argv, "imche:")) != -1) {
		switch (c) {
			case 'i':
				ipr = 1;
				break;
			case 'm':
				mirror = 1;
				break;
			case 'c':
				onlyCount = 1;
				break;
			case 'e':
				outputType = optarg[0];
				switch(outputType) {
					case 'p':
					case 's':
					case 't':
						break;
					default:
						usage(name);
						return 1;
				}
				break;
			case 'h':
				help(name);
				return 0;
			default:
				usage(name);
				return 1;
		}
	}

	// check the non-option arguments
	if(argc - optind < 3 || argc - optind > 4){
		usage(name);
		return 1;
	}
	
	i = optind;

	//parse the number of hexagons
	pentagons = strtol(argv[i++], NULL, 10);
	
	//parse the length of the shortest side
	sside = strtol(argv[i++], NULL, 10);
	
	switch (*(argv[i++])) {
		case 's':
			symmetric = 1;
			break;
		case 'n':
			symmetric = 0;
			break;
		default:
			usage(name);
			return 1;
	}
	
	if(i<argc){
		hexagonLayers = strtol(argv[i++], NULL, 10);
	} else {
		hexagonLayers = 0;
	}
		
	/*=========== parameter validation ===========*/
	
	if(!validate(pentagons, sside, hexagonLayers, symmetric)){
		return 1;
	}

	/*=========== generation ===========*/

	//check the fixed minima lengths for sside
	if(symmetric){
		if(ipr){
			if(symmetricMinimaIPR[pentagons-1]>sside){
				fprintf(stderr, "There are no symmetric cones with IPR, %d pentagons and side length %d.\n", pentagons, sside);
				return 0;
			}
		} else {
			if(symmetricMinima[pentagons-1]>sside){
				fprintf(stderr, "There are no symmetric cones with %d pentagons and side length %d.\n", pentagons, sside);
				return 0;
			}
		}
	} else {
		if(ipr){
			if(nearsymmetricMinimaIPR[pentagons-2]>sside){
				fprintf(stderr, "There are no nearsymmetric cones with IPR, %d pentagons and shortest side length %d.\n", pentagons, sside);
				return 0;
			}
		} else {
			if(nearsymmetricMinima[pentagons-2]>sside){
				fprintf(stderr, "There are no nearsymmetric cones with %d pentagons and shortest side length %d.\n", pentagons, sside);
				return 0;
			}
		}
	}
	
	if(symmetric)
		fprintf(stderr, "Generating all symmetric cones with side length %d and %d pentagons and surrounding the patches with %d hexagon layers.\n", sside, pentagons, hexagonLayers);
	else
		fprintf(stderr, "Generating all nearsymmetric cones with side length %d and %d pentagons and surrounding the patches with %d hexagon layers.\n", sside, pentagons, hexagonLayers);

	return 0;
}