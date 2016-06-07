#include "LKH.h"


int Forbidden(const Node * ta, const Node * tb)
{
    return ProblemType == ATSP &&
        (ta->Id <= DimensionSaved) == (tb->Id <= DimensionSaved);
}
