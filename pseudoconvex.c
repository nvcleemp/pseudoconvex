/*
 *  pseudoconvex.c
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#include "pseudoconvex.h"
#include "pseudoconvexuser.h"
#include "oldspiral2planar.h"

#include <stdio.h>
#include <stdlib.h>

/*========== DATA STRUCTURES ===========*/

INNERSPIRAL *getNewSpiral(int numberOfPentagons){
	INNERSPIRAL *is = (INNERSPIRAL *)malloc(sizeof(INNERSPIRAL));
	is->code = malloc(sizeof(int)*(numberOfPentagons+1)); //one extra for safety: fix later
	is->length = numberOfPentagons;
	is->position = 0;
	return is;
}

FRAGMENT *addNewFragment(FRAGMENT *currentFragment){
	if(currentFragment==NULL){
		FRAGMENT *fragment = (FRAGMENT *)malloc(sizeof(FRAGMENT));
		fragment->prev = fragment->next = NULL;
		fragment->isEnd = 0;
		fragment->isLayersFragment = 0;
		return fragment;
	} else {
		if(currentFragment->next==NULL){
			FRAGMENT *fragment = (FRAGMENT *)malloc(sizeof(FRAGMENT));
			fragment->next = NULL;
			fragment->prev = currentFragment;
			currentFragment->next = fragment;
			fragment->isEnd = 0;
			fragment->isLayersFragment = 0;
			return fragment;
		} else {
			return currentFragment->next;
		}
	}
}

FRAGMENT *createLayersFragment(FRAGMENT *currentFragment, int faces){
	FRAGMENT *fragment = addNewFragment(currentFragment);
	fragment->isLayersFragment = 1;
	fragment->faces = faces;
	fragment->endsWithPentagon = 0;
	return fragment;
}

void freeFragment(FRAGMENT *fragment){
	if(fragment==NULL){
		return;
	} else {
		freeFragment(fragment->next);
		free(fragment);
	}
}

SHELL *addNewShell(SHELL *currentShell, int size, FRAGMENT *start){
	if(currentShell==NULL){
		SHELL *shell = (SHELL *)malloc(sizeof(SHELL));
		shell->prev = shell->next = NULL;
		shell->size = size;
		shell->start = start;
		shell->isActive = 1;
		shell->nrOfPentagons = 0;
                shell->nonCyclicShell = 0;
		return shell;
	} else  if(currentShell->next==NULL){
		SHELL *shell = (SHELL *)malloc(sizeof(SHELL));
		shell->next = NULL;
		shell->prev = currentShell;
		currentShell->next = shell;
		shell->size = size;
		shell->start = start;
		shell->isActive = 1;
		shell->nrOfPentagons = 0;
                shell->nonCyclicShell = 0;
		return shell;
	} else {
		SHELL *shell = currentShell->next;
		shell->size = size;
		shell->start = start;
		shell->isActive = 1;
		shell->nrOfPentagons = 0;
                shell->nonCyclicShell = 0;
		return shell;
	}
}

void freeShell(SHELL *shell){
	if(shell==NULL){
		return;
	} else {
		freeShell(shell->next);
		free(shell);
	}
}

/*========== EXPORT ===========*/
void exportPlanarGraphCode(PATCH *patch){
    exportSpiralCode_impl(patch, 0);
}

void exportPlanarGraphTable(PATCH *patch ){
    exportSpiralCode_impl(patch, 1);
}

void exportInnerSpiral(PATCH *patch){
    INNERSPIRAL* is = patch->innerspiral;
    if(is->length==0){
        fprintf(stderr, "patch without pentagons\n");
        return;
    }
    int i;
    for(i=0; i<is->length-1; i++)
        fprintf(stderr, "%d, ", is->code[i]);
    fprintf(stderr, "%d\n", is->code[is->length-1]);
}

