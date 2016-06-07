#include "LKH.h"


void Activate(Node * N)
{
    if (N->Next != 0)
        return;
    if (FirstActive == 0)
        FirstActive = LastActive = N;
    else
        LastActive = LastActive->Next = N;
    LastActive->Next = FirstActive;
}
