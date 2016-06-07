#include "LKH.h"



int Excludable(Node * ta, Node * tb)
{
    if (ta == tb->OldPred)
        return !tb->OldPredExcluded;
    if (ta == tb->OldSuc)
        return !tb->OldSucExcluded;
    return 0;
}
