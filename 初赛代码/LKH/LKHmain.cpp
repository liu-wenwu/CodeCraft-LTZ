extern "C"{
#include "LKH.h"
#include "Genetic.h"
#include "Heap.h"

}
#include "../common.h"



static void CreateNodes()
{
	Node *Prev = 0, *N = 0;
	int i;

	if (Dimension <= 0)
		eprintf("DIMENSION is not positive (or not specified)");
	if (ProblemType == ATSP)
		Dimension *= 2;
	else if (ProblemType == HPP) {
		Dimension++;
		if (Dimension > MaxMatrixDimension)
			eprintf("Dimension too large in HPP problem");
	}
	assert(NodeSet = (Node *)calloc(Dimension + 1, sizeof(Node)));
	for (i = 1; i <= Dimension; i++, Prev = N) {
		N = &NodeSet[i];
		if (i == 1)
			FirstNode = N;
		else
			Link(Prev, N);
		N->Id = i;
		if (MergeTourFiles >= 1)
			assert(N->MergeSuc =
			(Node **)calloc(MergeTourFiles, sizeof(Node *)));
	}
	Link(N, FirstNode);
}




static void init_graph()
{
	Node *Ni, *Nj;
	int i, j, n, W;


	if (!FirstNode)
		CreateNodes();

	n = Dimension / 2;
	assert(CostMatrix = (int *)calloc((size_t)n * n, sizeof(int)));
	for (Ni = FirstNode; Ni->Id <= n; Ni = Ni->Suc)
		Ni->C = &CostMatrix[(size_t)(Ni->Id - 1) * n] - 1;

	n = Dimension / 2;
	for (i = 1; i <= n; i++) {
		Ni = &NodeSet[i];
		for (j = 1; j <= n; j++) {
			W = TSP_DIST[i - 1][j - 1];//
			if (W == 0) W = INFW;//
			Ni->C[j] = W;
			if (i != j && W > M)
				M = W;
		}
		Nj = &NodeSet[i + n];
		if (!Ni->FixedTo1)
			Ni->FixedTo1 = Nj;
		else if (!Ni->FixedTo2)
			Ni->FixedTo2 = Nj;
		if (!Nj->FixedTo1)
			Nj->FixedTo1 = Ni;
		else if (!Nj->FixedTo2)
			Nj->FixedTo2 = Ni;
	}
	Distance = Distance_ATSP;
	WeightType = -1;


}




void init(){
	FreeStructures();
	FirstNode = 0;
	WeightType = EXPLICIT;

	WeightFormat = FULL_MATRIX;
	ProblemType = ATSP;

	CoordType = NO_COORDS;
//	Name = "Unnamed";
	Type = EdgeWeightType = EdgeWeightFormat = 0;
	EdgeDataFormat = NodeCoordType = DisplayDataType = 0;
	Distance = 0;
	C = 0;
	c = 0;


	Distance = Distance_EXPLICIT;

	DimensionSaved = Dimension = TSP_N;
	init_graph();




	Swaps = 0;

	/* Adjust parameters */
	if (Seed == 0)
		Seed = (unsigned)time(0);
	if (Precision == 0)
		Precision = 100;
	if (InitialStepSize == 0)
		InitialStepSize = 1;
	if (MaxSwaps < 0)
		MaxSwaps = Dimension;
	if (KickType > Dimension / 2)
		KickType = Dimension / 2;

	if (Runs == 0)
		Runs = 10;

	if (MaxCandidates > Dimension - 1)
		MaxCandidates = Dimension - 1;
	if (ExtraCandidates > Dimension - 1)
		ExtraCandidates = Dimension - 1;
	if (SubproblemSize >= Dimension)
		SubproblemSize = Dimension;
	else if (SubproblemSize == 0) {
		if (AscentCandidates > Dimension - 1)
			AscentCandidates = Dimension - 1;
		if (InitialPeriod < 0) {
			InitialPeriod = Dimension / 2;
			if (InitialPeriod < 100)
				InitialPeriod = 100;
		}
		if (Excess < 0)
			Excess = 1.0 / Dimension;
		if (MaxTrials == -1)
			MaxTrials = Dimension;
		MakeHeap(Dimension);
	}

	if (CostMatrix == 0 && Dimension <= MaxMatrixDimension && Distance != 0
		&& Distance != Distance_1 && Distance != Distance_ATSP) {
		Node *Ni, *Nj;
		assert(CostMatrix =
			(int *)calloc((size_t)Dimension * (Dimension - 1) / 2,
			sizeof(int)));
		Ni = FirstNode->Suc;
		do {
			Ni->C =
				&CostMatrix[(size_t)(Ni->Id - 1) * (Ni->Id - 2) / 2] - 1;
			if (ProblemType != HPP || Ni->Id < Dimension)
			for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
				Ni->C[Nj->Id] = Fixed(Ni, Nj) ? 0 : Distance(Ni, Nj);
			else
			for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
				Ni->C[Nj->Id] = 0;
		} while ((Ni = Ni->Suc) != FirstNode);
		WeightType = EXPLICIT;
		c = 0;
	}


	if (Precision > 1 && (WeightType == EXPLICIT || ProblemType == ATSP)) {
		int j, n = ProblemType == ATSP ? Dimension / 2 : Dimension;
		for (int i = 2; i <= n; i++) {
			Node *N = &NodeSet[i];
			for (j = 1; j < i; j++)
			if (N->C[j] * Precision / Precision != N->C[j])
			{
				printf("PRECISION (= %d) is too large", Precision);
				system("pause");
			}

		}
	}



	C = WeightType == EXPLICIT ? C_EXPLICIT : C_FUNCTION;
	D = WeightType == EXPLICIT ? D_EXPLICIT : D_FUNCTION;
	if (SubsequentMoveType == 0)
		SubsequentMoveType = MoveType;
	int K = MoveType >= SubsequentMoveType
		|| !SubsequentPatching ? MoveType : SubsequentMoveType;
	if (PatchingC > K)
		PatchingC = K;
	if (PatchingA > 1 && PatchingA >= PatchingC)
		PatchingA = PatchingC > 2 ? PatchingC - 1 : 1;
	if (NonsequentialMoveType == -1 ||
		NonsequentialMoveType > K + PatchingC + PatchingA - 1)
		NonsequentialMoveType = K + PatchingC + PatchingA - 1;
	if (PatchingC >= 1 && NonsequentialMoveType >= 4) {
		BestMove = BestSubsequentMove = BestKOptMove;
		if (!SubsequentPatching && SubsequentMoveType <= 5) {
			MoveFunction BestOptMove[] =
			{ 0, 0, Best2OptMove, Best3OptMove,
			Best4OptMove, Best5OptMove
			};
			BestSubsequentMove = BestOptMove[SubsequentMoveType];
		}
	}
	else {
		MoveFunction BestOptMove[] = { 0, 0, Best2OptMove, Best3OptMove,
			Best4OptMove, Best5OptMove
		};
		BestMove = MoveType <= 5 ? BestOptMove[MoveType] : BestKOptMove;
		BestSubsequentMove = SubsequentMoveType <= 5 ?
			BestOptMove[SubsequentMoveType] : BestKOptMove;
	}
	if (ProblemType == HCP || ProblemType == HPP)
		MaxCandidates = 0;
	//if (TraceLevel >= 1) {
	//	printff("done\n");
	//	PrintParameters();
	//}





}


