#include "LKH.h"


void SymmetrizeCandidateSet()
{
    Node *From, *To;
    Candidate *NFrom;

    From = FirstNode;
    do {
        for (NFrom = From->CandidateSet; NFrom && (To = NFrom->To); NFrom++)
            AddCandidate(To, From, NFrom->Cost, NFrom->Alpha);
    }
    while ((From = From->Suc) != FirstNode);
    ResetCandidateSet();
}
