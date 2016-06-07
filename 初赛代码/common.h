#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <unordered_map>

using namespace std;

#define MAX_VERTEX_NUM 600//总点数
#define MAX_SETS_NUM 50//必经点个数
#define MAX_VERTEX_OUTPUT 8//出度

#define INFW 0x3f3f3f3f //权值无穷大


enum VertexType{
	VT_NONE,//无效点
	VT_FREE,//自由点
	VT_START,//起始点
	VT_END,//结束点
	VT_VSET//必经点
};


#define MP

#define MAX_TSP_SIZE  (600 + 1)



extern int gGraphWeight[MAX_VERTEX_NUM][MAX_VERTEX_NUM];

extern int gGraphEdge[MAX_VERTEX_NUM][MAX_VERTEX_NUM];



extern int gGraphSFPA[MAX_VERTEX_NUM][MAX_VERTEX_NUM];


extern vector<int> gMap[MAX_VERTEX_NUM];

extern int gFloydPath[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
//每个顶点的类型
extern VertexType gVertexType[MAX_VERTEX_NUM];

extern VertexType gVertexTypeLKH[MAX_VERTEX_NUM];

extern int gVertexNum ;

extern int gEdgeNum ;

//记录V子集
extern int gVertexsVSet[MAX_SETS_NUM];
//V子集点个数
extern int gVertexNumVSet ;
//V子集边数
extern int gEdgeNumVSet ;
//起点
extern int gVertexStart;
//终点
extern int gVertexEnd;



extern int gVertexOutputNum[MAX_VERTEX_NUM];//出度
extern int gVertexInputNum[MAX_VERTEX_NUM];//入度

extern vector<int> gShortestPath;


//-------------------TSP相关-------------------
extern int TSP_N;
extern int TSP_DIST[MAX_TSP_SIZE][MAX_TSP_SIZE];
extern int TSP_RESULT[MAX_TSP_SIZE];
extern int TSP_KEYMAP[MAX_TSP_SIZE];

//----------------编号映射---------------------
extern int gIndexMap[600000] ;
extern int gIndexCount ;
extern int gInputIndex[MAX_VERTEX_NUM];

extern bool gDisableSPFA[MAX_VERTEX_NUM];


extern int vset_keymap[600];

void putIDStartEnd(int start, int end);
int putID(int id);


void rand_reset();
bool init_tsp();
bool init_for_tsp();


bool sovle_by_lkh(int &weight);



int tsp_lkh();//LKH算法


bool write_result();
int get_all_path(std::vector<int> &path);




int dijkstra_binheap(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, int source, int end, int path[]);
int dijkstra_binheap(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, int source, int end, bool disable[], int path[]);


int spfa(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, std::vector<int> gMap[MAX_VERTEX_NUM], int start, int end, bool disable[], int path[]);

void spfa_allpair(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, std::vector<int> gMap[MAX_VERTEX_NUM], int start,
	bool disable[MAX_VERTEX_NUM], int dist[MAX_VERTEX_NUM], int path[]);



void timer_start();


#ifdef WIN32
double timer_elapse();
double timer_elapse_print(const char * head);
#else
unsigned long timer_elapse();
#endif


#endif

