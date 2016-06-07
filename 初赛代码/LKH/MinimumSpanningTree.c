#include "LKH.h"
#include "Heap.h"


void MinimumSpanningTree(int Sparse)
{
    Node *Blue;         /* Points to the last node included in the tree */
    Node *NextBlue = 0; /* Points to the provisional next node to be included */
    Node *N;
    Candidate *NBlue;
    int d;

    Blue = N = FirstNode;
    Blue->Dad = 0;              /* The root of the tree has no father */
    if (Sparse && Blue->CandidateSet) {
        /* The graph is sparse */
        /* Insert all nodes in the heap */
        Blue->Loc = 0;          /* A blue node is not in the heap */
        while ((N = N->Suc) != FirstNode) {
            N->Dad = Blue;
            N->Cost = N->Rank = INT_MAX;
            HeapLazyInsert(N);
        }
        /* Update all neighbors to the blue node */
        for (NBlue = Blue->CandidateSet; (N = NBlue->To); NBlue++) {
            if (FixedOrCommon(Blue, N)) {
                N->Dad = Blue;
                N->Cost = NBlue->Cost + Blue->Pi + N->Pi;
                N->Rank = INT_MIN;
                HeapSiftUp(N);
            } else if (!Blue->FixedTo2 && !N->FixedTo2) {
                N->Dad = Blue;
                N->Cost = N->Rank = NBlue->Cost + Blue->Pi + N->Pi;
                HeapSiftUp(N);
            }
        }
        /* Loop as long as there are more nodes to include in the tree */
        while ((NextBlue = HeapDeleteMin())) {
            Follow(NextBlue, Blue);
            Blue = NextBlue;
            /* Update all neighbors to the blue node */
            for (NBlue = Blue->CandidateSet; (N = NBlue->To); NBlue++) {
                if (!N->Loc)
                    continue;
                if (FixedOrCommon(Blue, N)) {
                    N->Dad = Blue;
                    N->Cost = NBlue->Cost + Blue->Pi + N->Pi;
                    N->Rank = INT_MIN;
                    HeapSiftUp(N);
                } else if (!Blue->FixedTo2 && !N->FixedTo2 &&
                           (d = NBlue->Cost + Blue->Pi + N->Pi) < N->Cost)
                {
                    N->Dad = Blue;
                    N->Cost = N->Rank = d;
                    HeapSiftUp(N);
                }
            }
        }
    } else {
        /* The graph is dense */
        while ((N = N->Suc) != FirstNode)
            N->Cost = INT_MAX;
        /* Loop as long as there a more nodes to include in the tree */
        while ((N = Blue->Suc) != FirstNode) {
            int Min = INT_MAX;
            /* Update all non-blue nodes (the successors of Blue in the list) */
            do {
                if (FixedOrCommon(Blue, N)) {
                    N->Dad = Blue;
                    N->Cost = D(Blue, N);
                    NextBlue = N;
                    Min = INT_MIN;
                } else {
                    if (!Blue->FixedTo2 && !N->FixedTo2 &&
                        !Forbidden(Blue, N) &&
                        (!c || c(Blue, N) < N->Cost) &&
                        (d = D(Blue, N)) < N->Cost) {
                        N->Cost = d;
                        N->Dad = Blue;
                    }
                    if (N->Cost < Min) {
                        Min = N->Cost;
                        NextBlue = N;
                    }
                }
            }
            while ((N = N->Suc) != FirstNode);
            Follow(NextBlue, Blue);
            Blue = NextBlue;
        }
    }
}