void exportExtendedInnerSpiral_impl(FRAGMENT* xis){
    if(xis->isLayersFragment)
        fprintf(stderr, "|%d|", xis->faces - (xis->endsWithPentagon ? 1 : 0));
    else
        fprintf(stderr, "(%d)", xis->faces - (xis->endsWithPentagon ? 1 : 0));
    if(xis->endsWithPentagon) fprintf(stderr, "(P) ");
    if(xis->next==NULL || xis->isEnd)
        fprintf(stderr, "\n");
    else
        exportExtendedInnerSpiral_impl(xis->next);
}

void exportExtendedInnerSpiral(PATCH *patch){
    exportExtendedInnerSpiral_impl(patch->firstFragment);
}

void exportShells(SHELL *shell){
	if(shell==NULL) return;
	
	SHELL *current = shell;
	while(current->prev != NULL)
		current = current->prev;
	
	while(current!=shell){
		if(current->start->isLayersFragment){
			fprintf(stderr, "[%d]", current->size);
		} else {
                    if(current->nonCyclicShell)
			fprintf(stderr, "[* %d:", current->size);
                    else
                        fprintf(stderr, "[%d:", current->size);
                    int faces = current->size;
                    FRAGMENT *fragment = current->start;
                    while(faces>0){
                            faces -= fragment->faces;
                            fprintf(stderr, "(%d)", fragment->faces - (fragment->endsWithPentagon ? 1 : 0));
                            if(fragment->endsWithPentagon) fprintf(stderr, "(P) ");
                            fragment = fragment->next;
                    }
                    if(faces<0) fprintf(stderr, "\n\nERROR\n\n");
                    fprintf(stderr, "]");
		}
		current = current->next;
	}
	//export last shell
        if(current->size>0){
            fprintf(stderr, "[%d:", current->size);
            int faces = current->size;
            FRAGMENT *fragment = current->start;
            while(faces>0 && fragment!=NULL && (fragment->prev==NULL || !fragment->prev->isEnd)){
                    faces -= fragment->faces;
                    fprintf(stderr, "(%d)", fragment->faces - (fragment->endsWithPentagon ? 1 : 0));
                    if(fragment->endsWithPentagon) fprintf(stderr, "(P) ");
                    fragment = fragment->next;
            }
            if(faces<0) fprintf(stderr, "\n\nERROR\n\n");

            fprintf(stderr, "]\n");
        } else {
            fprintf(stderr, "\n");
        }
}

/*========== CONSTRUCTION ==========*/

#define HEXFRAG(frag, size) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 0;
  
#define PENTFRAG(frag, size, shell) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 1; \
	(shell)->nrOfPentagons++;

