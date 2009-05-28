/*
 *  pseudoconvexuser.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _PSEUDOCONVEXUSER_H //if not defined
#define _PSEUDOCONVEXUSER_H

#include "util.h"

/* This method gets called each time a completed structure is found and right before processStructure is called.
 */
boolean validateStructure(INNERSPIRAL *is);

/* This method gets called each time a valid structure is found.
 */
void processStructure(INNERSPIRAL *is, FRAGMENT *xis);

#endif // end if not defined, and end the header file
