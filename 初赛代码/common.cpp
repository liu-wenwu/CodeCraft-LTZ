#include "common.h"
#include "lib_record.h"
#include <time.h>
#include <string.h>
#include <algorithm>


using namespace std;




//--------------------------------------------

int gGraphWeight[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };

int gGraphEdge[MAX_VERTEX_NUM][MAX_VERTEX_NUM];


int gGraphSFPA[MAX_VERTEX_NUM][MAX_VERTEX_NUM];

int gGraphVsetPath[MAX_VERTEX_NUM][MAX_VERTEX_NUM];

vector<int> gMap[MAX_VERTEX_NUM];


//每个顶点的类型
VertexType gVertexType[MAX_VERTEX_NUM];

VertexType gVertexTypeLKH[MAX_VERTEX_NUM];


int gVertexNum = 0;

int gEdgeNum = 0;

//记录V子集
int gVertexsVSet[MAX_SETS_NUM];
//V子集点个数
int gVertexNumVSet = 0;
//V子集边数
int gEdgeNumVSet = 0;
//起点
int gVertexStart;
//终点
int gVertexEnd;



int gVertexOutputNum[MAX_VERTEX_NUM];//出度
int gVertexInputNum[MAX_VERTEX_NUM];//入度

vector<int> gShortestPath;


//-------------------TSP相关-------------------
int TSP_N;
int TSP_DIST[MAX_TSP_SIZE][MAX_TSP_SIZE];
int TSP_RESULT[MAX_TSP_SIZE];
int TSP_KEYMAP[MAX_TSP_SIZE];

//----------------编号映射---------------------
int gIndexMap[600000] = { 0 };
int gIndexCount = 0;
int gInputIndex[MAX_VERTEX_NUM] = { 0 };


bool gDisableSPFA[MAX_VERTEX_NUM];




//start==0,end==1
int vstartbak = 0;
void putIDStartEnd(int start, int end)
{

	gInputIndex[0] = start;
	gIndexMap[start] = 0;

	gInputIndex[1] = end;
	gIndexMap[end] = 1;

	gIndexCount = 2;
	vstartbak = start;
}

int putID(int id)
{
	if (id == vstartbak)
		return 0;

	int putid = 0;


	if (gIndexMap[id] != 0)
	{
		putid = gIndexMap[id];
	}
	else
	{
		putid = gIndexCount;
		gInputIndex[putid] = id;
		gIndexMap[id] = putid;

		gIndexCount++;
	}

	return putid;

}

//////////////////////////////


void rand_reset()
{
	srand(time(NULL));
}



//int TSP_OUTPUT[MAX_VERTEX_NUM];
//int TSP_INPUT[MAX_VERTEX_NUM];

int vset_keymap[600];
int map_count = 1;

int put_tsp_id(int idx)
{
	int id = 0;
	if (vset_keymap[idx] == -1)
	{
		TSP_KEYMAP[map_count] = idx;
		id = map_count;
		vset_keymap[idx] = id;
		map_count++;

	}
	else id = vset_keymap[idx];
	
	return id;
}


bool init_tsp()
{

//	memcpy(gGraphSFPA, gGraphWeight, sizeof(gGraphWeight));
//	memset(gGraphVsetPath, -1, sizeof(gGraphVsetPath));
	//除去FREE的所有的最短路
	for (int k = 0; k < gVertexNum; k++)
	{
		if (gVertexTypeLKH[k] != VT_FREE)
		{
			spfa_allpair(gGraphWeight, gVertexNum, gMap, k,
				gDisableSPFA, gGraphSFPA[k], gGraphVsetPath[k]);
		}
	}


	TSP_N = 1;
	for (int i = 0; i < MAX_VERTEX_NUM; i++)
	if (gVertexTypeLKH[i] == VT_VSET)
	{
		TSP_N++;
	}

	//printf("TSP_N:%d\n", TSP_N);

	memset(TSP_DIST, INFW, sizeof(TSP_DIST));

	for (int vi = 0; vi < gVertexNum; vi++)
	{

		//自由点
		if (gVertexTypeLKH[vi] == VT_NONE || gVertexTypeLKH[vi] == VT_FREE) continue;

		for (int vj = 0; vj < gVertexNum; vj++)
		{
			if (vi == vj) continue;

			if (gVertexTypeLKH[vj] == VT_NONE || gVertexTypeLKH[vj] == VT_FREE) continue;


			if (gGraphSFPA[vi][vj] < INFW)
			{
				if (vi == gVertexStart && vj == gVertexEnd)
					continue;

				int ss = put_tsp_id(vi);
				int tt = put_tsp_id(vj);

				TSP_DIST[ss][tt] = gGraphSFPA[vi][vj];
			}

		}
	}


	return true;
}





int getOnePathDijkstra(int start, int end, std::vector<int> &path, bool used[MAX_VERTEX_NUM])
{

	int vpath[MAX_VERTEX_NUM];
	memset(vpath, -1, sizeof(vpath));
	int dis = dijkstra_binheap(gGraphWeight, gVertexNum, start, end, used, vpath);

	if (dis >= INFW)
		return INFW;

	int pos = end;
	while (true)
	{
		pos = vpath[pos];
		if (pos == start) break;
		path.push_back(pos);
	}

	std::reverse(path.begin(), path.end());



	return dis;
}


int getOnePathSPFA(int start, int end, std::vector<int> &path, bool used[MAX_VERTEX_NUM])
{

	int vpath[MAX_VERTEX_NUM];
	memset(vpath, -1, sizeof(vpath));
	int dis = spfa(gGraphWeight, gVertexNum, gMap, start, end, used, vpath);

	if (dis >= INFW)
		return INFW;

	int pos = end;
	while (true)
	{
		pos = vpath[pos];
		if (pos == start) break;
		path.push_back(pos);
	}

	std::reverse(path.begin(), path.end());



	return dis;
}



