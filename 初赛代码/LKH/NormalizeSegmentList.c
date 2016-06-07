#include "LKH.h"



void NormalizeSegmentList()
{
    Segment *s1, *s2;

    s1 = FirstSegment;
    do {
        if (!s1->Parent->Reversed)
            s2 = s1->Suc;
        else {
            s2 = s1->Pred;
            s1->Pred = s1->Suc;
            s1->Suc = s2;
        }
    }
    while ((s1 = s2) != FirstSegment);
}
