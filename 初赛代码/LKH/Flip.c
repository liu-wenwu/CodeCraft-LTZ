#include "LKH.h"



void Flip(Node * t1, Node * t2, Node * t3)
{
    Node *s1, *s2, *t4;
    int R, Temp, Ct2t3, Ct4t1;

    assert(t1->Pred == t2 || t1->Suc == t2);
    if (t3 == t2->Pred || t3 == t2->Suc)
        return;
    t4 = t1->Suc == t2 ? t3->Pred : t3->Suc;
    if (t1->Suc != t2) {
        s1 = t1;
        t1 = t2;
        t2 = s1;
        s1 = t3;
        t3 = t4;
        t4 = s1;
    }
    /* Find the segment with the smallest number of nodes */
    if ((R = t2->Rank - t3->Rank) < 0)
        R += Dimension;
    if (2 * R > Dimension) {
        s1 = t3;
        t3 = t2;
        t2 = s1;
        s1 = t4;
        t4 = t1;
        t1 = s1;
    }
    Ct2t3 = C(t2, t3);
    Ct4t1 = C(t4, t1);
    /* Swap segment (t3 --> t1) */
    R = t1->Rank;
    t1->Suc = 0;
    s2 = t3;
    while ((s1 = s2)) {
        s2 = s1->Suc;
        s1->Suc = s1->Pred;
        s1->Pred = s2;
        s1->Rank = R--;
        Temp = s1->SucCost;
        s1->SucCost = s1->PredCost;
        s1->PredCost = Temp;
    }
    (t3->Suc = t2)->Pred = t3;
    (t4->Suc = t1)->Pred = t4;
    t3->SucCost = t2->PredCost = Ct2t3;
    t1->PredCost = t4->SucCost = Ct4t1;
    SwapStack[Swaps].t1 = t1;
    SwapStack[Swaps].t2 = t2;
    SwapStack[Swaps].t3 = t3;
    SwapStack[Swaps].t4 = t4;
    Swaps++;
    Hash ^= (Rand[t1->Id] * Rand[t2->Id]) ^
        (Rand[t3->Id] * Rand[t4->Id]) ^
        (Rand[t2->Id] * Rand[t3->Id]) ^ (Rand[t4->Id] * Rand[t1->Id]);
}
