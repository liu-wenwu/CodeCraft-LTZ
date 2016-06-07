#include "LKH.h"


static Node *RandomNode();
static int compare(const void *Na, const void *Nb);

void KSwapKick(int K)
{
    Node **s, *N;
    int Count, i;

    assert(s = (Node **) malloc(K * sizeof(Node *)));
    Count = 0;
    N = FirstNode;
    do {
        N->Rank = ++Count;
        N->V = 0;
    } while ((N = N->Suc) != FirstNode);
    N = s[0] = RandomNode();
    if (!N)
        goto End_KSwapKick;
    N->V = 1;
    for (i = 1; i < K; i++) {
        N = s[i] = RandomNode();
        if (!N)
            K = i;
        else
            N->V = 1;
    }
    if (K < 3)
        goto End_KSwapKick;
    qsort(s, K, sizeof(Node *), compare);
    for (i = 0; i < K; i++)
        s[i]->OldSuc = s[i]->Suc;
    for (i = 0; i < K; i++)
        Link(s[(i + 2) % K], s[i]->OldSuc);
  End_KSwapKick:
    free(s);
}



static Node *RandomNode()
{
    Node *N;
    int Count;

    if (Dimension == DimensionSaved)
        N = &NodeSet[1 + Random() % Dimension];
    else {
        N = FirstNode;
        for (Count = Random() % Dimension; Count > 0; Count--)
            N = N->Suc;
    }
    Count = 0;
    while ((FixedOrCommon(N, N->Suc) || N->V) && Count < Dimension) {
        N = N->Suc;
        Count++;
    }
    return Count < Dimension ? N : 0;
}

static int compare(const void *Na, const void *Nb)
{
    return (*(Node **) Na)->Rank - (*(Node **) Nb)->Rank;
}
