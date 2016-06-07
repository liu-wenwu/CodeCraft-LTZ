#include "LKH.h"


void ResetCandidateSet()
{
    Node *From;
    Candidate *NFrom, *NN, Temp;

    From = FirstNode;
    /* Loop for all nodes */
    do {
        if (!From->CandidateSet)
            continue;
        /* Reorder the candidate array of From */
        for (NFrom = From->CandidateSet; NFrom->To; NFrom++) {
            Temp = *NFrom;
            for (NN = NFrom - 1;
                 NN >= From->CandidateSet &&
                 (Temp.Alpha < NN->Alpha ||
                  (Temp.Alpha == NN->Alpha && Temp.Cost < NN->Cost)); NN--)
                *(NN + 1) = *NN;
            *(NN + 1) = Temp;
        }
        NFrom--;
        /* Remove included edges */
        while (NFrom >= From->CandidateSet + 2 && NFrom->Alpha == INT_MAX)
            NFrom--;
        NFrom++;
        NFrom->To = 0;
        /* Remove impossible candidates */
        for (NFrom = From->CandidateSet; NFrom->To; NFrom++) {
            if (!IsPossibleCandidate(From, NFrom->To)) {
                for (NN = NFrom; NN->To; NN++)
                    *NN = *(NN + 1);
                NFrom--;
            }
        }
    }
    while ((From = From->Suc) != FirstNode);
}
