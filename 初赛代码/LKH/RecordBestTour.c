#include "LKH.h"


void RecordBestTour()
{
    int i, Dim = ProblemType != ATSP ? Dimension : Dimension / 2;

    for (i = 0; i <= Dim; i++)
        BestTour[i] = BetterTour[i];
}
