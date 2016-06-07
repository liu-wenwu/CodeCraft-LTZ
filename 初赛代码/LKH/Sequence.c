#include "Sequence.h"
#include "Segment.h"

static Node *tp1;

static int compare(const void *pa, const void *pb)
{
    return BETWEEN(tp1, t[*(int *) pa], t[*(int *) pb]) ? -1 : 1;
}

void FindPermutation(int k)
{
    int i, j;

    for (i = j = 1; j <= k; i += 2, j++)
        p[j] = SUC(t[i]) == t[i + 1] ? i : i + 1;
    tp1 = t[p[1]];
    qsort(p + 2, k - 1, sizeof(int), compare);
    for (j = 2 * k; j >= 2; j -= 2) {
        p[j - 1] = i = p[j / 2];
        p[j] = i & 1 ? i + 1 : i - 1;
    }
    for (i = 1; i <= 2 * k; i++)
        q[p[i]] = i;
}



int FeasibleKOptMove(int k)
{
    int Count, i;

    FindPermutation(k);
    for (Count = 1, i = 2 * k; (i = q[incl[p[i]]] ^ 1); Count++);
    return Count == k;
}



int Cycles(int k)
{
    int i, j, Count = 0;

    for (i = 1; i <= 2 * k; i++)
        cycle[i] = 0;
    for (i = 1; i <= 2 * k; i++) {
        if (!cycle[p[i]]) {
            Count++;
            j = i;
            do {
                cycle[p[j]] = Count;
                j = q[incl[p[j]]];
                cycle[p[j]] = Count;
                if ((j ^= 1) > 2 * k)
                    j = 1;
            }
            while (j != i);
        }
    }
    return Count;
}



int Added(const Node * ta, const Node * tb)
{
    return ta->Added1 == tb || ta->Added2 == tb;
}



int Deleted(const Node * ta, const Node * tb)
{
    return ta->Deleted1 == tb || ta->Deleted2 == tb;
}


void MarkAdded(Node * ta, Node * tb)
{
    if (!ta->Added1)
        ta->Added1 = tb;
    else if (!ta->Added2)
        ta->Added2 = tb;
    if (!tb->Added1)
        tb->Added1 = ta;
    else if (!tb->Added2)
        tb->Added2 = ta;
}



void MarkDeleted(Node * ta, Node * tb)
{
    if (!ta->Deleted1)
        ta->Deleted1 = tb;
    else if (!ta->Deleted2)
        ta->Deleted2 = tb;
    if (!tb->Deleted1)
        tb->Deleted1 = ta;
    else if (!tb->Deleted2)
        tb->Deleted2 = ta;
}



void UnmarkAdded(Node * ta, Node * tb)
{
    if (ta->Added1 == tb)
        ta->Added1 = 0;
    else if (ta->Added2 == tb)
        ta->Added2 = 0;
    if (tb->Added1 == ta)
        tb->Added1 = 0;
    else if (tb->Added2 == ta)
        tb->Added2 = 0;
}



void UnmarkDeleted(Node * ta, Node * tb)
{
    if (ta->Deleted1 == tb)
        ta->Deleted1 = 0;
    else if (ta->Deleted2 == tb)
        ta->Deleted2 = 0;
    if (tb->Deleted1 == ta)
        tb->Deleted1 = 0;
    else if (tb->Deleted2 == ta)
        tb->Deleted2 = 0;
}
