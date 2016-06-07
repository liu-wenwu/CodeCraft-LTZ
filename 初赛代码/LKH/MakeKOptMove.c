#include "Sequence.h"
#include "Segment.h"


static void Reverse(int i, int j);
static int Score(int Left, int Right, int K);

void MakeKOptMove(int K)
{
    int i, j, Best_i = 0, Best_j = 0, BestScore, s;

    FindPermutation(K);
  FindNextReversal:
    /* Find the oriented reversal that has maximal score */
    BestScore = -1;
    for (i = 1; i <= 2 * K - 2; i++) {
        j = q[incl[p[i]]];
        if (j >= i + 2 && (i & 1) == (j & 1) &&
            (s = i & 1 ? Score(i + 1, j, K) :
             Score(i, j - 1, K)) > BestScore) {
            BestScore = s;
            Best_i = i;
            Best_j = j;
        }
    }
    if (BestScore >= 0) {
        i = Best_i;
        j = Best_j;
        if (i & 1) {
            Swap1(t[p[i + 1]], t[p[i]], t[p[j]]);
            Reverse(i + 1, j);
        } else {
            Swap1(t[p[i - 1]], t[p[i]], t[p[j]]);
            Reverse(i, j - 1);
        }
        goto FindNextReversal;
    }
    /* No more oriented reversals. Cut a simpe hurdle, if any.
     * Note that there can be no super hurdles */
    for (i = 1; i <= 2 * K - 3; i += 2) {
        j = q[incl[p[i]]];
        if (j >= i + 3) {
            Swap1(t[p[i]], t[p[i + 1]], t[p[j]]);
            Reverse(i + 1, j - 1);
            goto FindNextReversal;
        }
    }
}

/*
 * The Reverse function reverses the sequence of elements in p[i:j].
 * The inverse permutation q is updated accordingly.
 */

static void Reverse(int i, int j)
{
    for (; i < j; i++, j--) {
        int pi = p[i];
        q[p[i] = p[j]] = i;
        q[p[j] = pi] = j;
    }
}

/*
 * The Score function computes the score of a reversal. The score is the 
 * number of oriented pairs in the resulting reversal.
 */

static int Score(int Left, int Right, int K)
{
    int Count = 0, i, j;

    Reverse(Left, Right);
    for (i = 1; i <= 2 * K - 2; i++) {
        j = q[incl[p[i]]];
        if (j >= i + 2 && (i & 1) == (j & 1))
            Count++;
    }
    Reverse(Left, Right);
    return Count;
}
