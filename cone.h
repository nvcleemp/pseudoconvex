/*
 *  cone.h
 *  
 *
 *  Created by Nico Van Cleemput on 19/05/09.
 *
 */

#ifndef _CONE_H //if not defined
#define _CONE_H

#include "pseudoconvex.h"
#include "util.h"

void start5PentagonsCone(int sside, boolean mirror, INNERSPIRAL *is);
void start4PentagonsCone(int sside, int symmetric, boolean mirror, INNERSPIRAL *is);
void start3PentagonsCone(int sside, int symmetric, boolean mirror, INNERSPIRAL *is);

#endif // end if not defined, and end the header file
