#include "LKH.h"



int Between_SL(const Node * ta, const Node * tb, const Node * tc)
{
    const Segment *Pa, *Pb, *Pc;

    if (tb == ta || tb == tc)
        return 1;
    if (ta == tc)
        return 0;
    Pa = ta->Parent;
    Pb = tb->Parent;
    Pc = tc->Parent;
    if (Pa == Pc) {
        if (Pb == Pa)
            return (Reversed == Pa->Reversed) ==
                (ta->Rank < tc->Rank ?
                 tb->Rank > ta->Rank && tb->Rank < tc->Rank :
                 tb->Rank > ta->Rank || tb->Rank < tc->Rank);
        return (Reversed == Pa->Reversed) == (ta->Rank > tc->Rank);
    }
    if (Pb == Pc)
        return (Reversed == Pb->Reversed) == (tb->Rank < tc->Rank);
    if (Pa == Pb)
        return (Reversed == Pa->Reversed) == (ta->Rank < tb->Rank);
    return Reversed !=
        (Pa->Rank < Pc->Rank ?
         Pb->Rank > Pa->Rank && Pb->Rank < Pc->Rank :
         Pb->Rank > Pa->Rank || Pb->Rank < Pc->Rank);
}
