#include "LKH.h"


int IsPossibleCandidate(Node * From, Node * To)
{
    Node *Na, *Nb, *Nc, *N;

    if (InInitialTour(From, To) ||
        From->SubproblemSuc == To || To->SubproblemSuc == From ||
        FixedOrCommon(From, To))
        return 1;
    if (From->FixedTo2 || To->FixedTo2)
        return 0;
    if (MergeTourFiles < 2)
        return 1;
    if (!From->Head) {
        Nb = FirstNode;
        do {
            Na = Nb;
            Nb = Na->MergeSuc[0];
        } while (Nb != FirstNode && FixedOrCommon(Na, Nb));
        if (Nb != FirstNode) {
            N = Nb;
            do {
                Nc = Nb;
                do {
                    Na = Nb;
                    Na->Head = Nc;
                    Nb = Na->MergeSuc[0];
                } while (FixedOrCommon(Na, Nb));
                do
                    Nc->Tail = Na;
                while ((Nc = Nc->MergeSuc[0]) != Nb);
            } while (Nb != N);
        } else {
            do
                Nb->Head = Nb->Tail = FirstNode;
            while ((Nb = Nb->Suc) != FirstNode);
        }
    }
    if (From->Head == To->Head ||
        (From->Head != From && From->Tail != From) ||
        (To->Head != To && To->Tail != To))
        return 0;
    return 1;
}
