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
	boolean pentagonAtBreakEdge;   /* lies that pentagon at a break-edge from the previous shell */
	
	boolean isEnd;
	
	struct frag *next;             /* The next fragment in the spiral */
	struct frag *prev;             /* The previous fragment in the spiral */
};

typedef struct frag FRAGMENT;

/* data structure for a shell (implemented as a doubly-linked-list) */
struct _shell {
	int size;
	FRAGMENT *start;
	
	boolean isActive;
	
	struct _shell *next;
	struct _shell *prev;
};

typedef struct _shell SHELL;

INNERSPIRAL *getNewSpiral(int numberOfPentagons);
FRAGMENT *addNewFragment(FRAGMENT *currentFragment);
void freeFragment(FRAGMENT *fragment);
SHELL *addNewShell(SHELL *currentShell, int size, FRAGMENT *start);
void freeShell(SHELL *shell);

/*========== EXPORT ===========*/
void exportPlanarGraphCode(INNERSPIRAL *is);
void exportPlanarGraphTable(INNERSPIRAL *is);
void exportInnerSpiral(INNERSPIRAL *is);
void exportExtendedInnerSpiral(FRAGMENT *xis);
void exportShells(SHELL *shell);

/*========== CONSTRUCTION ==========*/
void fillPatch_5PentagonsLeft(int k, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);
void fillPatch_4PentagonsLeft(int k1, int k2, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);
void fillPatch_3PentagonsLeft(int k1, int k2, int k3, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);
void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);
void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);
void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, INNERSPIRAL *is, FRAGMENT *current, FRAGMENT *start, int shellCounter, SHELL *currentShell);

#endif // end if not defined, and end the header file
