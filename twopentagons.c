/*
 *  twopentagons.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#include "twopentagons.h"
#include "cone.h"
#include "util.h"
#include "pseudoconvex.h"
#include "pseudoconvexuser.h"

#include <stdlib.h>

/* int getTwoPentagonsPatch(int sside, int symmetric, int mirror) */
/*
	generates the canonical cone patches with two pentagons and the given boundary
*/
void getTwoPentagonsCones(int sside, boolean symmetric, boolean mirror, INNERSPIRAL *is){
	if(is->length!=2) exit(1);

	int i;

	if(symmetric){
		//sside patches with spiral code i, 2*sside (i=0,...,sside-1)
		int upperbound = sside;
		if(!mirror){
			upperbound = HALFFLOOR(sside) + 1;
		}
		is->code[1] = 2*sside;
		for(i=0;i<upperbound;i++){
			processStructure(is);
			is->code[0]++;
		}
	} else {
		//sside patches with spiral code i, 2*sside + 1 (i=0,...,sside-1)
		int lowerbound = sside+1;
		int upperbound = 3*sside+1;
		if(!mirror){
			lowerbound += HALFFLOOR(sside+1);
			upperbound -= HALFFLOOR(sside);
		}
		is->code[1] = 2*sside + 1;
		for(i=lowerbound;i<=upperbound;i++){
			processStructure(is);
			is->code[0]++;
		}
	}
}


int getTwoPentagonsConesCount(int sside, boolean symmetric, boolean mirror){
	if(symmetric){
		if(mirror){
			return sside;
		} else {
			return HALFFLOOR(sside) + 1;
		}
	} else {
		int lowerbound = sside+1;
		int upperbound = 3*sside+1;
		if(!mirror){
			lowerbound += HALFFLOOR(sside+1);
			upperbound -= HALFFLOOR(sside);
		}
		return upperbound - lowerbound + 1;
	}
}
