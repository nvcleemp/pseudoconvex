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
	
	boolean isEnd;
	boolean isLayersFragment;
	
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
	
	int nrOfBreakEdges;
	int nrOfPossibleStartingPoints; //the number of extra possible starting points in clockwise direction

	//Usually there will be very few shells per case, but they will be reused quite often.
	//Therefore we already fix the following arrays to there maximum length. Otherwise they
	//will often need to be freed and reallocated.

	//there are at most five break-edges in a pseudoconvex patch
	 //we store the number of the face in the shell
	int breakEdge2FaceNumber[5];
	
	//there are at most five break-edges, so at most 4 extra starting points
	int startingPoint2BreakEdge[4];
	int startingPoint2FaceNumber[4]; //we store the number of the face in the shell
	
	int nrOfPossibleMirrorStartingPoints;
	
	//there are at most five break-edges, so at most 5 mirror starting points
	//these starting points are stilled stored in clockwise order
	int mirrorStartingPoint2BreakEdge[5];
	int mirrorStartingPoint2FaceNumber[5];
	
	int nrOfPentagons; //the number of pentagons that belong to this shell
};

typedef struct _shell SHELL;

/* data structure for a pseudoconvex patch */

struct _patch {
	int numberOfPentagons;
	int *boundary;
	INNERSPIRAL *innerspiral;
	FRAGMENT *firstFragment;
	SHELL *outershell;
};

typedef struct _patch PATCH;

INNERSPIRAL *getNewSpiral(int numberOfPentagons);
FRAGMENT *addNewFragment(FRAGMENT *currentFragment);
FRAGMENT *createLayersFragment(FRAGMENT *currentFragment, int faces);
void freeFragment(FRAGMENT *fragment);
SHELL *addNewShell(SHELL *currentShell, int size, FRAGMENT *start);
void freeShell(SHELL *shell);

/*========== EXPORT ===========*/
void exportPlanarGraphCode(INNERSPIRAL *is);
void exportPlanarGraphTable(INNERSPIRAL *is);
void exportInnerSpiral(INNERSPIRAL *is);
void exportExtendedInnerSpiral(FRAGMENT *xis);
void exportShells(SHELL *shell);

/*========== CANONICITY ============*/
/**
 * Check whether this shell is canonical. Returns 0 when this shell is not canonical
 * and 1 if it is. If the shell is canonical, the possible startpoints for the next
 * shell are set.
 */
boolean checkShellCanonicity(PATCH *patch, SHELL *shell, SHELL *nextShell, int nrOfBreakEdges, int* boundarySides);

/*========== CONSTRUCTION ==========*/
void fillPatch_5PentagonsLeft(int k, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);
void fillPatch_4PentagonsLeft(int k1, int k2, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);
void fillPatch_3PentagonsLeft(int k1, int k2, int k3, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);
void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);
void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);
void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell);

#endif // end if not defined, and end the header file
