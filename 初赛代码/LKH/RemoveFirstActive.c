#include "LKH.h"


Node *RemoveFirstActive()
{
    Node *N = FirstActive;
    if (FirstActive == LastActive)
        FirstActive = LastActive = 0;
    else
        LastActive->Next = FirstActive = FirstActive->Next;
    if (N)
        N->Next = 0;
    return N;
}
