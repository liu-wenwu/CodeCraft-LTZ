#include "Segment.h"
#include "LKH.h"


void StoreTour()
{
    Node *t, *u;
    Candidate *Nt;
    int i;

    while (Swaps > 0) {
        Swaps--;
        for (i = 1; i <= 4; i++) {
            t = i == 1 ? SwapStack[Swaps].t1 :
                i == 2 ? SwapStack[Swaps].t2 :
                i == 3 ? SwapStack[Swaps].t3 : SwapStack[Swaps].t4;
            Activate(t);
            t->OldPred = t->Pred;
            t->OldSuc = t->Suc;
            t->OldPredExcluded = t->OldSucExcluded = 0;
            t->Cost = INT_MAX;
            for (Nt = t->CandidateSet; (u = Nt->To); Nt++)
                if (u != t->Pred && u != t->Suc && Nt->Cost < t->Cost)
                    t->Cost = Nt->Cost;
        }
    }
}
