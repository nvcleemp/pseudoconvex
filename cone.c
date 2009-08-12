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
#include "twopentagons.h"
#include "util.h"

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

void processStructure(PATCH *patch, SHELL *shell){
	structureCounter++;
	if(onlyCount) return;
	switch(outputType) {
		case 'i':
			exportInnerSpiral(patch);
			break;
		case 'x':
			exportExtendedInnerSpiral(patch);
			break;
		case 's':
			exportShells(shell);
			break;
		case 'p':
			exportPlanarGraphCode(patch);
			break;
		case 't':
			exportPlanarGraphCode(patch);
			break;
	}
}

boolean validateStructure(PATCH *patch){
	int i;
	int symmetric;
	if(symmetric){
		//first check all other break-edges in clockwise direction
		for(i=1; i<6-patch->numberOfPentagons; i++){
		
		}
		if(!mirror){
			//check all break-edges in counterclockwise direction
			
		}
	} else if(!mirror){
		//only one other spiral needs to be investigated
		
	} //there are no ther cases: a nearsymmetric patch is always canonical if mirror images are considered nonisomorphic
	return 1;
}

void start5PentagonsCone(PATCH *patch, int sside, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell){
	FRAGMENT *current = addNewFragment(currentFragment);
	SHELL *shell = addNewShell(currentShell, sside, current);
	if(patch->firstFragment==NULL){
		patch->firstFragment = current;
		patch->outershell = shell;
	}
	int upperbound = (mirror ? sside-1 : HALFFLOOR(sside)+1);
	patch->outershell = shell;
	INNERSPIRAL *is = patch->innerspiral;
	shell->nrOfBreakEdges = 1;
	shell->breakEdge2FaceNumber[0]=0;
	shell->nrOfPossibleStartingPoints = 0;
	shell->nrOfPossibleMirrorStartingPoints = mirror ? 0 : 1;
	shell->mirrorStartingPoint2BreakEdge[0]=0;
	shell->mirrorStartingPoint2FaceNumber[0]=0;

	//pentagon after i hexagons
	int i;
	for(i=0; i<upperbound; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;
		
		current->faces = i+1;
		current->endsWithPentagon = 1;
		
		shell->nrOfPentagons = 1;
		
		fillPatch_4PentagonsLeft(sside-2-i, 1+i, patch, addNewFragment(current), sside-i-1, shell);
		is->position--;
		is->code[is->position]-=i;
	}

	if(mirror){
		//pentagon after sside-1 hexagons
		is->code[is->position]+=sside-1;
		is->position++;
		is->code[is->position]=0;
		
		current->faces = sside-1;
		current->endsWithPentagon = 1;
		
		shell->nrOfPentagons = 1;
		
		fillPatch_4PentagonsLeft(0, sside-2, patch, addNewFragment(current), 0, shell);
		is->position--;
		is->code[is->position]-=sside-1;
	}
}

void start4PentagonsCone(PATCH *patch, int sside, int symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell){
	FRAGMENT *current = addNewFragment(currentFragment);
	current->endsWithPentagon = 1;
	SHELL *shell = addNewShell(currentShell, 2*sside+(symmetric ? 0 : 1), current);
	if(patch->firstFragment==NULL){
		patch->firstFragment = current;
		patch->outershell = shell;
	}
	int lside = (symmetric ? sside : sside + 1);
	int upperbound = (mirror ? sside : HALFFLOOR(sside)+1);
	patch->outershell = shell;
	INNERSPIRAL *is = patch->innerspiral;

	//pentagon after i hexagons
	int i;
	for(i=0; i<upperbound; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;

		current->faces = i+1;
		
		shell->nrOfPentagons = 1;
		
		fillPatch_3PentagonsLeft(sside-1-i, lside-1, 1+i, patch, addNewFragment(current), 2*sside+(symmetric ? 0 : 1) -i-1, shell);
		is->position--;
		is->code[is->position]-=i;
	}
}

