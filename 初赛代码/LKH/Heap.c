#include "LKH.h"
#include "Heap.h"


static int HeapCount;    /* Its current number of elements */
static int HeapCapacity; /* Its capacity */

void MakeHeap(int Size)
{
    assert(Heap = (Node **) malloc((Size + 1) * sizeof(Node *)));
    HeapCapacity = Size;
    HeapCount = 0;
}



void HeapSiftUp(Node * N)
{
    int Loc = N->Loc, Parent = Loc / 2;

    while (Parent && N->Rank < Heap[Parent]->Rank) {
        Heap[Loc] = Heap[Parent];
        Heap[Loc]->Loc = Loc;
        Loc = Parent;
        Parent /= 2;
    }
    Heap[Loc] = N;
    N->Loc = Loc;
}



void HeapSiftDown(Node * N)
{
    int Loc = N->Loc, Child;

    while (Loc <= HeapCount / 2) {
        Child = 2 * Loc;
        if (Child < HeapCount && Heap[Child + 1]->Rank < Heap[Child]->Rank)
            Child++;
        if (N->Rank <= Heap[Child]->Rank)
            break;
        Heap[Loc] = Heap[Child];
        Heap[Loc]->Loc = Loc;
        Loc = Child;
    }
    Heap[Loc] = N;
    N->Loc = Loc;
}



Node *HeapDeleteMin()
{
    Node *Remove;

    if (!HeapCount)
        return 0;
    Remove = Heap[1];
    Heap[1] = Heap[HeapCount--];
    Heap[1]->Loc = 1;
    HeapSiftDown(Heap[1]);
    Remove->Loc = 0;
    return Remove;
}



void HeapInsert(Node * N)
{
    HeapLazyInsert(N);
    HeapSiftUp(N);
}


void HeapDelete(Node * N)
{
    int Loc = N->Loc;
    if (!Loc)
        return;
    Heap[Loc] = Heap[HeapCount--];
    Heap[Loc]->Loc = Loc;
    if (Heap[Loc]->Rank > N->Rank)
        HeapSiftDown(Heap[Loc]);
    else
        HeapSiftUp(Heap[Loc]);
    N->Loc = 0;
}



void HeapLazyInsert(Node * N)
{
    assert(HeapCount < HeapCapacity);
    Heap[++HeapCount] = N;
    N->Loc = HeapCount;
}



void Heapify()
{
    int Loc;
    for (Loc = HeapCount / 2; Loc >= 1; Loc--)
        HeapSiftDown(Heap[Loc]);
}
