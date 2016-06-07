#include "LKH.h"
#include "Genetic.h"


static char Delimiters[] = "= \n\t\r\f\v\xef\xbb\xbf";
static char *GetFileName(char *Line);
static char *ReadYesOrNo(int *V);
#undef max
static size_t max(size_t a, size_t b);

void ReadParameters()
{
    char *Line, *Keyword, *Token, *Name;
    unsigned int i;

    ProblemFileName = PiFileName = InputTourFileName =
        OutputTourFileName = TourFileName = 0;
    CandidateFiles = MergeTourFiles = 0;
    AscentCandidates = 50;
    BackboneTrials = 0;
    Backtracking = 0;
    CandidateSetSymmetric = 0;
    CandidateSetType = ALPHA;
    Crossover = ERXT;
    DelaunayPartitioning = 0;
    DelaunayPure = 0;
    Excess = -1;
    ExtraCandidates = 0;
    ExtraCandidateSetSymmetric = 0;
    ExtraCandidateSetType = QUADRANT;
    Gain23Used = 1;
    GainCriterionUsed = 1;
    InitialPeriod = -1;
    InitialStepSize = 0;
    InitialTourAlgorithm = WALK;
    InitialTourFraction = 1.0;
    KarpPartitioning = 0;
    KCenterPartitioning = 0;
    KMeansPartitioning = 0;
    Kicks = 1;
    KickType = 0;
    MaxBreadth = INT_MAX;
    MaxCandidates = 5;
    MaxPopulationSize = 0;
    MaxSwaps = -1;
    MaxTrials = -1;
    MoorePartitioning = 0;
    MoveType = 5;
    NonsequentialMoveType = -1;
    Optimum = MINUS_INFINITY;
    PatchingA = 2;
    PatchingC = 3;
    PatchingAExtended = 0;
    PatchingARestricted = 0;
    PatchingCExtended = 0;
    PatchingCRestricted = 0;
    Precision = 1;//修改这里，原值100
    RestrictedSearch = 1;
    RohePartitioning = 0;
    Runs = 1;//修改这里，原值10
    Seed = 1;
    SierpinskiPartitioning = 0;
    StopAtOptimum = 1;
    Subgradient = 1;
    SubproblemBorders = 0;
    SubproblemsCompressed = 0;
    SubproblemSize = 0;
    SubsequentMoveType = 0;
    SubsequentPatching = 1;
    TimeLimit = 5;//修改这里，原值DBL_MAX
    TraceLevel = 0;//修改这里，原值1


}
