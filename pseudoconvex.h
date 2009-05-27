/*
 *  pseudoconvex.h
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#ifndef _PSEUDOCONVEX_H //if not defined
#define _PSEUDOCONVEX_H

#include "util.h"

/*========== DATA STRUCTURES ===========*/

struct _innerspiral {
	int length;     /* the length of the code (i.e. the number of pentagons in the patch) */
	int position;   /* the current possition in the spiral code */
	int *code;      /* the array containing the actual code */

};

typedef struct _innerspiral INNERSPIRAL;

/* data structure for an extended inner spiral fragment (implemented as a doubly-linked-list) */
struct frag {
	int faces;                     /* The number of faces in this fragment */
	boolean endsWithPentagon;      /* Is the last face in this fragment a pentagon */
	boolean pentagonAtBreakEdge;   /* lies that pentagon at a break-edge */
	
	struct frag *next;             /* The next fragment in the spiral */
	struct frag *prev;             /* The previous fragment in the spiral */
};

typedef struct frag FRAGMENT;

INNERSPIRAL *getNewSpiral(int numberOfPentagons);
FRAGMENT *addNewFragment(FRAGMENT *currentFragment);
void freeFragment(FRAGMENT *fragment);

/*========== EXPORT ===========*/
void exportPlanarGraphCode(INNERSPIRAL *is);
void exportPlanarGraphTable(INNERSPIRAL *is);
void exportInnerSpiral(INNERSPIRAL *is);


/*========== CONSTRUCTION ==========*/
void fillPatch_5PentagonsLeft(int k, INNERSPIRAL *is);
void fillPatch_4PentagonsLeft(int k1, int k2, INNERSPIRAL *is);
void fillPatch_3PentagonsLeft(int k1, int k2, int k3, INNERSPIRAL *is);
void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, INNERSPIRAL *is);
void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, INNERSPIRAL *is);
void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, INNERSPIRAL *is);

#endif // end if not defined, and end the header file
