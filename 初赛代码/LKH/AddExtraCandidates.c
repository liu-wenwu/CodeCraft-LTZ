#include "LKH.h"



void AddExtraCandidates(int K, int CandidateSetType, int Symmetric)
{
    Candidate *Nt, *ExtraCandidateSet, **SavedCandidateSet;
    Node *t;

    assert(SavedCandidateSet =
           (Candidate **) malloc((1 + DimensionSaved) *
                                 sizeof(Candidate *)));
    t = FirstNode;
    do {
        SavedCandidateSet[t->Id] = t->CandidateSet;
        t->CandidateSet = 0;
    } while ((t = t->Suc) != FirstNode);
    if (CandidateSetType == NN)
        CreateNearestNeighborCandidateSet(K);
    else if (CandidateSetType == QUADRANT)
        CreateQuadrantCandidateSet(K);
    t = FirstNode;
    do {
        ExtraCandidateSet = t->CandidateSet;
        t->CandidateSet = SavedCandidateSet[t->Id];
        for (Nt = ExtraCandidateSet; Nt && Nt->To; Nt++) {
            AddCandidate(t, Nt->To, Nt->Cost, Nt->Alpha);
            if (Symmetric)
                AddCandidate(Nt->To, t, Nt->Cost, Nt->Alpha);
        }
        free(ExtraCandidateSet);
    } while ((t = t->Suc) != FirstNode);
    free(SavedCandidateSet);
}
