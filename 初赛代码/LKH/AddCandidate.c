#include "LKH.h"

int AddCandidate(Node * From, Node * To, int Cost, int Alpha)
{
    int Count;
    Candidate *NFrom;

    if (From->Subproblem != FirstNode->Subproblem)
        return 0;
    if (From->CandidateSet == 0)
        assert(From->CandidateSet =
               (Candidate *) calloc(3, sizeof(Candidate)));
    if (From == To || To->Subproblem != FirstNode->Subproblem ||
        !IsPossibleCandidate(From, To))
        return 0;
    Count = 0;
    for (NFrom = From->CandidateSet; NFrom->To && NFrom->To != To; NFrom++)
        Count++;
    if (NFrom->To) {
        if (NFrom->Alpha == INT_MAX)
            NFrom->Alpha = Alpha;
        return 0;
    }
    NFrom->Cost = Cost;
    NFrom->Alpha = Alpha;
    NFrom->To = To;
    assert(From->CandidateSet =
           (Candidate *) realloc(From->CandidateSet,
                                 (Count + 2) * sizeof(Candidate)));
    From->CandidateSet[Count + 1].To = 0;
    return 1;
}
