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
	is->length = numberOfPentagons;
	is->position = 0;
	return is;
}

FRAGMENT *addNewFragment(FRAGMENT *currentFragment){
	if(currentFragment==NULL){
		FRAGMENT *fragment = (FRAGMENT *)malloc(sizeof(FRAGMENT));
		fragment->prev = fragment->next = NULL;
		fragment->isEnd = 0;
		return fragment;
	} else {
		if(currentFragment->next==NULL){
			FRAGMENT *fragment = (FRAGMENT *)malloc(sizeof(FRAGMENT));
			fragment->next = NULL;
			fragment->prev = currentFragment;
			currentFragment->next = fragment;
			fragment->isEnd = 0;
			return fragment;
		} else {
			return currentFragment->next;
		}
	}
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
		return shell;
	} else  if(currentShell->next==NULL){
		SHELL *shell = (SHELL *)malloc(sizeof(SHELL));
		shell->next = NULL;
		shell->prev = currentShell;
		currentShell->next = shell;
		shell->size = size;
		shell->start = start;
		shell->isActive = 1;
		return shell;
	} else {
		SHELL *shell = currentShell->next;
		shell->size = size;
		shell->start = start;
		shell->isActive = 1;
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

void exportExtendedInnerSpiral(FRAGMENT *xis){
	fprintf(stderr, "(%d)", xis->faces - (xis->endsWithPentagon ? 1 : 0));
	if(xis->endsWithPentagon) fprintf(stderr, "(P) ");
	if(xis->next==NULL || xis->isEnd)
		fprintf(stderr, "\n");
	else
		exportExtendedInnerSpiral(xis->next);
}

void exportShells(SHELL *shell){
	if(shell==NULL) return;
	
	SHELL *current = shell;
	while(current->prev != NULL)
		current = current->prev;
	
	while(current!=shell){
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
		current = current->next;
	}
	//export last shell
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
}

/*========== CONSTRUCTION ==========*/

#define HEXFRAG(frag, size) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 0; \
	(frag)->pentagonAtBreakEdge = 0;
  
#define PENTFRAG(frag, size, atBreakEdge) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 1; \
	(frag)->pentagonAtBreakEdge = (atBreakEdge);
  
void fillPatch_5PentagonsLeft(int k, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d\n", k);
#endif
	if(k<=0)
		return;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k, current);
	}
	
	//add a side of hexagons
	is->code[is->position]+=k;
	
	HEXFRAG(current, k)
	
	fillPatch_5PentagonsLeft(k-1, is, addNewFragment(current), start, shellCounter-k, currentShell);
	is->code[is->position]-=k;
	
	//pentagon after i hexagons
	int i;
	for(i=0; i<k-1; i++){
		is->code[is->position]+=i;
		is->position++;
		is->code[is->position]=0;
	
		PENTFRAG(current, i+1, i==0)
	
		fillPatch_4PentagonsLeft(k-2-i, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
		is->position--;
		is->code[is->position]-=i;
	}

	//pentagon after k-1 hexagons
	is->code[is->position]+=k-1;
	is->position++;
	is->code[is->position]=0;
	
	PENTFRAG(current, k, 0)
		
	fillPatch_4PentagonsLeft(0, k-2, is, addNewFragment(current), start, shellCounter-k, currentShell);
	is->position--;
	is->code[is->position]-=k-1;
}

