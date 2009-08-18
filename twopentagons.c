/*
 *  twopentagons.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#include "twopentagons.h"
#include "cone.h"
#include "util.h"
#include "pseudoconvex.h"
#include "pseudoconvexuser.h"

#include <stdlib.h>

/* int getTwoPentagonsPatch(int sside, int symmetric, int mirror) */

/*
        generates the canonical cone patches with two pentagons and the given boundary
 */
void getTwoPentagonsCones(PATCH *patch, int sside, boolean symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;
    if (is->length != 2) exit(1);

    int i;

    if (symmetric) {
        FRAGMENT *fragmentWithFirstPentagon = addNewFragment(currentFragment);
        fragmentWithFirstPentagon->endsWithPentagon = 1;
        FRAGMENT *fragmentAfterFirstPentagon = addNewFragment(fragmentWithFirstPentagon);
        fragmentAfterFirstPentagon->endsWithPentagon = 0;
        FRAGMENT *secondSide = addNewFragment(fragmentAfterFirstPentagon);
        secondSide->faces = sside;
        FRAGMENT *fragment4 = addNewFragment(secondSide);
        FRAGMENT *fragment5 = addNewFragment(fragment4);
        fragment5->endsWithPentagon = 0;
        FRAGMENT *fragment6;
        currentShell = addNewShell(currentShell, 4 * sside, fragmentWithFirstPentagon);

        //sside patches with spiral code i, 2*sside (i=0,...,sside-1)
        int upperbound = sside;
        if (!mirror) {
            upperbound = HALFFLOOR(sside) + 1;
        }
        is->code[1] = 2 * sside;
        for (i = 0; i < upperbound; i++) {
            fragmentWithFirstPentagon->faces = i + 1;
            fragmentAfterFirstPentagon->faces = sside - i;
            if (i == 0) {
                secondSide->endsWithPentagon = 1;
                fragment4->endsWithPentagon = 0;
                fragment4->faces = sside;
                fragment5->faces = sside - 1;
            } else if (i == 1) {
                /*
                 * This is necessary because in case i==0 there is one fragment less
                 * so when i==1 we need to add an extra fragment, and from that moment
                 * on everything stays the same.
                 */
                secondSide->endsWithPentagon = 0;
                fragment4->endsWithPentagon = 1;
                fragment6 = addNewFragment(fragment5);
                fragment6->faces = sside - 1;
                fragment6->endsWithPentagon = 0;

                fragment4->faces = 1;
                fragment5->faces = sside - 1;
            } else {
                fragment4->faces = i;
                fragment5->faces = sside - i;
            }
            processStructure(patch, currentShell);
            is->code[0]++;
        }
    } else {
        FRAGMENT *fragment1 = addNewFragment(currentFragment);
        fragment1->faces = sside;
        fragment1->endsWithPentagon = 0;
        FRAGMENT *fragment2 = addNewFragment(fragment1);
        FRAGMENT *fragment3 = addNewFragment(fragment2);
        FRAGMENT *fragment4 = addNewFragment(fragment3);
        FRAGMENT *fragment5 = addNewFragment(fragment4);
        FRAGMENT *fragment6 = addNewFragment(fragment5);
        FRAGMENT *fragment7 = addNewFragment(fragment6);
        FRAGMENT *fragment8 = addNewFragment(fragment7);
        //TODO: not finished yet!!!!

        //sside patches with spiral code i, 2*sside + 1 (i=0,...,sside-1)
        int lowerbound = sside + 1;
        int upperbound = 3 * sside + 1;
        if (!mirror) {
            lowerbound += HALFFLOOR(sside + 1);
            upperbound -= HALFFLOOR(sside);
        }
        is->code[1] = 2 * sside + 1;
        for (i = lowerbound; i <= upperbound; i++) {
            processStructure(patch, NULL);
            is->code[0]++;
        }
    }
}

int getTwoPentagonsConesCount(int sside, boolean symmetric, boolean mirror) {
    if (symmetric) {
        if (mirror) {
            return sside;
        } else {
            return HALFFLOOR(sside) + 1;
        }
    } else {
        int lowerbound = sside + 1;
        int upperbound = 3 * sside + 1;
        if (!mirror) {
            lowerbound += HALFFLOOR(sside + 1);
            upperbound -= HALFFLOOR(sside);
        }
        return upperbound - lowerbound + 1;
    }
}
