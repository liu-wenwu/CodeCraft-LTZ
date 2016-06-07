#include "LKH.h"


void Connect(Node * N1, int Max, int Sparse)
{
    Node *N;
    Candidate *NN1;
    int d;

    N1->Next = 0;
    N1->NextCost = INT_MAX;
    if (!Sparse || N1->CandidateSet == 0 ||
        N1->CandidateSet[0].To == 0 || N1->CandidateSet[1].To == 0) {
        /* Find the requested edge in a dense graph */
        N = FirstNode;
        do {
            if (N == N1 || N == N1->Dad || N1 == N->Dad)
                continue;
            if (FixedOrCommon(N1, N)) {
                N1->NextCost = D(N1, N);
                N1->Next = N;
                return;
            }
            if (!N1->FixedTo2 && !N->FixedTo2 &&
                !Forbidden(N1, N) &&
                (!c || c(N1, N) < N1->NextCost) &&
                (d = D(N1, N)) < N1->NextCost) {
                N1->NextCost = d;
                if (d <= Max)
                    return;
                N1->Next = N;
            }
        }
        while ((N = N->Suc) != FirstNode);
    } else {
        /* Find the requested edge in a sparse graph */
        for (NN1 = N1->CandidateSet; (N = NN1->To); NN1++) {
            if (N == N1->Dad || N1 == N->Dad)
                continue;
            if (FixedOrCommon(N1, N)) {
                N1->NextCost = NN1->Cost + N1->Pi + N->Pi;
                N1->Next = N;
                return;
            }
            if (!N1->FixedTo2 && !N->FixedTo2 &&
                (d = NN1->Cost + N1->Pi + N->Pi) < N1->NextCost) {
                N1->NextCost = d;
                if (d <= Max)
                    return;
                N1->Next = N;
            }
        }
    }
}
