#include "LKH.h"



void RecordBetterTour()
{
    Node *N;
    int i = 0;

    N = FirstNode;
    do {
        if (ProblemType != ATSP)
            BetterTour[++i] = N->Id;
        else if (N->Id <= Dimension / 2) {
            i++;
            if (N->Suc->Id != N->Id + Dimension / 2)
                BetterTour[i] = N->Id;
            else
                BetterTour[Dimension / 2 - i + 1] = N->Id;
        }
        N->NextBestSuc = N->BestSuc;
        N->BestSuc = N->Suc;
    }
    while ((N = N->Suc) != FirstNode);
    BetterTour[0] = BetterTour[ProblemType != ATSP ? Dimension : Dimension / 2];
}
