/*
 *  pseudoconvex.c
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#include "pseudoconvex.h"
#include "pseudoconvexuser.h"

#include <stdio.h>
#include <stdlib.h>

/*========== DATA STRUCTURES ===========*/

INNERSPIRAL *getNewSpiral(int numberOfPentagons){
	INNERSPIRAL *is = (INNERSPIRAL *)malloc(sizeof(INNERSPIRAL));
	is->code = malloc(sizeof(int)*numberOfPentagons);
	return is;
}

/*========== EXPORT ===========*/
void exportPlanarGraphCode(INNERSPIRAL *is){

}

void exportPlanarGraphTable(INNERSPIRAL *is){

}

void exportInnerSpiral(INNERSPIRAL *is){
	if(is->length==0){
		fprintf(stderr, "patch without pentagons\n");
		return;
	}
	int i;
	for(i=0; i<is->length-1; i++)
		fprintf(stderr, "%d, ", is->code[i]);
	fprintf(stderr, "%d\n", is->code[is->length-1]);
}

/*========== CONSTRUCTION ==========*/
void fillPatch_5PentagonsLeft(int k, INNERSPIRAL *is){
	if(k<=0)
		return;
	
	//add a side of hexagons
	is->code[is->position]+=k;
	fillPatch_5PentagonsLeft(k-1, is);
	is->code[is->position]-=k;
	
	//pentagon after i hexagons
	int i;
	for(i=0; i<k-1; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;
		fillPatch_4PentagonsLeft(k-2-i, 1+i, is);
		is->position--;
		is->code[is->position]-=i;
	}

	//pentagon after k-1 hexagons
	is->code[is->position]+=k-1;
	is->position++;
	is->code[is->position]=0;
	fillPatch_4PentagonsLeft(0, k-2, is);
	is->position--;
	is->code[is->position]-=k-1;
}

void fillPatch_4PentagonsLeft(int k1, int k2, INNERSPIRAL *is){
	if(k1<0 || k2<0 || (k1==0 && k2==0))
		return;
	
	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;
		fillPatch_4PentagonsLeft(k2-2, 1, is);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_4PentagonsLeft(0, k1-3, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_3PentagonsLeft(k1-2-i, 0, i, is);
			is->position--;
			is->code[is->position]-=i;
		}
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_4PentagonsLeft(k2-2, k1+1, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_3PentagonsLeft(k1-1-i, k2-1, 1+i, is);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		fillPatch_3PentagonsLeft(0, k2-2, k1, is);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_3PentagonsLeft(int k1, int k2, int k3, INNERSPIRAL *is){
	if(k1 < 0 || k2 < 0 || k3 < 0)
		return;
	int zeroes = 0;
	if(k1==0) zeroes++;
	if(k2==0) zeroes++;
	if(k3==0) zeroes++;
	if(zeroes>1) return;
	
	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;
		fillPatch_3PentagonsLeft(k2-1, k3-1, 1, is);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_3PentagonsLeft(0, k3-2, k1, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, is);
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k3==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_3PentagonsLeft(k2-2, k3, k1, is);
		is->code[is->position]-=k1+1;
	
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_2PentagonsLeft(k1-1-i, k2-1, k3, i, is);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		fillPatch_2PentagonsLeft(0, k2-2, k3, k1-1, is);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_3PentagonsLeft(k2-1, k3-1, k1+1, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, is);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		fillPatch_2PentagonsLeft(0, k2-1, k3-1, k1, is);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, INNERSPIRAL *is){
	if(k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
		return;
	if((k1 == 0 && k2==0) || (k2 == 0 && k3==0) || (k3 == 0 && k4==0) || (k4 == 0 && k1==0))
		return;
	
	if(k1==0){//TODO: this case is equal to the general case: look for more of these cases
		//place a hexagon
		is->code[is->position]+=1;
		fillPatch_2PentagonsLeft(k2-1, k3, k4-1, 1, is);
		is->code[is->position]-=1;

		//place a pentagon
		is->position++;
		is->code[is->position]=0;
		fillPatch_1PentagonLeft(0, k2-1, k3, k4-1, 0, is);
		is->position--;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_2PentagonsLeft(0, k3-1, k4-1, k1, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, is);
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k4==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_2PentagonsLeft(k2-1, k3-1, k4, k1, is);
		is->code[is->position]-=k1+1;

		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_1PentagonLeft(k1-1-i, k2, k3-1, k4, i, is);
			is->position--;
			is->code[is->position]-=i;
		}

		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		fillPatch_1PentagonLeft(0, k2-1, k3-1, k4, k1-1, is);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		fillPatch_2PentagonsLeft(k2-1, k3, k4-1, k1+1, is);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, is);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		fillPatch_1PentagonLeft(0, k2-1, k3, k4-1, k1, is);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, INNERSPIRAL *is){
}

void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, INNERSPIRAL *is){
	//check to see if the boundary is closed in the hexagonal lattice
	int x = 2*k1 + k2 - k3 - 2*k4 - k5 + k6;
	int y = k1 + 2*k2 + k3 - k4 - 2*k5 - k6;
	if(x==0 && y==0){
		//export structure
	}
}

