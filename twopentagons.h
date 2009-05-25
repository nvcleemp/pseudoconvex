/*
 *  twopentagons.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _TWOPENTAGONS_H //if not defined
#define _TWOPENTAGONS_H

#include "cone.h"
#include "pseudoconvex.h"

void getTwoPentagonsCones(int sside, boolean symmetric, boolean mirror, INNERSPIRAL *is);
int getTwoPentagonsConesCount(int sside, boolean symmetric, boolean mirror);

#endif // end if not defined, and end the header file