int get_all_path(std::vector<int> &path)
{
	
	vector<vector<int> > segment(path.size());

	for (int i = 0; i < path.size() - 1; i++)
	{
		vector<int> apth;

		int start = path[i];
		int end = path[i+1];
		//printf("%d -> %d\n", start,end);
		int pos = end;
		while (pos!=start)
		{
			pos = gGraphVsetPath[start][pos];
			if (pos != start)
				apth.push_back(pos);
		}

		std::reverse(apth.begin(), apth.end());
		segment[i] = apth;

	}


	std::vector<int> lastpath;
	lastpath.clear();

	for (int i = 0; i < path.size() - 1; i++)
	{

		lastpath.push_back(path[i]);
		lastpath.insert(lastpath.end(), segment[i].begin(), segment[i].end());
	}
	lastpath.push_back(gVertexEnd);



	gShortestPath = lastpath;

	int weight = 0;
	for (int k = 0; k < lastpath.size() - 1; k++)
	{
		if (gGraphWeight[lastpath[k]][lastpath[k + 1]] >= INFW)
		{
			return INFW;
		}

		weight += gGraphWeight[lastpath[k]][lastpath[k + 1]];

	}
	return weight;


	return 0;

}





//----------------------------------------
//新算法
int all_path_usedcount[MAX_VERTEX_NUM];

bool get_all_path_new(std::vector<int> &path,int &weight)
{

	memset(all_path_usedcount, 0, sizeof(all_path_usedcount));

	//vector<vector<int> > segment(path.size());
	//
	//vector<bool> flag(path.size());

	//vector<int> needAdjustIndex;

	 weight = 0;
	for (int i = 0; i < path.size() - 1; i++)
	{
		
		int start = path[i];
		int end = path[i + 1];

		vector<int> tpath;

		int pos = gGraphVsetPath[start][end];

	

		while (pos != start)
		{
			tpath.push_back(pos);
	
			pos = gGraphVsetPath[start][pos];
		}

		


		for (int j = 0; j < tpath.size(); j++)
		{
			all_path_usedcount[tpath[j]] ++;
		}


		std::reverse(tpath.begin(), tpath.end());
		vector<int> tmppath;
		tmppath.push_back(start);
		tmppath.insert(tmppath.end(), tpath.begin(), tpath.end());
		tmppath.push_back(end);

		int w = 0;
		for (int j = 0; j < tmppath.size() - 1; j++)
		{
			w += gGraphWeight[tmppath[j]][tmppath[j + 1]];
		}


		weight += w;

		
	}

	bool needJx = false;
	for (int i = 0; i < MAX_VERTEX_NUM; i++)
	{
		if (all_path_usedcount[i] >1)
		{
			if (gVertexTypeLKH[i] != VT_VSET)
			{
				gVertexTypeLKH[i] = VT_VSET;
				needJx = true;
				gDisableSPFA[i] = true;

			}
				
		}
			
	}


	return !needJx;

}





int get_tsp_path(vector<int> &path)
{

	if (!path.empty())
		path.clear();

	int zeroindex = 0;
	for (int i = 0; i < TSP_N; i++)
	{
		if (TSP_RESULT[i] == 0)
		{
			zeroindex = i;
			break;
		}
	}

	path.push_back(gVertexStart);
	int i = (zeroindex + 1) % (TSP_N);

	while (path.size() != TSP_N)
	{
		path.push_back(TSP_KEYMAP[TSP_RESULT[i]]);
		i = (i + 1) % (TSP_N);
	}
	path.push_back(gVertexEnd);

	int totalW = 0;
	for (int i = 0; i < path.size() - 1; i++)
	{

		totalW += gGraphWeight[path[i]][path[i + 1]];
	}

	return totalW;
}






bool sovle_by_lkh(int &weight)
{



	int dis = tsp_lkh();

	if (dis >= INFW)
	{
		weight = INFW;
		return true;
	}



	vector<int> tsp_path;
	get_tsp_path(tsp_path);

	int tweight = 0;
	bool sloved = get_all_path_new(tsp_path, tweight);
	gShortestPath = tsp_path;
	weight = tweight;
	return sloved;
}




//由TSP_RESULT得到vset路径
int vsetused[MAX_VERTEX_NUM];
int edgeused[5000];

bool write_result()
{

	/*
		printf("\n==========	result	===========\n");
		int weight = 0;
		for (int i = 0; i < gShortestPath.size(); i++)
		{
			printf("%d -> ",gInputIndex[gShortestPath[i] ]);

			vsetused[gShortestPath[i]] ++;
			if (i < gShortestPath.size()-1)
				weight += gGraphWeight[gShortestPath[i]][gShortestPath[i+1]];


		}
		printf("\n=====================  %d\n", weight);
		for (int i = 0; i < gShortestPath.size() - 1; i++)
		{
		//	printf("%d -> ", gGraphEdge[gShortestPath[i]][gShortestPath[i + 1]]);

			edgeused[gGraphEdge[gShortestPath[i]][gShortestPath[i + 1]]] ++;
		}
		//printf("\n=====================\n");

		for (int i = 0; i<gVertexNumVSet; i++)
		{
			if (vsetused[gVertexsVSet[i]] != 1)
			{
				return false;
			}
		}

		for (int i = 0; i<5000; i++)
		{
			if (edgeused[i] > 1)
			{
				return false;
			}
		}
		*/

		if (gShortestPath.size()<2)
			return false;


		for (int i = 0; i < gShortestPath.size() - 1; i++)
			record_result(gGraphEdge[gShortestPath[i]][gShortestPath[i + 1]]);

	
		return true;
}
