#include "LKH.h"


void Exclude(Node * ta, Node * tb)
{
    if (ta == tb->Pred || ta == tb->Suc)
        return;
    if (ta == tb->OldPred)
        tb->OldPredExcluded = 1;
    else if (ta == tb->OldSuc)
        tb->OldSucExcluded = 1;
    if (tb == ta->OldPred)
        ta->OldPredExcluded = 1;
    else if (tb == ta->OldSuc)
        ta->OldSucExcluded = 1;
}
