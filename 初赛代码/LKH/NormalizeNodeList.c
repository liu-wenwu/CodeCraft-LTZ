#include "Segment.h"
#include "LKH.h"


void NormalizeNodeList()
{
    Node *t1, *t2;

    t1 = FirstNode;
    do {
        t2 = SUC(t1);
        t1->Pred = PRED(t1);
        t1->Suc = t2;
    }
    while ((t1 = t2) != FirstNode);
}