boolean checkShellCanonicity(PATCH *patch, SHELL *shell, SHELL *nextShell, int nrOfBreakEdges, int* boundarySides){
	int i, j; //counter variables
	nextShell->nrOfBreakEdges = nrOfBreakEdges;

	//we start by calculating the breakEdge2FaceNumber for the next shell
	nextShell->breakEdge2FaceNumber[0] = 0;
	for(i=1;i<nrOfBreakEdges;i++){
		nextShell->breakEdge2FaceNumber[i] = nextShell->breakEdge2FaceNumber[i-1] + boundarySides[i-1];
	}

	//Then we handle some special cases in which we now that the shell is canonical
	if(shell->nrOfPentagons==0){
		//a shell without pentagons is always canonical
		nextShell->nrOfPossibleStartingPoints = shell->nrOfPossibleStartingPoints;
		DEBUGCONDITIONALMSG(nextShell->nrOfPossibleStartingPoints>nrOfBreakEdges, "Error in checkShellCanonicity: more starting points than break-edges")
		for(i=0; i<shell->nrOfPossibleStartingPoints; i++){
			nextShell->startingPoint2BreakEdge[i] = shell->startingPoint2BreakEdge[i];
			DEBUGCONDITIONALMSG(nextShell->startingPoint2BreakEdge[i]>nrOfBreakEdges, "Error in checkShellCanonicity: reference to non-existing break-edge")
			nextShell->startingPoint2FaceNumber[i] = 0;
			for(j=0; j<nextShell->startingPoint2BreakEdge[i]; j++){
				nextShell->startingPoint2FaceNumber[i] += boundarySides[j];
			}
		}
		nextShell->nrOfPossibleMirrorStartingPoints = shell->nrOfPossibleMirrorStartingPoints;
		DEBUGCONDITIONALMSG(nextShell->nrOfPossibleMirrorStartingPoints>nrOfBreakEdges, "Error in checkShellCanonicity: more mirror starting points than break-edges")
		for(i=0; i<shell->nrOfPossibleMirrorStartingPoints; i++){
			nextShell->startingPoint2BreakEdge[i] = shell->startingPoint2BreakEdge[i];
			DEBUGCONDITIONALMSG(nextShell->startingPoint2BreakEdge[i]>nrOfBreakEdges, "Error in checkShellCanonicity: reference to non-existing break-edge")
			nextShell->startingPoint2FaceNumber[i] = 0;
			for(j=0; j<nextShell->startingPoint2BreakEdge[i]; j++){
				nextShell->startingPoint2FaceNumber[i] += boundarySides[j];
			}
		}
		return 1;
	} else if(shell->nrOfPossibleStartingPoints + shell->nrOfPossibleMirrorStartingPoints == 0) {
		//a shell without alternate starting points is also canonical
		nextShell->nrOfPossibleStartingPoints = nextShell->nrOfPossibleMirrorStartingPoints = 0;
		//no further values need to be set
		return 1;
	}
	//in all other cases we need to check if the current shell is canonical
	
	//First construct the code corresponding with the current shell
	int code[shell->nrOfPentagons];
	FRAGMENT *frag = shell->start;
	for(i = 0; i<shell->nrOfPentagons; i++){
		code[i]=0;
		while(!frag->endsWithPentagon){
			code[i]+=frag->faces;
			frag = frag->next;
		}
		code[i]+=frag->faces - 1; //-1 because we don't count the pentagon
                frag = frag->next;
        }

        //a shell is canonical if the shell code is as large as possible
        //larger shell code means an early pentagon
        int shellCode[shell->size];

        for (i = 0; i < shell->size; i++) {
            shellCode[i] = 0;
        }

        int prevPentagon=0;
        for (i = 0; i < shell->nrOfPentagons; i++) {
            shellCode[code[i] + prevPentagon] = 1;
            prevPentagon += code[i] + 1; // +1 because we also count the pentagon
        }

	//Start with checking all alternate starting points in clockwise direction
	boolean newPossibleStartingPoints[shell->nrOfPossibleStartingPoints];
            //stores which starting points are still possible for the next shell
	int newNrOfStartingPoints = 0; //nr of starting points for the next shell
	int startAt = 0;
	for(i=0; i<shell->nrOfPossibleStartingPoints; i++){
            startAt = shell->startingPoint2FaceNumber[i];

            int j;
            for (j = 0; j < shell->size; j++) {
                if(shellCode[j]<shellCode[(j+startAt)%shell->size]){
                    //we found a starting points which gives a larger shell code
                    //i.e. the shell is not canonical
                    return 0;
                } else if(shellCode[j]>shellCode[(j+startAt)%shell->size]){
                    //we found a starting points which gives a smaller shell code
                    //i.e. we can disable this starting point for the next shell
                    //and jump out of the for
                    newPossibleStartingPoints[i]=0;
                    break;
                }
            }
            if(j==shell->size){
                //we found neither a smaller nor a larger code
                newPossibleStartingPoints[i]=1;
                newNrOfStartingPoints++;
            }
	}

	//Continue with checking all starting points in counterclockwise direction
	boolean newPossibleMirrorStartingPoints[shell->nrOfPossibleMirrorStartingPoints];
            //stores which starting points are still possible for the next shell
	int newNrOfMirrorStartingPoints = 0; //nr of starting points for the next shell
	startAt = 0;
	for(i=0; i<shell->nrOfPossibleMirrorStartingPoints; i++){
		startAt = shell->mirrorStartingPoint2FaceNumber[i];
            int j;
            for (j = 0; j < shell->size; j++) {
                if(shellCode[j]<shellCode[(-j + startAt + shell->size)%shell->size]){
                    //we found a starting points which gives a larger shell code
                    //i.e. the shell is not canonical
                    return 0;
                } else if(shellCode[j]>shellCode[(-j + startAt + shell->size)%shell->size]){
                    //we found a starting points which gives a smaller shell code
                    //i.e. we can disable this starting point for the next shell
                    //and jump out of the for
                    newPossibleMirrorStartingPoints[i]=0;
                    break;
                }
            }
            if(j==shell->size){
                //we found neither a smaller nor a larger code
                newPossibleMirrorStartingPoints[i]=1;
                newNrOfMirrorStartingPoints++;
            }
	}


	//if we reach this, we didn't find a smaller code, so the shell is canonical
	//so set the possible starting points for the next shell and return 1.

	nextShell->nrOfPossibleStartingPoints = newNrOfStartingPoints;
	nextShell->nrOfPossibleMirrorStartingPoints = newNrOfMirrorStartingPoints;
	//we first calculate the relation between the old break-edges and the new
	//breakedges based on the position of the pentagons in this shell and the
	//position of the break-edges.
	int currentPentagonPosition = code[0];
	int currentPentagonCounter = 1;
	int extraBreakEdges = 0;
	int oldBreakEdge2NewBreakEdge[shell->nrOfBreakEdges];
	oldBreakEdge2NewBreakEdge[0]=0;
	for(i=1; i<shell->nrOfBreakEdges; i++){
		while(currentPentagonPosition<shell->breakEdge2FaceNumber[i] &&
                        currentPentagonCounter<shell->nrOfPentagons){
                    extraBreakEdges++;
                    currentPentagonPosition = code[currentPentagonCounter++];
		}
		oldBreakEdge2NewBreakEdge[i]=i+extraBreakEdges;
	}
	
	//Next we store the new breakedges for the starting points that remain.
	j=0;
	for(i=0; i<shell->nrOfPossibleStartingPoints; i++){
		if(newPossibleStartingPoints[i]){
			nextShell->startingPoint2BreakEdge[j++] =
                                oldBreakEdge2NewBreakEdge[shell->startingPoint2BreakEdge[i]];
			DEBUGASSERT(j<newNrOfStartingPoints)
		}
	}
	j=0;
	for(i=0; i<shell->nrOfPossibleMirrorStartingPoints; i++){
		if(newPossibleMirrorStartingPoints[i]){
			nextShell->mirrorStartingPoint2BreakEdge[j++] =
                                oldBreakEdge2NewBreakEdge[shell->mirrorStartingPoint2BreakEdge[i]];
			DEBUGASSERT(j<newNrOfMirrorStartingPoints)
		}
	}

	//Finally we also calculate the maps startingPoint2FaceNumber and mirrorStartingPoint2FaceNumber
	for(i=0; i<shell->nrOfPossibleStartingPoints; i++){
		nextShell->startingPoint2FaceNumber[i] =
                        nextShell->breakEdge2FaceNumber[nextShell->startingPoint2BreakEdge[i]];
	}
	for(i=0; i<shell->nrOfPossibleMirrorStartingPoints; i++){
		nextShell->mirrorStartingPoint2FaceNumber[i] =
                        nextShell->breakEdge2FaceNumber[nextShell->mirrorStartingPoint2BreakEdge[i]];
	}
	return 1;
}

