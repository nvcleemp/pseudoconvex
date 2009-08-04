/*
 *  util.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _UTIL_H //if not defined
#define _UTIL_H

#define HALFFLOOR(n) (n%2==0 ? n/2 : (n-1)/2)

//define boolean
typedef int boolean;

#define DEBUGMSG(msg) fprintf(stderr, "%s:%u %s\n", __FILE__, __LINE__, msg);

#define DEBUGDUMP(var, format) fprintf(stderr, "%s:%u %s=" format "\n", __FILE__, __LINE__, #var, var);

#define DEBUGASSERT(assertion) if(!(assertion)) fprintf(stderr, "%s:%u Assertion failed: %s\n", __FILE__, __LINE__, #assertion);

#endif // end if not defined, and end the header file
