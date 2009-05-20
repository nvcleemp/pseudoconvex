/*
 *  pseudoconvex.h
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

/*========== DATA STRUCTURES ===========*/

struct _innerspiral {
	int length;     /* the length of the code (i.e. the number of pentagons in the patch) */
	int position;   /* the current possition in the spiral code */
	int *code;      /* the array containing the actual code */

};

typedef struct _innerspiral INNERSPIRAL;

/*========== EXPORT ===========*/
void exportPlanarGraphCode();
void exportPlanarGraphTable();
void exportInnerSpiral(INNERSPIRAL *is);


/*========== CONSTRUCTION ==========*/
void fillPatch_5PentagonsLeft(int k, INNERSPIRAL *is);
void fillPatch_4PentagonsLeft(int k1, int k2, INNERSPIRAL *is);
void fillPatch_3PentagonsLeft(int k1, int k2, int k3, INNERSPIRAL *is);
void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, INNERSPIRAL *is);
void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, INNERSPIRAL *is);
void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, INNERSPIRAL *is);
