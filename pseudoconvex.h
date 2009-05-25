/*
 *  pseudoconvex.h
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#ifndef _PSEUDOCONVEX_H //if not defined
#define _PSEUDOCONVEX_H

/*========== DATA STRUCTURES ===========*/

struct _innerspiral {
	int length;     /* the length of the code (i.e. the number of pentagons in the patch) */
	int position;   /* the current possition in the spiral code */
	int *code;      /* the array containing the actual code */

};

typedef struct _innerspiral INNERSPIRAL;

INNERSPIRAL *getNewSpiral(int numberOfPentagons);

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
