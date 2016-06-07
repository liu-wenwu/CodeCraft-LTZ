#include "Segment.h"
#include "LKH.h"


void RestoreTour()
{
    Node *t1, *t2, *t3, *t4;

    /* Loop as long as the stack is not empty */
    while (Swaps > 0) {
        /* Undo topmost 2-opt move */
        Swaps--;
        t1 = SwapStack[Swaps].t1;
        t2 = SwapStack[Swaps].t2;
        t3 = SwapStack[Swaps].t3;
        t4 = SwapStack[Swaps].t4;
        Swap1(t3, t2, t1);
        Swaps--;
        /* Make edges (t1,t2) and (t2,t3) excludable again */
        t1->OldPredExcluded = t1->OldSucExcluded = 0;
        t2->OldPredExcluded = t2->OldSucExcluded = 0;
        t3->OldPredExcluded = t3->OldSucExcluded = 0;
        t4->OldPredExcluded = t4->OldSucExcluded = 0;
    }
}
