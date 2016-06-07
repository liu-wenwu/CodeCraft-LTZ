#include "LKH.h"



int IsCandidate(const Node * ta, const Node * tb)
{
    Candidate *Nta;

    for (Nta = ta->CandidateSet; Nta && Nta->To; Nta++)
        if (Nta->To == tb)
            return 1;
    return 0;
}
