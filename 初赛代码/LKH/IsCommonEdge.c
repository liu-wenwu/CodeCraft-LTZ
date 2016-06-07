#include "LKH.h"



int IsCommonEdge(const Node * ta, const Node * tb)
{
    int i;

    if (MergeTourFiles < 2)
        return 0;
    for (i = 0; i < MergeTourFiles; i++)
        if (ta->MergeSuc[i] != tb && tb->MergeSuc[i] != ta)
            return 0;
    return 1;
}