void start3PentagonsCone(PATCH *patch, int sside, int symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell){
	FRAGMENT *current = addNewFragment(currentFragment);
	SHELL *shell = addNewShell(currentShell, 3*sside + (symmetric ? 0 : 2), current);
	if(patch->firstFragment==NULL){
		patch->firstFragment = current;
		patch->outershell = shell;
	}
	int lside = (symmetric ? sside : sside + 1);
	int upperbound = (mirror ? sside : HALFFLOOR(sside)+1);
	patch->outershell = shell;
	INNERSPIRAL *is = patch->innerspiral;
	
	//pentagon after i hexagons
	int i;
	for(i=0; i<upperbound; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;
		
		current->faces = i+1;
		current->endsWithPentagon = 1;
		
		shell->nrOfPentagons = 1;
		
		fillPatch_2PentagonsLeft(sside-1-i, lside, lside-1, 1+i, patch, addNewFragment(current), 3*sside + (symmetric ? 0 : 2) - i-1, shell);
		is->position--;
		is->code[is->position]-=i;
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
	fprintf(stderr, "                i    inner spirals\n");
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
					case 'i':
					case 'p':
					case 's':
					case 't':
					case 'x':
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
	
	//create the data structure for the pseudoconvex patch
	PATCH *patch = (PATCH *)malloc(sizeof(PATCH));
	patch->numberOfPentagons = pentagons;
	patch->boundary = (int *)malloc((6-pentagons)*sizeof(int));
	patch->boundary[0] = sside;
	{
		int i;
		for(i=1;i<(6-pentagons);i++) patch->boundary[i] = sside + 1 - symmetric;
	}

	patch->outershell = NULL;
			
	//determine the amount of hexagons to add for the given number of layers
	int hexagonsToAdd = 0;
	FRAGMENT *currentFragment = NULL;
	SHELL *currentShell = NULL;
	if(onlyCount) hexagonLayers = 0;
	{
		int i;
		for(i = 0; i<hexagonLayers; i++){
			hexagonsToAdd += (sside + 1 - symmetric + hexagonLayers - i)*(6-pentagons) - (1 - symmetric);
			if(i==0){
				patch->firstFragment = createLayersFragment(NULL, (sside + 1 - symmetric + hexagonLayers - i)*(6-pentagons) - (1 - symmetric));
				currentFragment = patch->firstFragment;
				patch->outershell = addNewShell(NULL, currentFragment->faces, currentFragment);
				currentShell = patch->outershell;
			} else {
				currentFragment = createLayersFragment(currentFragment, (sside + 1 - symmetric + hexagonLayers - i)*(6-pentagons) - (1 - symmetric));
				currentShell = addNewShell(currentShell, currentFragment->faces, currentFragment);
			}
		}
	}

	
	//create the innerspiral and add the initial hexagons
	INNERSPIRAL *is = getNewSpiral(pentagons);
	is->code[0] = hexagonsToAdd;
	patch->innerspiral = is;
	
	//start the algorithm
	if(pentagons==1){
		if(sside==0) {
			FRAGMENT *frag = addNewFragment(currentFragment);
			frag->faces = 1;
			frag->endsWithPentagon = 1;

			processStructure(patch, addNewShell(currentShell, 1, frag));
		}
	} else if(pentagons==2){
		if(onlyCount)
			structureCounter = getTwoPentagonsConesCount(sside, symmetric, mirror);
		else
			getTwoPentagonsCones(patch, sside, symmetric, mirror, currentFragment, currentShell);
	} else if(pentagons==3){
		start3PentagonsCone(patch, sside, symmetric, mirror, currentFragment, currentShell);
	} else if(pentagons==4){
		start4PentagonsCone(patch, sside, symmetric, mirror, currentFragment, currentShell);
	} else {
		start5PentagonsCone(patch, sside, mirror, currentFragment, currentShell);
	}
	
	//print the results
	//fprintf(stderr, "Found %d canonical cones satisfying the given parameters.\n", structureCounter);
	fprintf(stderr, "Found %d cones satisfying the given parameters.\n", structureCounter);

	return 0;
}