void fillPatch_4PentagonsLeft(int k1, int k2, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d, %d\n", k1, k2);
#endif
	if(k1<0 || k2<0 || (k1==0 && k2==0))
		return;
	
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2, current);
	}
	
	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;

		HEXFRAG(current, 1)

		fillPatch_4PentagonsLeft(k2-2, 1, is, addNewFragment(current), start, shellCounter-1, currentShell);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_4PentagonsLeft(0, k1-3, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, 0)//TODO: at break edge

			fillPatch_3PentagonsLeft(k1-2-i, 0, i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_4PentagonsLeft(k2-2, k1+1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_3PentagonsLeft(k1-1-i, k2-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, 0)//TODO: at break edge

		fillPatch_3PentagonsLeft(0, k2-2, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_3PentagonsLeft(int k1, int k2, int k3, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d, %d, %d\n", k1, k2, k3);
#endif
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
	}
	
	if(k1==0){
		//only possibility: place a hexagon
		is->code[is->position]+=1;

		HEXFRAG(current, 1)

		fillPatch_3PentagonsLeft(k2-1, k3-1, 1, is, addNewFragment(current), start, shellCounter-1, currentShell);
		is->code[is->position]-=1;
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(0, k3-2, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1-1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k3==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(k2-2, k3, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
	
		//pentagon after i hexagons
		int i;
		for(i=1; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, 0)//TODO: at break edge

			fillPatch_2PentagonsLeft(k1-1-i, k2-1, k3, i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, 0)//TODO: at break edge

		fillPatch_2PentagonsLeft(0, k2-2, k3, k1-1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)

		fillPatch_3PentagonsLeft(k2-1, k3-1, k1+1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_2PentagonsLeft(k1-1-i, k2, k3-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, 0)//TODO: at break edge

		fillPatch_2PentagonsLeft(0, k2-1, k3-1, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d, %d, %d, %d\n", k1, k2, k3, k4);
#endif
	if(k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
		return;
	if((k1 == 0 && k2==0) || (k2 == 0 && k3==0) || (k3 == 0 && k4==0) || (k4 == 0 && k1==0))
		return;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4, current);
	}
	
	if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(0, k3-1, k4-1, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k4==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(k2-1, k3-1, k4, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;

		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_1PentagonLeft(k1-1-i, k2, k3-1, k4, i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}

		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, 0)//TODO: at break edge

		fillPatch_1PentagonLeft(0, k2-1, k3-1, k4, k1-1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;

		HEXFRAG(current, k1+1)

		fillPatch_2PentagonsLeft(k2-1, k3, k4-1, k1+1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;

			PENTFRAG(current, i+1, i==0)//TODO: at break edge

			fillPatch_1PentagonLeft(k1-1-i, k2, k3, k4-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;

		PENTFRAG(current, k1+1, 0)//TODO: at break edge

		fillPatch_1PentagonLeft(0, k2-1, k3, k4-1, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d, %d, %d, %d, %d\n", k1, k2, k3, k4, k5);
#endif
	if(k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
		return;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4+k5, current);
	}
	
	if(k1==0 && k2==0 && k3==0 && k4==0 && k5==0){
		//TODO: immediately process structure here
		PENTFRAG(current, 1, 0)//TODO: at break edge
		if(currentShell->size == 0) currentShell->size = 1;
		fillPatch_0PentagonsLeft(0, 0, 0, 0, 0, 0, is, addNewFragment(current), start, shellCounter-1, currentShell);
	} else if(k1==0 && k2==0){
		//only one possible filling in case the following is true
		if(k3==k5 && k4==0){
			is->code[is->position]+=k3;
			PENTFRAG(current, k3+1, 0)//TODO: at break edge
			if(validateStructure(is)){
				current->isEnd = 1;
				processStructure(is, start, currentShell);
				current->isEnd = 0;
			}			
			is->code[is->position]-=k3;
		}
	} else if(k1==0 && k5==0){
		//only one possible filling in case the following is true
		if(k2==k4 && k3==0){
			is->code[is->position]+=k2;
			PENTFRAG(current, k2+1, 0)//TODO: at break edge
			if(validateStructure(is)){
				current->isEnd = 1;
				processStructure(is, start, currentShell);
				current->isEnd = 0;
			}			
			is->code[is->position]-=k2;
		}
	} else if(k4==0 && k5==0){
		//only one possible filling in case the following is true
		if(k1==k3 && k2==0){
			is->code[is->position]+=k1;
			PENTFRAG(current, k1+1, 0)//TODO: at break edge
			if(validateStructure(is)){
				current->isEnd = 1;
				processStructure(is, start, currentShell);
				current->isEnd = 0;
			}			
			is->code[is->position]-=k1;
		}
	} else if(k3==0 && k4==0){
		//only one possible filling in case the following is true
		if(k1==0 && k2==k5){
			PENTFRAG(current, 1, 0)//TODO: at break edge
			if(validateStructure(is)){
				current->isEnd = 1;
				processStructure(is, start, currentShell);
				current->isEnd = 0;
			}
		}
	} else if(k2==0 && k3==0){
		//only one possible filling in case the following is true
		if(k5==0 && k1==k4){
			PENTFRAG(current, 1, 0)//TODO: at break edge
			if(validateStructure(is)){
				current->isEnd = 1;
				processStructure(is, start, currentShell);
				current->isEnd = 0;
			}
		}
	} else if(k2==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(0, k3-1, k4, k5-1, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, i==0)//TODO: at break edge
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4, k5-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
	} else if(k5==0){
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(k2-1, k3, k4-1, k5, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;

		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, i==0)//TODO: at break edge
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4-1, k5, i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}

		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		PENTFRAG(current, k1+1, 0)//TODO: at break edge
		fillPatch_0PentagonsLeft(0, k2-1, k3, k4-1, k5, k1-1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	} else {
		//add a side of hexagons
		is->code[is->position]+=k1+1;
		
		HEXFRAG(current, k1+1)
		
		fillPatch_1PentagonLeft(k2-1, k3, k4, k5-1, k1+1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->code[is->position]-=k1+1;
		
		//pentagon after i hexagons
		int i;
		for(i=0; i<k1; i++){
			is->code[is->position]+=i;
			is->position++;
			is->code[is->position]=0;
			
			PENTFRAG(current, i+1, i==0)//TODO: at break edge
			
			fillPatch_0PentagonsLeft(k1-1-i, k2, k3, k4, k5-1, 1+i, is, addNewFragment(current), start, shellCounter-i-1, currentShell);
			is->position--;
			is->code[is->position]-=i;
		}
		
		//pentagon after k1 hexagons
		is->code[is->position]+=k1;
		is->position++;
		is->code[is->position]=0;
		
		PENTFRAG(current, k1+1, 0)//TODO: at break edge
		
		fillPatch_0PentagonsLeft(0, k2-1, k3, k4, k5-1, k1, is, addNewFragment(current), start, shellCounter-k1-1, currentShell);
		is->position--;
		is->code[is->position]-=k1;
	}
}

void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell){
#ifdef _DEBUG
	fprintf(stderr, "%d, %d, %d, %d, %d, %d\n", k1, k2, k3, k4, k5, k6);
#endif
	//check to see if the boundary is closed in the hexagonal lattice
	int x = 2*k1 + k2 - k3 - 2*k4 - k5 + k6;
	int y = k1 + 2*k2 + k3 - k4 - 2*k5 - k6;
		
	//shell handling
	if(shellCounter==0){
		currentShell = addNewShell(currentShell, shellCounter = k1+k2+k3+k4+k5+k6, current);
	}
	
	if(x==0 && y==0){
		if(validateStructure(is)){
			current->prev->isEnd = 1;
			processStructure(is, start, currentShell);
			current->prev->isEnd = 0;
		}		
	}
}