int tsp_lkh()
{
	GainType Cost, OldOptimum;
	double Time, LastTime = GetTime();

	/* Read the specification of the problem */


	ReadParameters();
	MaxMatrixDimension = 10000;

	init();


	AllocateStructures();
	CreateCandidateSet();
	InitializeStatistics();


	BestCost = PLUS_INFINITY;



	for (Run = 1; Run <= Runs; Run++) {
		LastTime = GetTime();
		Cost = FindTour();      /* using the Lin-Kernighan heuristic */
		if (MaxPopulationSize > 1) {
			/* Genetic algorithm */
			int i;
			for (i = 0; i < PopulationSize; i++) {
				GainType OldCost = Cost;
				Cost = MergeTourWithIndividual(i);
				if (TraceLevel >= 1 && Cost < OldCost) {
					//				printff("  Merged with %d: Cost = " GainFormat, i + 1,
					///				Cost);
					//		if (Optimum != MINUS_INFINITY && Optimum != 0)
					//		printff(", Gap = %0.4f%%",
					//	100.0 * (Cost - Optimum) / Optimum);
					//printff("\n");
				}
			}
			if (!HasFitness(Cost)) {
				if (PopulationSize < MaxPopulationSize) {
					AddToPopulation(Cost);
					if (TraceLevel >= 1)
						PrintPopulation();
				}
				else if (Cost < Fitness[PopulationSize - 1]) {
					i = ReplacementIndividual(Cost);
					ReplaceIndividualWithTour(i, Cost);
					if (TraceLevel >= 1)
						PrintPopulation();
				}
			}
		}
		else if (Run > 1)
			Cost = MergeTourWithBestTour();
		if (Cost < BestCost) {
			BestCost = Cost;
			RecordBetterTour();
			RecordBestTour();

		}
		OldOptimum = Optimum;
		if (Cost < Optimum) {
			if (FirstNode->InputSuc) {
				Node *N = FirstNode;
				while ((N = N->InputSuc = N->Suc) != FirstNode);
			}
			Optimum = Cost;
			//printff("*** New optimum = " GainFormat " ***\n\n", Optimum);
		}
		Time = fabs(GetTime() - LastTime);
		UpdateStatistics(Cost, Time);
		/*
		if (TraceLevel >= 1 && Cost != PLUS_INFINITY) {
			printff("Run %d: Cost = " GainFormat, Run, Cost);
			if (Optimum != MINUS_INFINITY && Optimum != 0)
				printff(", Gap = %0.4f%%",
				100.0 * (Cost - Optimum) / Optimum);
			printff(", Time = %0.2f sec. %s\n\n", Time,
				Cost < Optimum ? "<" : Cost == Optimum ? "=" : "");
		}*/
		if (StopAtOptimum && Cost == OldOptimum && MaxPopulationSize >= 1) {
			Runs = Run;
			break;
		}
		if (PopulationSize >= 2 &&
			(PopulationSize == MaxPopulationSize ||
			Run >= 2 * MaxPopulationSize) && Run < Runs) {
			Node *N;
			int Parent1, Parent2;
			Parent1 = LinearSelection(PopulationSize, 1.25);
			do
			Parent2 = LinearSelection(PopulationSize, 1.25);
			while (Parent2 == Parent1);
			ApplyCrossover(Parent1, Parent2);
			N = FirstNode;
			do {
				if (ProblemType != HCP && ProblemType != HPP) {
					int d = C(N, N->Suc);
					AddCandidate(N, N->Suc, d, INT_MAX);
					AddCandidate(N->Suc, N, d, INT_MAX);
				}
				N = N->InitialSuc = N->Suc;
			} while (N != FirstNode);
		}
		SRandom(++Seed);
	}
	//	PrintStatistics();

	for (int i = 0; i < TSP_N; i++)
	{
		TSP_RESULT[i] = BestTour[i] - 1;
	}
	//		printf("%d -¡·",BestTour[i]-1);

	return BestCost;
}
