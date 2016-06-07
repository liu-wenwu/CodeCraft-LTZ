#include "LKH.h"


int Between(const Node * ta, const Node * tb, const Node * tc)
{
    int a, b = tb->Rank, c;

    if (!Reversed) {
        a = ta->Rank;
        c = tc->Rank;
    } else {
        a = tc->Rank;
        c = ta->Rank;
    }
    return a <= c ? b >= a && b <= c : b >= a || b <= c;
}
