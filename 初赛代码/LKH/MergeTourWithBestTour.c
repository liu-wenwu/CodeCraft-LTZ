#include "LKH.h"



GainType MergeTourWithBestTour()
{
    Node *N1, *N2, *M1, *M2;
    int i;

    if (ProblemType != ATSP) {
        for (i = 1; i <= Dimension; i++) {
            N1 = &NodeSet[BestTour[i - 1]];
            N2 = &NodeSet[BestTour[i]];
            N1->Next = N2;
        }
    } else {
        int Dim = Dimension / 2;
        for (i = 1; i <= Dim; i++) {
            N1 = &NodeSet[BestTour[i - 1]];
            N2 = &NodeSet[BestTour[i]];
            M1 = &NodeSet[N1->Id + Dim];
            M2 = &NodeSet[N2->Id + Dim];
            M1->Next = N1;
            N1->Next = M2;
            M2->Next = N2;
        }
    }
    return MergeWithTour();
}