void fillPatch_5PentagonsLeft(int k, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k, "%d")
	DEBUGMSG("=======")
	if(k<=0)
		return;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k, current);
		int sides[1];
		sides[0]=k;
		if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 1, sides))
			return;
	}
	
	INNERSPIRAL *is = patch->innerspiral;

	//add a side of hexagons
	is->code[is->position]+=k;
	
	HEXFRAG(current, k)
	
	fillPatch_5PentagonsLeft(k-1, patch, addNewFragment(current), shellCounter-k, currentShell);
	is->code[is->position]-=k;
	
	//pentagon after i hexagons
	int i;
	for(i=0; i<k-1; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;
	
		PENTFRAG(current, i+1, currentShell)
	
		fillPatch_4PentagonsLeft(k-2-i, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=i;
	}

	//pentagon after k-1 hexagons
	is->code[is->position]+=k-1;
	is->position++;
	is->code[is->position]=0;
	
	PENTFRAG(current, k, currentShell)
		
	fillPatch_4PentagonsLeft(0, k-2, patch, addNewFragment(current), shellCounter-k, currentShell);
	currentShell->nrOfPentagons--;
	is->position--;
	is->code[is->position]-=k-1;
}

void fillPatch_4PentagonsLeft(int k1, int k2, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k1, "%d")
	DEBUGDUMP(k2, "%d")
	DEBUGMSG("=======")
	if(k1<0 || k2<0 || (k1==0 && k2==0))
		return;
	
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2, current);
		int sides[2];
		sides[0]=k1;
		sides[1]=k2;
		if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 2, sides))
			return;
	}
	
	INNERSPIRAL *is = patch->innerspiral;
	
	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;

		HEXFRAG(current, 1)

		fillPatch_4PentagonsLeft(k2-2, 1, patch, addNewFragment(current), shellCounter-1, currentShell);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_4PentagonsLeft(0, k1-3, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_3PentagonsLeft(k1-2-i, 0, i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_4PentagonsLeft(k2-2, k1+1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_3PentagonsLeft(k1-1-i, k2-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, currentShell)

		fillPatch_3PentagonsLeft(0, k2-2, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_3PentagonsLeft(int k1, int k2, int k3, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k1, "%d")
	DEBUGDUMP(k2, "%d")
	DEBUGDUMP(k3, "%d")
	DEBUGMSG("=======")
	if(k1 < 0 || k2 < 0 || k3 < 0)
		return;
	int zeroes = 0;
	if(k1==0) zeroes++;
	if(k2==0) zeroes++;
	if(k3==0) zeroes++;
	if(zeroes>1) return;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3, current);
		int sides[3];
		sides[0]=k1;
		sides[1]=k2;
		sides[2]=k3;
		if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 3, sides))
			return;
	}
	
	INNERSPIRAL *is = patch->innerspiral;

	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;

		HEXFRAG(current, 1)

		fillPatch_3PentagonsLeft(k2-1, k3-1, 1, patch, addNewFragment(current), shellCounter-1, currentShell);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(0, k3-2, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k3==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(k2-2, k3, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
	
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_2PentagonsLeft(k1-1-i, k2-1, k3, i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, currentShell)

		fillPatch_2PentagonsLeft(0, k2-2, k3, k1-1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(k2-1, k3-1, k1+1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, currentShell)

		fillPatch_2PentagonsLeft(0, k2-1, k3-1, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k1, "%d")
	DEBUGDUMP(k2, "%d")
	DEBUGDUMP(k3, "%d")
	DEBUGDUMP(k4, "%d")
	DEBUGMSG("=======")
	if(k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
            return;
	if((k1 == 0 && k2==0) || (k2 == 0 && k3==0) || (k3 == 0 && k4==0) || (k4 == 0 && k1==0))
            return;
		
	//shell handling
	if(shellCounter==0){
            if((k1 == 0 && k3 == 0 && k2 == k4) || (k2 == 0 && k4 == 0 && k1 == k3)){
                //the size is either k1 + 1 == k3 + 1 or k2 + 1 == k4 + 1
		currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + 1, current);
                currentShell->nonCyclicShell = 1;
            } else {
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4, current);
            }
            int sides[4];
            sides[0]=k1;
            sides[1]=k2;
            sides[2]=k3;
            sides[3]=k4;
            if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 4, sides))
                    return;
	}
	
	INNERSPIRAL *is = patch->innerspiral;
	
	if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(0, k3-1, k4-1, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k4==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(k2-1, k3-1, k4, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;

		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_1PentagonLeft(k1-1-i, k2, k3-1, k4, i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}

		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, currentShell)

		fillPatch_1PentagonLeft(0, k2-1, k3-1, k4, k1-1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(k2-1, k3, k4-1, k1+1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, currentShell)

			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, currentShell)

		fillPatch_1PentagonLeft(0, k2-1, k3, k4-1, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k1, "%d")
	DEBUGDUMP(k2, "%d")
	DEBUGDUMP(k3, "%d")
	DEBUGDUMP(k4, "%d")
	DEBUGDUMP(k5, "%d")
	DEBUGMSG("=======")
	if(k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
            return;
		
	//shell handling
	if(shellCounter==0){
            if((k1 == k4 && k2 + k3 + k5==0) ||
                    (k2 == k5 && k1 + k3 + k4 == 0) ||
                    (k3 == k1 && k2 + k4 + k5 == 0) ||
                    (k4 == k2 && k1 + k3 + k5 == 0) ||
                    (k5 == k3 && k1 + k2 + k4 == 0)){
                //in this case the shell is no longer cyclic
                currentShell = addNewShell(currentShell, shellCounter = (k1+k2+k3+k4+k5)/2+1, current);
                currentShell->nonCyclicShell = 1;
            } else {
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4+k5, current);
            }
            int sides[5];
            sides[0]=k1;
            sides[1]=k2;
            sides[2]=k3;
            sides[3]=k4;
            sides[4]=k5;
            if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 5, sides))
                return;
	}
	
	INNERSPIRAL *is = patch->innerspiral;
	
	if(k1==0 && k2==0 && k3==0 && k4==0 && k5==0){
		PENTFRAG(current, 1, currentShell)
		if(currentShell->size == 0) currentShell->size = 1;
		fillPatch_0PentagonsLeft(0, 0, 0, 0, 0, 0, patch, addNewFragment(current), shellCounter-1, currentShell);
		currentShell->nrOfPentagons--;
	} else if(k1==0 && k2==0){
		//only one possible filling in case the following is true
		if(k3==k5 && k4==0){
			is->code[is->position]+=1;
			HEXFRAG(current, 1)
			fillPatch_1PentagonLeft(0, k3-1, 0, k5-1, 0, patch, addNewFragment(current), shellCounter-1, currentShell);
			is->code[is->position]-=1;
		}
	} else if(k1==0 && k5==0){
		//only one possible filling in case the following is true
		if(k2==k4 && k3==0){
			is->code[is->position]+=1;
			HEXFRAG(current, 1)
			fillPatch_1PentagonLeft(k2-1, 0, k4-1, 0, 0, patch, addNewFragment(current), shellCounter-1, currentShell);
			is->code[is->position]-=1;
		}
	} else if(k4==0 && k5==0){
		//only one possible filling in case the following is true
		if(k1==k3 && k2==0){
                    if(k1>1 && shellCounter==1){
			is->code[is->position]+=1;
                        HEXFRAG(current, 1)
                        fillPatch_1PentagonLeft(k1-1, 0, k1-1, 0, 0, patch, addNewFragment(current), 0, currentShell);
                        is->code[is->position]-=1;
                    } else {
			is->code[is->position]+=k1;
			PENTFRAG(current, k1+1, currentShell)
			//TODO: make sure that shells are closed at this point
			if(validateStructure(patch)){
				current->isEnd = 1;
				processStructure(patch, currentShell);
				current->isEnd = 0;
			}			
			currentShell->nrOfPentagons--;
			is->code[is->position]-=k1;
                    }
		}
	} else if(k3==0 && k4==0){
		//only one possible filling in case the following is true
		if(k1==0 && k2==k5){
                    if(k2>1 && shellCounter==1){
			//is->code[is->position]+=0;
                        PENTFRAG(current, 1, currentShell)
                        fillPatch_0PentagonsLeft(k2-1, 0, 0, k2-1, 0, 0, patch, addNewFragment(current), 0, currentShell);
                        //is->code[is->position]-=0;
                    } else {
			PENTFRAG(current, 1, currentShell)
			//TODO: make sure that shells are closed at this point
			if(validateStructure(patch)){
				current->isEnd = 1;
				processStructure(patch, currentShell);
				current->isEnd = 0;
			}
			currentShell->nrOfPentagons--;
                    }
		}
	} else if(k2==0 && k3==0){
		//only one possible filling in case the following is true
		if(k5==0 && k1==k4){
                    if(k1>1 && shellCounter==1){
			//is->code[is->position]+=0;
                        PENTFRAG(current, 1, currentShell)
                        fillPatch_0PentagonsLeft(k1-1, 0, 0, k1-1, 0, 0, patch, addNewFragment(current), 0, currentShell);
                        //is->code[is->position]-=0;
                    } else {
			PENTFRAG(current, 1, currentShell)
                        //TODO: make sure that shells are closed at this point
			if(validateStructure(patch)){
				current->isEnd = 1;
				processStructure(patch, currentShell);
				current->isEnd = 0;
			}
			currentShell->nrOfPentagons--;
                    }
		}
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(0, k3-1, k4, k5-1, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, currentShell)
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4, k5-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k5==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(k2-1, k3, k4-1, k5, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;

		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, currentShell)
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4-1, k5, i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}

		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		PENTFRAG(current, k1+1, currentShell)
		fillPatch_0PentagonsLeft(0, k2-1, k3, k4-1, k5, k1-1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(k2-1, k3, k4, k5-1, k1+1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, currentShell)
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4, k5-1, 1+i, patch, addNewFragment(current), shellCounter-i-1, currentShell);
			currentShell->nrOfPentagons--;
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		
		PENTFRAG(current, k1+1, currentShell)
		
		fillPatch_0PentagonsLeft(0, k2-1, k3, k4, k5-1, k1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
		currentShell->nrOfPentagons--;
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell){
	DEBUGDUMP(k1, "%d")
	DEBUGDUMP(k2, "%d")
	DEBUGDUMP(k3, "%d")
	DEBUGDUMP(k4, "%d")
	DEBUGDUMP(k5, "%d")
	DEBUGDUMP(k6, "%d")
	DEBUGMSG("=======")
	//check to see if the boundary is closed in the hexagonal lattice
	int x = 2*k1 + k2 - k3 - 2*k4 - k5 + k6;
	int y = k1 + 2*k2 + k3 - k4 - 2*k5 - k6;
		
	if(x==0 && y==0){
            //shell handling
            if(shellCounter==0){
                if(!(k1 + k2 + k3 + k4 + k5 + k6 == 0) &&
                        ((k1 == k4 && k2 + k3 + k5 + k6==0) ||
                        (k2 == k5 && k1 + k3 + k4 + k6 == 0) ||
                        (k3 == k6 && k1 + k2 + k4 + k5 == 0) ||
                        (k4 == k1 && k2 + k3 + k5 + k6 == 0) ||
                        (k5 == k2 && k1 + k3 + k4 + k6 == 0) ||
                        (k6 == k3 && k1 + k2 + k4 + k5 == 0))){
                    //in this case the shell is no longer cyclic
                    currentShell = addNewShell(currentShell, shellCounter = (k1+k2+k3+k4+k5+k6)/2+1, current);
                    currentShell->nonCyclicShell = 1;
                } else {
                    currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4+k5+k6, current);
                }
                int sides[6];
                sides[0]=k1;
                sides[1]=k2;
                sides[2]=k3;
                sides[3]=k4;
                sides[4]=k5;
                sides[5]=k6;
                if(!checkShellCanonicity(patch, currentShell->prev, currentShell, 6, sides))
                    return;
		if(validateStructure(patch)){
			current->prev->isEnd = 1;
			processStructure(patch, currentShell);
			current->prev->isEnd = 0;
		}
            } else {
                //complete shell
                HEXFRAG(current, k1+1)
                //no need to add something to the code: where already past the last pentagon
		fillPatch_0PentagonsLeft(k2-1, k3, k4, k5, k6-1, k1 + 1, patch, addNewFragment(current), shellCounter-k1-1, currentShell);
            }
	
        }

}

