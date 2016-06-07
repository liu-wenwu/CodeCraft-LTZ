#include "LKH.h"



int IsBackboneCandidate(const Node * ta, const Node * tb)
{
    Candidate *Nta;

    for (Nta = ta->BackboneCandidateSet; Nta && Nta->To; Nta++)
        if (Nta->To == tb)
            return 1;
    return 0;
}
