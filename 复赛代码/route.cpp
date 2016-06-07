#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include "tsp_solver.h"
#include "lib_time.h"
#include <sys/timeb.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
LARGE_INTEGER startCount;
LARGE_INTEGER endCount;
LARGE_INTEGER freq;
void timer_start()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startCount);
}
//ms
double timer_elapse()
{
	QueryPerformanceCounter(&endCount);
	double elapsed = (double)(endCount.QuadPart - startCount.QuadPart) / freq.QuadPart * 1000;
	return elapsed;
}
#else
static int ms = 0;
static unsigned long s = 0;
void timer_start()
{
	struct timeb rawtime;
	ftime(&rawtime);
	ms = rawtime.millitm;
	s = rawtime.time;
}
//ms
double timer_elapse()
{
	struct timeb rawtime;
	ftime(&rawtime);
	int out_ms = rawtime.millitm - ms;
	unsigned long out_s = rawtime.time - s;
	return out_ms + out_s * 1000;
}
#endif
#define MAX_VERTEX_NUM 2000*2 //真实点+虚拟点
const int g_vertex_start = 0;
const int g_vertex_end = 1;
int g_vid_count = g_vertex_end + 1;
int g_ori_vertex_start = -1;
int g_ori_vertex_end = -1;
int g_vertex_true_num = 0;
int g_graph_newtoold_idmap[MAX_VERTEX_NUM] = { 0 };//新id -> 原id
int g_graph_oldtonew_idmap[MAX_VERTEX_NUM] = { 0 };//原id -> 新id
//主边
int g_graph_weight[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
int g_graph_edgeid[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
//备用边
int g_graph_backup_weight[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
int g_graph_backup_edgeid[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
std::vector<int> g_graph_backup_index;
VertexType g_vertex_type[MAX_VERTEX_NUM] = { VT_START, VT_END, VT_FREE };
int g_vertex_setp[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };
int g_vertex_setp_num[MAX_DEMAND_NUM] = { 0 };
std::vector<int> g_input_map[MAX_VERTEX_NUM];//入度图
std::vector<int> g_output_map[MAX_VERTEX_NUM];//出度图
int g_graph_spfa_dist[MAX_DEMAND_NUM][MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
int g_graph_spfa_path[MAX_DEMAND_NUM][MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
bool g_graph_spfa_ignore[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };
int g_tsp_segment_dist[MAX_DEMAND_NUM][MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
int g_tsp_segment_path[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };
int g_tsp_segment_n[MAX_DEMAND_NUM];
std::vector<int> g_tsp_segment[MAX_DEMAND_NUM][MAX_VERTEX_NUM];
int g_tsp_dist[MAX_DEMAND_NUM][MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
int g_tsp_path[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };
int g_tsp_n[MAX_DEMAND_NUM];
std::vector<int> g_tsp_setp_path[MAX_DEMAND_NUM];
int g_tsp_newtoold_idmap[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };//新id -> 原id
int g_tsp_oldtonew_idmap[MAX_DEMAND_NUM][MAX_VERTEX_NUM] = { 0 };//原id -> 新id
int g_tsp_id_cnt[MAX_DEMAND_NUM] = { 1, 1 };
std::vector<int> g_tsp_input_map[MAX_DEMAND_NUM][MAX_VERTEX_NUM];//入度图
std::vector<int> g_tsp_output_map[MAX_DEMAND_NUM][MAX_VERTEX_NUM];//出度图
std::vector<int> g_setp_input_map[MAX_VERTEX_NUM];
std::vector<int> g_setp_output_map[MAX_VERTEX_NUM];
std::vector<int> g_vertex_result[MAX_DEMAND_NUM];
std::vector<int> g_segment_result[MAX_DEMAND_NUM][MAX_VERTEX_NUM];
int g_vertex_outdegree[MAX_VERTEX_NUM] = { 0 };//出度,不计算重边
int g_vertex_indegree[MAX_VERTEX_NUM] = { 0 };//入度,不计算重边
int g_vertex_type_mask[MAX_VERTEX_NUM] = { 0 };
#define CLEAR_SETP(v) {g_vertex_type_mask[v]=0;}
#define ADD_SETP_VERTEX(v,p) {g_vertex_type_mask[v]|=1<<p;}
#define REMOVE_SETP_VERTEX(v,p) {g_vertex_type_mask[v]^=1<<p;}
#define IS_SETP_VERTEX(v,p) (g_vertex_type_mask[v] & (1 << p))
//必经边
int g_edge_setp_mask[MAX_EDGE_NUM + 1] = { 0 };
int g_edge_to_vertex[MAX_EDGE_NUM + 1] = { 0 };
#define RESET_EDGE_SETP(e) {g_edge_setp_mask[e]=0;}
#define SET_EDGE_SETP(e,p) {g_edge_setp_mask[e]=1<<p;}
#define ADD_EDGE_SETP(e,p) {g_edge_setp_mask[e]|=1<<p;}
#define IS_EDGE_SETP(e,p) (g_edge_setp_mask[e] & (1 << p))
#define IS_EDGE_OTHER_SETP(e,p) (g_edge_setp_mask[e] & (~(1 << p)))
bool g_virtual_vertex_added[MAX_VERTEX_NUM][MAX_VERTEX_NUM] = { 0 };
bool g_path_ok[MAX_DEMAND_NUM] = { 0, 0 };
void set_graph_startend_vid(int start, int end)
{
	g_graph_newtoold_idmap[g_vertex_start] = start;
	g_graph_newtoold_idmap[g_vertex_end] = end;
	g_graph_oldtonew_idmap[start] = g_vertex_start;
	g_graph_oldtonew_idmap[end] = g_vertex_end;
	g_ori_vertex_start = start;
	g_ori_vertex_end = end;
}
//放入原id，得到转换后id,如果已经放入，直接返回
int put_graph_vid(int vid)
{
	if (vid == g_ori_vertex_start)
		return g_vertex_start;
	if (g_graph_oldtonew_idmap[vid] > 0)
		return g_graph_oldtonew_idmap[vid];
	else
	{
		g_graph_newtoold_idmap[g_vid_count] = vid;
		g_graph_oldtonew_idmap[vid] = g_vid_count;
		++g_vid_count;
		return  g_graph_oldtonew_idmap[vid];
	}
}
int spfa(int k, int start, int end, bool ignore[MAX_VERTEX_NUM], int path[])
{
	int d[MAX_VERTEX_NUM];
	bool used[MAX_VERTEX_NUM];
	std::queue<int>q;
	q.push(start);
	memcpy(used, ignore, sizeof(used));
	int i;
	for (i = 0; i < MAX_VERTEX_NUM; i++)
		d[i] = INFW;
	d[start] = 0;
	while (!q.empty())
	{
		int node = q.front();
		q.pop();
		used[node] = false;
		int t, dis, p;
		for (i = 0; i<g_output_map[node].size(); i++)
		{
			t = g_output_map[node][i];
			int edgeid = g_graph_edgeid[node][t];
			if (!IS_EDGE_SETP(edgeid, k) && IS_EDGE_OTHER_SETP(edgeid, k))//是其他的必经边
				continue;
			dis = g_graph_weight[node][t];
			if (d[t]>d[node] + dis)
			{
				d[t] = d[node] + dis;
				path[t] = node;
				if (!used[t])
				{
					used[t] = true;
					q.push(t);
				}
			}
		}
	}
	return d[end];
}
void spfa(int k, int start, bool ignore[MAX_VERTEX_NUM], int dist[MAX_VERTEX_NUM], int path[MAX_VERTEX_NUM])
{
	int d[MAX_VERTEX_NUM];
	bool used[MAX_VERTEX_NUM];
	std::queue<int>q;
	q.push(start);
	memcpy(used, ignore, sizeof(used));
	memset(path, 0, sizeof(path));
	for (int i = 0; i < MAX_VERTEX_NUM; i++)
		d[i] = INFW;
	d[start] = 0;
	while (!q.empty())
	{
		int node = q.front();
		q.pop();
		used[node] = false;
		int t, dis;
		for (int i = 0; i < g_output_map[node].size(); i++)
		{
			t = g_output_map[node][i];
			//检查边是否可以通过
			int edgeid = g_graph_edgeid[node][t];
			if (!IS_EDGE_SETP(edgeid, k) && IS_EDGE_OTHER_SETP(edgeid, k))//是其他的必经边
				continue;
			dis = g_graph_weight[node][t];
			if (d[t] > d[node] + dis)
			{
				d[t] = d[node] + dis;
				path[t] = node;
				if (!used[t])
				{
					used[t] = true;
					q.push(t);
				}
			}
		}
	}
	memcpy(dist, d, sizeof(d));
	dist[start] = INFW;
	return;
}
//放入原id，得到转换后id,如果已经放入，直接返回
int put_tsp_vid(int vid, int k)
{
	if (vid == g_vertex_start || vid == g_vertex_end)
		return 0;
	if (g_tsp_oldtonew_idmap[k][vid] > 0)
		return g_tsp_oldtonew_idmap[k][vid];
	else
	{
		g_tsp_newtoold_idmap[k][g_tsp_id_cnt[k]] = vid;
		g_tsp_oldtonew_idmap[k][vid] = g_tsp_id_cnt[k];
		++g_tsp_id_cnt[k];
		return  g_tsp_oldtonew_idmap[k][vid];
	}
}
int get_tsp_setp_path(std::vector<int> &path, int k)
{
	if (!path.empty())
		path.clear();
	int zeroindex = 0;
	for (int i = 0; i < g_tsp_n[k]; i++)
	{
		if (g_tsp_path[k][i] == 0)
		{
			zeroindex = i;
			break;
		}
	}
	path.push_back(g_vertex_start);
	int i = (zeroindex + 1) % (g_tsp_n[k]);
	while (path.size() != g_tsp_n[k])
	{
		path.push_back(g_tsp_newtoold_idmap[k][g_tsp_path[k][i]]);
		i = (i + 1) % (g_tsp_n[k]);
	}
	path.push_back(g_vertex_end);
	int dist = 0;
	for (int i = 0; i < path.size() - 1; i++)
	{
		dist += g_graph_spfa_dist[k][path[i]][path[i + 1]];
	}
	return dist;
}
/**************************************************************************************************
由必经点的路径得到全路径
**************************************************************************************************/
bool get_all_path_status(int k, std::vector<int> &inpath, std::vector<int> &outpath)
{
	int all_path_usedcount[MAX_VERTEX_NUM];
	//segment[i]:为path[i]到path[i+1]的路径，如果为空，那么就是直达
	std::vector<int> segment[MAX_VERTEX_NUM];//路径分段
	std::vector<int> path = inpath;
	//得到分段路径和重点
	memset(all_path_usedcount, 0, sizeof(all_path_usedcount));
	for (int i = 0; i < path.size() - 1; i++)
	{
		int start = path[i];
		int end = path[i + 1];
		std::vector<int> tpath;
		int pos = g_graph_spfa_path[k][start][end];
		while (pos != start)
		{
			tpath.push_back(pos);
			pos = g_graph_spfa_path[k][start][pos];
		}
		//统计点重复
		for (int j = 0; j < tpath.size(); j++)
		{
			all_path_usedcount[tpath[j]] ++;
		}
		//反转后才是正确路径
		std::reverse(tpath.begin(), tpath.end());
		segment[i] = tpath;
	}
	//尽可能扩展必经点
	while (true)
	{
		std::vector<int> newpath;
		std::vector<int> newsegment[MAX_VERTEX_NUM];
		int new_cnt = 0;
		bool ok = true;
		for (int i = 0; i < path.size() - 1; i++)
		{
			int start = path[i];
			int end = path[i + 1];
			newpath.push_back(start);//一个节点
			for (int j = 0; j < segment[i].size(); j++)
			{
				if (all_path_usedcount[segment[i][j]]>1)
				{
					bool ignore[MAX_VERTEX_NUM];
					int path[MAX_VERTEX_NUM];
					memcpy(ignore, g_graph_spfa_ignore[k], sizeof(ignore));
					ignore[segment[i][j]] = true;
					int dis = spfa(k, start, end, ignore, path);
					if (dis >= INFW)
					{
						ok = false;
						new_cnt++;
						newpath.push_back(segment[i][j]);
					}
					else
					{
						newsegment[new_cnt].push_back(segment[i][j]);
					}
				}
			}
		}
		if (ok) break;
		newpath.push_back(path[path.size() - 1]);
		path = newpath;
		for (int i = 0; i < MAX_VERTEX_NUM; i++)
			segment[i] = newsegment[i];
	}
	bool newpath_ignore[MAX_VERTEX_NUM];
	memcpy(newpath_ignore, g_graph_spfa_ignore[k], sizeof(newpath_ignore));
	for (int i = 0; i < path.size(); i++)
	{
		newpath_ignore[path[i]] = true;
	}
	for (int i = 0; i < path.size() - 1; i++)
	{
		int start = path[i];
		int end = path[i + 1];
		bool need = false;
		for (int j = 0; j < segment[i].size(); j++)
		{
			if (all_path_usedcount[segment[i][j]]>1)
			{
				need = true;
			}
		}
		if (segment[i].size() == 0 && g_graph_weight[start][end] == 0)
		{
			need = true;
		}
		if (need)
		{
			bool ignore[MAX_VERTEX_NUM];
			int vpath[MAX_VERTEX_NUM];
			memcpy(ignore, g_graph_spfa_ignore[k], sizeof(ignore));
			int dis = spfa(k, start, end, newpath_ignore, vpath);
			std::vector<int> tpath;
			int pos = vpath[end];
			while (pos != start)
			{
				tpath.push_back(pos);
				pos = vpath[pos];
			}
			std::reverse(tpath.begin(), tpath.end());
			segment[i] = tpath;
		}
	}
	memset(all_path_usedcount, 0, sizeof(all_path_usedcount));
	for (int i = 0; i < path.size() - 1; i++)
	{
		all_path_usedcount[path[i]]++;
		for (int j = 0; j < segment[i].size(); j++)
		{
			all_path_usedcount[segment[i][j]]++;
		}
	}
	std::vector<int> allpath;
	for (int i = 0; i < path.size() - 1; i++)
	{
		allpath.push_back(path[i]);
		//printf("%d-->", path[i]);
		g_segment_result[k][i].clear();
		g_segment_result[k][i].push_back(path[i]);
		for (int j = 0; j < segment[i].size(); j++)
		{
			allpath.push_back(segment[i][j]);
			//printf("%d-->", segment[i][j]);
			g_segment_result[k][i].push_back(segment[i][j]);
		}
		g_segment_result[k][i].push_back(path[i + 1]);
	}
	allpath.push_back(1);
	outpath = allpath;
	bool ok = true;
	for (int i = 0; i < MAX_VERTEX_NUM; i++)
	{
		if (all_path_usedcount[i] >1)
		{
			if (!IS_SETP_VERTEX(i, k))
			{
				ok = false;
				g_vertex_type[i] = VT_SETP;
				ADD_SETP_VERTEX(i, k);
			}
		}
	}
	return ok;
}
/**************************************************************************************************
1、初始化
**************************************************************************************************/
bool init_first(char *graph[MAX_EDGE_NUM], int edge_num, char *condition[MAX_DEMAND_NUM], int demand_num)
{
	//解析demand.csv
	int index, startv, endv;
	for (int k = 0; k < demand_num; k++)
	{
		char *demand_str = condition[k];
		sscanf(demand_str, "%d,%d,%d", &index, &startv, &endv);
		if (index != k + 1) //检查序号合法？
			return false;
		if (g_ori_vertex_start != -1 && (g_ori_vertex_start != startv || g_ori_vertex_end != endv)) //检查是起点终点一致？
			return false;
		set_graph_startend_vid(startv, endv);
		int commma_cnt = 0;//逗号计数
		int vline_cnt = 1;//竖线计数
		int vline_idx[MAX_SETS_NUM];//第一个索引是第三个逗号，之后是竖线
		for (unsigned int i = 0; i < strlen(demand_str); i++)
		{
			if (demand_str[i] == ',')
			{
				commma_cnt++;
				if (commma_cnt == 3)
					vline_idx[0] = i;
			}
			else if (demand_str[i] == '|'){
				vline_idx[vline_cnt] = i;
				vline_cnt++;
			}
		}
		//得到了必经点个数
		g_vertex_setp_num[k] = vline_cnt;
		if (vline_cnt == 1)
		{
			char *str = demand_str + vline_idx[0] + 1;
			if (str[0] == 'N' && str[1] == 'A')
			{
				g_vertex_setp_num[k] = 0;
			}
		}
		for (unsigned int i = 0; i < g_vertex_setp_num[k]; i++)
		{
			sscanf(demand_str + vline_idx[i] + 1, "%d", &g_vertex_setp[k][i]);
			g_vertex_setp[k][i] = put_graph_vid(g_vertex_setp[k][i]);
			g_vertex_type[g_vertex_setp[k][i]] = VT_SETP;//设置类型：必经点
			ADD_SETP_VERTEX(g_vertex_setp[k][i], k);//设置必经点所属点集
		}
	}
	//解析topo.csv
	int edge, start, end, weight;
	for (int i = 0; i < edge_num; i++)
	{
		sscanf(graph[i], "%d,%d,%d,%d", &edge, &start, &end, &weight);
		weight++;//这里很重要。。
		edge++;
		if (start == end)//去除输入的环
			continue;
		start = put_graph_vid(start);
		end = put_graph_vid(end);
		//忽略终点的出边和终点的入边
		if (start == g_vertex_end || end == g_vertex_start)
			continue;
		g_edge_to_vertex[edge] = start << 16 | end;
		//主边未启用时将这个边设置为主边
		if (g_graph_weight[start][end] == 0)
		{
			g_graph_edgeid[start][end] = edge;
			g_graph_weight[start][end] = weight;
			g_output_map[start].push_back(end);
			g_input_map[end].push_back(start);
			g_vertex_outdegree[start]++;
			g_vertex_indegree[end]++;
		}
		else //主边启用时，比较权值
		{
			//权值比主边小，主边使用这个边
			if (weight < g_graph_weight[start][end])
			{
				g_graph_backup_weight[start][end] = g_graph_weight[start][end];
				g_graph_backup_edgeid[start][end] = g_graph_edgeid[start][end];
				g_graph_edgeid[start][end] = edge;
				g_graph_weight[start][end] = weight;
				g_graph_backup_index.push_back((start << 16) | end);
			}
			else //备用边选择
			{
				if (weight < g_graph_backup_weight[start][end])
				{
					g_graph_backup_weight[start][end] = weight;
					g_graph_backup_edgeid[start][end] = edge;
				}
			}//else
		}//else
	}//while
	//真实点集个数，因为后面要加虚拟点，虚拟点个数：g_vid_count-g_vertex_true_num
	g_vertex_true_num = g_vid_count;
	//检查起点、终点、必经点集的出入点，初步判断是否无解
	return true;
}
/**************************************************************************************************
2、删除孤立点（包括出度或入度有一个为0的点、出入度为1但出入指向同一点的点。当然处理范围不包括起始点）
**************************************************************************************************/
bool delete_isolated_vertexs()
{
	while (true)
	{
		int find_iso = false;
		std::stack<int> vertexs;
		for (int i = 2; i < g_vertex_true_num; i++)//排除起始点
		{
			if ((g_vertex_outdegree[i] == 1 && g_vertex_indegree[i] == 0) ||
				(g_vertex_outdegree[i] == 0 && g_vertex_indegree[i] == 1))
			{
				vertexs.push(i);
				find_iso = true;
			}
			if (g_vertex_outdegree[i] == 1 && g_vertex_indegree[i] == 1)
			{
				if (g_input_map[i].at(0) == g_output_map[i].at(0))
				{
					vertexs.push(i);
					find_iso = true;
				}
			}
		}
		if (!find_iso) break;
		int iso_count = 0;
		while (!vertexs.empty())
		{
			int vp = vertexs.top();
			vertexs.pop();
			iso_count++;
			//1、入度为1，出度为0
			if (g_vertex_outdegree[vp] == 0 && g_vertex_indegree[vp] == 1)
			{
				for (int i = 0; i < g_input_map[vp].size(); i++)
				{
					int vin = g_input_map[vp][i];
					if (g_vertex_type[vin] != VT_START && g_vertex_type[vin] != VT_END)
						vertexs.push(vin);//新的点入栈
					//删去这条边
					g_graph_weight[vin][vp] = 0;
					g_graph_edgeid[vin][vp] = 0;
					//更新入点的出度
					std::vector<int> newoutmap;
					for (int j = 0; j < g_output_map[vin].size(); j++)
					{
						if (g_output_map[vin][j] != vp)
							newoutmap.push_back(g_output_map[vin][j]);
					}
					g_output_map[vin] = newoutmap;
					g_vertex_outdegree[vin]--;
				}//for
				//这个点的入度清零
				g_input_map[vp].clear();
				g_vertex_indegree[vp] = 0;
			}//1
			//2、入度为0，出度为1
			if (g_vertex_indegree[vp] == 0 && g_vertex_outdegree[vp] == 1)
			{
				for (int i = 0; i < g_output_map[vp].size(); i++)
				{
					int vout = g_output_map[vp][i];
					if (g_vertex_type[vout] != VT_START && g_vertex_type[vout] != VT_END)
						vertexs.push(vout);
					g_graph_weight[vout][vp] = 0;
					g_graph_edgeid[vout][vp] = 0;
					std::vector<int> newinmap;
					for (int j = 0; j < g_input_map[vout].size(); j++)
					{
						if (g_output_map[vout][j] != vp)
							newinmap.push_back(g_output_map[vout][j]);
					}
					g_input_map[vout] = newinmap;
					g_vertex_indegree[vout]--;
				}
				g_output_map[vp].clear();
				g_vertex_outdegree[vp] = 0;
			}//2
			//3、入度为1，出度为1，,出入为同一点
			if (g_vertex_outdegree[vp] == 1 && g_vertex_indegree[vp] == 1 &&
				(g_input_map[vp].at(0) == g_output_map[vp].at(0)))
			{
				//处理入点
				int vin = g_input_map[vp][0];
				vertexs.push(vin);
				g_graph_weight[vin][vp] = 0;
				g_graph_edgeid[vin][vp] = 0;
				std::vector<int> newoutmap;
				for (int j = 0; j < g_output_map[vin].size(); j++)
				{
					if (g_output_map[vin][j] != vp)
						newoutmap.push_back(g_output_map[vin][j]);
				}
				g_output_map[vin] = newoutmap;
				g_input_map[vp].clear();
				g_vertex_indegree[vp] = 0;
				g_vertex_outdegree[vin]--;
				//处理出点
				int vout = g_output_map[vp][0];
				vertexs.push(vout);
				g_graph_weight[vout][vp] = 0;
				g_graph_edgeid[vout][vp] = 0;
				std::vector<int> newinmap;
				for (int j = 0; j < g_input_map[vout].size(); j++)
				{
					if (g_output_map[vout][j] != vp)
						newinmap.push_back(g_output_map[vout][j]);
				}
				g_input_map[vout] = newinmap;
				g_output_map[vp].clear();
				g_vertex_outdegree[vp] = 0;
				g_vertex_indegree[vout]--;
			}//3
		}//while
	}//while
	return true;
}
/**************************************************************************************************
3、扩展必经点集，根据出入度
**************************************************************************************************/
bool extend_setp_vertexs()
{
	for (int k = 0; k < MAX_DEMAND_NUM; k++)
	{
		int cnt = 0;
		//根据出入度初始化栈
		std::stack<int> vertexs;
		for (int i = 0; i < g_vertex_setp_num[k]; i++)
		{
			if (g_vertex_indegree[g_vertex_setp[k][i]] == 1 || g_vertex_outdegree[g_vertex_setp[k][i]] == 1)
				vertexs.push(g_vertex_setp[k][i]);
		}
		//访问标记，以访问过的点不在重复访问
		bool visited[MAX_VERTEX_NUM];
		memset(visited, false, sizeof(visited));
		//由必经点向外扩展
		while (!vertexs.empty())
		{
			int vp = vertexs.top();
			vertexs.pop();
			if (visited[vp])
				continue;
			visited[vp] = true;
			if (g_vertex_indegree[vp] == 1 && g_vertex_outdegree[vp] == 1)
			{
				continue;
			}
			//入度为1
			if (g_vertex_indegree[vp] == 1)
			{
				int vfrom = g_input_map[vp][0];
				//这个点是自由点或不是本点集的必经点
				if (g_vertex_type[vfrom] != VT_SETP || (g_vertex_type[vfrom] == VT_SETP && !IS_SETP_VERTEX(vfrom, k)))
				{
					//如果入点是起点，就不加入了
					if (g_vertex_type[vfrom] != VT_START)
					{
						//将这个点加入该点集
						g_vertex_type[vfrom] = VT_SETP;
						g_vertex_setp[k][g_vertex_setp_num[k]] = vfrom;
						++g_vertex_setp_num[k];
						vertexs.push(vfrom);
						ADD_SETP_VERTEX(vfrom, k);
						cnt++;
					}
					//如果没有重边，这个就是必经边
					if (g_graph_backup_edgeid[vfrom][vp] == 0)
					{
						ADD_EDGE_SETP(g_graph_edgeid[vfrom][vp], k);
					}
				}
			}
			//出度为1
			if (g_vertex_outdegree[vp] == 1)
			{
				int vto = g_output_map[vp].at(0);
				if (g_vertex_type[vto] != VT_SETP || (g_vertex_type[vto] == VT_SETP && !IS_SETP_VERTEX(vto, k)))
				{
					if (g_vertex_type[vto] != VT_END)
					{
						g_vertex_type[vto] = VT_SETP;
						g_vertex_setp[k][g_vertex_setp_num[k]] = vto;
						++g_vertex_setp_num[k];
						vertexs.push(vto);
						ADD_SETP_VERTEX(vto, k);
						cnt++;
					}
					if (g_graph_backup_edgeid[vp][vto] == 0)
					{
						ADD_EDGE_SETP(g_graph_edgeid[vp][vto], k);
					}
				}
			}
		}//while
		//printf("%d 添加虚拟点数：%d\n", k, cnt);
	}//for
	return true;
}
/**************************************************************************************************
4、加虚拟点，去除重边，在最后结果处理，要特别注意虚拟点
**************************************************************************************************/
bool add_virtual_vertexs()
{
	//去重边，加虚拟点
	for (int i = 0; i < g_graph_backup_index.size(); i++)
	{
		int start = g_graph_backup_index.at(i) >> 16;
		int end = g_graph_backup_index.at(i) & 0xffff;
		if (g_virtual_vertex_added[start][end])
			continue;
		g_virtual_vertex_added[start][end] = true;
		if (g_graph_backup_weight[start][end] == 0)
			continue;
		//加点，要加入的id
		int newid = g_vid_count;
		g_graph_weight[start][newid] = g_graph_backup_weight[start][end] - 1;
		g_graph_weight[newid][end] = 1;
		g_output_map[start].push_back(newid);
		g_input_map[newid].push_back(start);
		g_output_map[newid].push_back(end);
		g_input_map[end].push_back(newid);
		//记录这个虚拟点对应的备用边索引，非常重要
		//更新出入度
		//g_vertex_outdegree[start]++;
		//g_vertex_indegree[end]++;
		//g_vertex_outdegree[newid] = 1;
		//g_vertex_indegree[newid] = 1;
		g_vid_count++;
	}
	return true;
}
/**************************************************************************************************
5、求解TSP
**************************************************************************************************/
int my_edge_cnt[MAX_DEMAND_NUM][MAX_EDGE_NUM + 1];
int chongbian_index[MAX_EDGE_NUM][MAX_DEMAND_NUM][3];
void solve_by_tsp()
{
	timer_start();
	//迭代计数
	int iter_count[MAX_DEMAND_NUM] = { 1, 1 };
	while (true)
	{
		for (int k = 0; k < MAX_DEMAND_NUM; ++k)
		{
			if (g_path_ok[k])
				continue;
			while (true)
			{
			//	printf("路径:%d 第%d次 迭代 time:%f ms\n", k, iter_count[k], timer_elapse());
				iter_count[k]++;
				//初始化TSP出入度图
				for (int i = 0; i < MAX_VERTEX_NUM; i++)
				{
					if (!g_tsp_input_map[k][i].empty())g_tsp_input_map[k][i].clear();
					if (!g_tsp_output_map[k][i].empty())g_tsp_output_map[k][i].clear();
				}
				//初始化SPFA搜索禁忌点
				for (int i = 0; i < g_vertex_true_num; i++)
				{
					if (g_vertex_type[i] == VT_START || g_vertex_type[i] == VT_END ||
						(g_vertex_type[i] == VT_SETP && IS_SETP_VERTEX(i, k)))
						g_graph_spfa_ignore[k][i] = true;
				}
				//SPFA求禁忌点两两最短路
				for (int i = 0; i < g_vertex_true_num; i++)
				{
					if (g_vertex_type[i] == VT_START || (g_vertex_type[i] == VT_SETP && IS_SETP_VERTEX(i, k)))
					{
						spfa(k, i, g_graph_spfa_ignore[k], g_graph_spfa_dist[k][i], g_graph_spfa_path[k][i]);
					}
				}
				//初始化TSP规模N
				g_tsp_n[k] = 1;
				for (int i = 0; i < g_vertex_true_num; ++i)
				{
					if (g_vertex_type[i] == VT_SETP && IS_SETP_VERTEX(i, k))
						++g_tsp_n[k];
				}
				//初始化TSP图，注意这里将id再一次进行了转换
				for (int vi = 0; vi < g_vertex_true_num; ++vi)
				{
					if (g_vertex_type[vi] == VT_START || (g_vertex_type[vi] == VT_SETP && IS_SETP_VERTEX(vi, k)))
					{
						for (int vj = 0; vj < g_vertex_true_num; ++vj)
						{
							if (g_vertex_type[vj] == VT_END || (g_vertex_type[vj] == VT_SETP && IS_SETP_VERTEX(vj, k)))
							{
								int start = put_tsp_vid(vi, k);
								int end = put_tsp_vid(vj, k);
								//printf("%d->%d  =>  %d->%d\n",vi,vj,start,end);
								if (start == end)
									g_tsp_dist[k][start][end] = INFW;
								else
								{
									g_tsp_dist[k][start][end] = g_graph_spfa_dist[k][vi][vj];
									if (g_tsp_dist[k][start][end] < INFW)
									{
										g_tsp_output_map[k][start].push_back(end);
										g_tsp_input_map[k][end].push_back(start);
									}
								}
							}//if
						}//for
					}//if
				}//for
				//对TSP图预处理：过程如下，如果只有B点到达A点，
				//那么删除所有B出去的边（不包括B->A），删除所有A进来的边(不包括B->A）
				//直到没有边可以删除。
				bool input_visited[MAX_VERTEX_NUM];
				bool output_visited[MAX_VERTEX_NUM];
				memset(input_visited, false, sizeof(input_visited));
				memset(output_visited, false, sizeof(output_visited));
				//
				while (true)
				{
					bool ok = true;
					for (int i = 0; i < g_tsp_n[k]; i++)
					{
						if (g_tsp_output_map[k][i].size() == 1 && g_tsp_input_map[k][i].size() == 1)
							continue;
						if (g_tsp_output_map[k][i].size() == 1 && !output_visited[i])
						{
							output_visited[i] = true;
							ok = false;
							int out = g_tsp_output_map[k][i][0];
							//printf("out %d -> %d\n", i, out);
							//点out，只保留从i的边
							for (int j = 0; j < g_tsp_n[k]; j++)
							{
								if (j != i)
								{
									g_tsp_dist[k][j][out] = INFW;
									std::vector<int> newout;
									for (int a = 0; a < g_tsp_output_map[k][j].size(); a++)
									{
										if (g_tsp_output_map[k][j][a] != out)
											newout.push_back(g_tsp_output_map[k][j][a]);
									}
									g_tsp_output_map[k][j] = newout;
								}
							}
							g_tsp_input_map[k][out].clear();
							g_tsp_input_map[k][out].push_back(i);
						}
						if (g_tsp_input_map[k][i].size() == 1 && !input_visited[i])
						{
							input_visited[i] = true;
							ok = false;
							int in = g_tsp_input_map[k][i][0];
							//printf("in %d -> %d\n", in, i);
							//只能in到i,其他到i的入度也要删掉
							//点in，只保留到i的边
							for (int j = 0; j < g_tsp_n[k]; j++)
							{
								if (j != i)
								{
									g_tsp_dist[k][in][j] = INFW;
									std::vector<int> newin;
									for (int a = 0; a < g_tsp_input_map[k][j].size(); a++)
									{
										if (g_tsp_input_map[k][j][a] != in)
											newin.push_back(g_tsp_input_map[k][j][a]);
									}
									g_tsp_input_map[k][j] = newin;
								}
							}
							g_tsp_output_map[k][in].clear();
							g_tsp_output_map[k][in].push_back(i);
						}
					}
					if (ok) break;
				}
				//------------------------------------------------------
				/*
				int tsp_segment_n = 0;
				bool visited[MAX_VERTEX_NUM];
				memset(visited, false, sizeof(visited));
				for (int i = 0; i < g_tsp_n[k]; i++)
				{
					if (visited[i]) continue;
					if (g_tsp_output_map[k][i].size() == 1 && g_tsp_input_map[k][i].size() > 1)
					{
						printf("%d	%d: ", tsp_segment_n, i);
						g_tsp_segment[k][tsp_segment_n].clear();
						visited[i] = true;
						int idx = i;
						g_tsp_segment[k][tsp_segment_n].push_back(idx);
						idx = g_tsp_output_map[k][idx][0];
						while (g_tsp_output_map[k][idx].size() == 1 && g_tsp_input_map[k][idx].size() == 1 )
						{
							printf("%d-> ", idx);
							visited[idx] = true;
							g_tsp_segment[k][tsp_segment_n].push_back(idx);
							idx = g_tsp_output_map[k][idx][0];
						}
						printf("\n");
						tsp_segment_n++;
					}
				}
				for (int i = 0; i < g_tsp_n[k]; i++)
				{
					if (visited[i]) continue;
					g_tsp_segment[k][tsp_segment_n].clear();
					g_tsp_segment[k][tsp_segment_n].push_back(i);
					tsp_segment_n++;
				}
				g_tsp_segment_n[k] = tsp_segment_n;
				for (int i = 0; i < g_tsp_segment_n[k]; i++)
				{
					int start = g_tsp_segment[k][i][g_tsp_segment[k][i].size()-1];
					for (int j = 0; j < g_tsp_segment_n[k]; j++)
					{
						int end = g_tsp_segment[k][j][0];
						//printf("%d->%d	",start,end);
						g_tsp_segment_dist[k][i][j] = g_tsp_dist[k][start][end];
					}
				}
				int dis = tsp_lkh(g_tsp_segment_n[k], g_tsp_segment_dist[k], g_tsp_segment_path[k]);
				printf("start tsp:%d			%d\n", tsp_segment_n, dis);
				printf("start tsp:%d time:%f\n", g_tsp_n[k], timer_elapse());
				*/
				int dist = INFW;
				if (g_tsp_n[k] == 1)
				{
					dist = spfa(k, g_vertex_start, g_vertex_end, g_graph_spfa_ignore[k], g_tsp_path[k]);
					if (dist >= INFW || dist <= 0)
					{
						break;
					}
					std::vector<int> tpath;
					int pos = g_tsp_path[k][g_vertex_end];
					while (pos != g_vertex_start)
					{
						tpath.push_back(pos);
						pos = g_tsp_path[k][pos];
					}
					std::reverse(tpath.begin(), tpath.end());
					g_vertex_result[k].clear();
					g_vertex_result[k].push_back(g_vertex_start);
					g_vertex_result[k].insert(g_vertex_result[k].end(), tpath.begin(), tpath.end());
					g_vertex_result[k].push_back(g_vertex_end);
					g_segment_result[k][0] = g_vertex_result[k];
					g_path_ok[k] = true;
				}
				else
				{
					tsp_solver solver;
					dist = solver.solve(g_tsp_n[k], g_tsp_dist[k], g_tsp_path[k]);
					if (dist >= INFW || dist <= 0)
					{
						//printf("tsp  break dist:   %d \n", dist);
						break;
					}
					else
					{
						dist = get_tsp_setp_path(g_tsp_setp_path[k], k);//将TSP结果转化
						g_path_ok[k] = get_all_path_status(k, g_tsp_setp_path[k], g_vertex_result[k]);
					}
				}
				//printf("end tsp:%d time:%f\n", g_tsp_n[k], timer_elapse());
				if (g_path_ok[k])
				{
					std::vector<int> result = g_vertex_result[k];
					//检查重边
					memset(my_edge_cnt[k], 0, sizeof(my_edge_cnt[0]));
					for (int i = 0; i < result.size() - 1; i++)
					{
						int edge;
						if (result[i + 1] >= g_vertex_true_num)
						{
							edge = g_graph_backup_edgeid[result[i]][result[i + 2]];
							i = i + 1;
						}
						else
						{
							edge = g_graph_edgeid[result[i]][result[i + 1]];
						}
						my_edge_cnt[k][edge]++;
					}
					break;
				}
			}//while
		}//for
		int chongbian = 0;
		if (g_path_ok[0] && g_path_ok[1])
		{
			//这里控制运行时间
			if (timer_elapse() > 7000) break;
			//printf("重边检查 time:%f ms\n", timer_elapse());
			for (int kk = 0; kk < MAX_DEMAND_NUM; kk++)
			for (int i = 0; i < MAX_VERTEX_NUM; i++)
			{
				if (g_segment_result[kk][i].size() == 0)
					break;
				int weight = 0;
				for (int j = 0; j < g_segment_result[kk][i].size() - 1; j++)
				{
					int vs = g_segment_result[kk][i][j];
					int vt = g_segment_result[kk][i][j + 1];
					if (vt >= g_vertex_true_num)
					{
						weight += g_graph_backup_weight[vs][vt];
						j = j + 1;
					}
					else
					{
						weight += g_graph_weight[vs][vt];
					}
				}
				for (int j = 0; j < g_segment_result[kk][i].size() - 1; j++)
				{
					int vs = g_segment_result[kk][i][j];
					int vt = g_segment_result[kk][i][j + 1];
					int edge;
					if (vt >= g_vertex_true_num)
					{
						edge = g_graph_backup_edgeid[vs][vt];
						j = j + 1;
					}
					else
					{
						edge = g_graph_edgeid[vs][vt];
					}
					if (my_edge_cnt[0][edge] + my_edge_cnt[1][edge] > 1)
					{
						chongbian_index[edge][kk][0] = g_segment_result[kk][i][0];
						chongbian_index[edge][kk][1] = g_segment_result[kk][i][g_segment_result[kk][i].size() - 1];
						chongbian_index[edge][kk][2] = weight;
						chongbian++;
					}
				}
			}
			//printf("重边个数:%d\n", chongbian / 2);
			bool change0 = false;
			bool change1 = false;
			for (int i = 0; i < MAX_EDGE_NUM + 1; i++)
			{
				if (my_edge_cnt[0][i] + my_edge_cnt[1][i]>1)
				{
					if (g_edge_setp_mask[i] > 0)
						continue;
					chongbian++;
					int vs = g_edge_to_vertex[i] >> 16;
					int vt = g_edge_to_vertex[i] & 0xFFFF;
					int newresult[MAX_DEMAND_NUM] = { 0, 0 };
					int newsum[MAX_DEMAND_NUM] = { 0, 0 };
					for (int kk = 0; kk < MAX_DEMAND_NUM; kk++)
					{
						int pvs = chongbian_index[i][kk][0];
						int pvt = chongbian_index[i][kk][1];
						SET_EDGE_SETP(i, 1 - kk);
						int path[MAX_VERTEX_NUM];
						newresult[kk] = spfa(kk, pvs, pvt, g_graph_spfa_ignore[kk], path);
						newsum[kk] = newresult[kk];
						newsum[kk] += chongbian_index[i][1 - kk][2];
						RESET_EDGE_SETP(i)
					}
					if (newsum[0] >= INFW &&  newsum[1] >= INFW)
					{
						continue;
					}
					if (newsum[0] >= INFW)
					{
						int pvs = chongbian_index[i][0][0];
						int pvt = chongbian_index[i][0][1];
						int tsp_vs = put_tsp_vid(pvs, 0);
						int tsp_vt = put_tsp_vid(pvt, 0);
						int temp = g_tsp_dist[0][tsp_vs][tsp_vt];
						g_tsp_dist[0][tsp_vs][tsp_vt] = INFW;
						int path[MAX_VERTEX_NUM];
						tsp_solver solver;
						int newdist = solver.solve(g_tsp_n[0], g_tsp_dist[0], path);
						newsum[0] = chongbian_index[i][1][2] + newdist;
						g_tsp_dist[0][tsp_vs][tsp_vt] = temp;
					}
					if (newsum[1] >= INFW)
					{
						int pvs = chongbian_index[i][1][0];
						int pvt = chongbian_index[i][1][1];
						int tsp_vs = put_tsp_vid(pvs, 1);
						int tsp_vt = put_tsp_vid(pvt, 1);
						int temp = g_tsp_dist[1][tsp_vs][tsp_vt];
						g_tsp_dist[1][tsp_vs][tsp_vt] = INFW;
						int path[MAX_VERTEX_NUM];
						tsp_solver solver;
						int newdist = solver.solve(g_tsp_n[1], g_tsp_dist[1], path);
						newsum[1] = chongbian_index[i][0][2] + newdist;
						g_tsp_dist[1][tsp_vs][tsp_vt] = temp;
					}
					if (newsum[0] <= newsum[1])
					{
						SET_EDGE_SETP(i, 1);
						change0 = true;
					}
					else
					{
						SET_EDGE_SETP(i, 0);
						change1 = true;
					}
				}
			}
			if (!change0 && !change1)
				break;
			g_path_ok[0] = !change0;
			g_path_ok[1] = !change1;
		}
		else break;
	}
}
/**************************************************************************************************
0、功能总入口
**************************************************************************************************/
void search_route(char *graph[MAX_EDGE_NUM], int edge_num, char *condition[MAX_DEMAND_NUM], int demand_num)
{
	init_first(graph, edge_num, condition, demand_num);
	delete_isolated_vertexs();
	extend_setp_vertexs();
	add_virtual_vertexs();
	solve_by_tsp();
	//打印结果
	std::vector<int> result_vertex[MAX_DEMAND_NUM];
	int count = 0;
	std::vector<int> result_edge[MAX_DEMAND_NUM];
	for (int k = 0; k < MAX_DEMAND_NUM; k++)
	{
		int dist = 0;
		result_vertex[k] = g_vertex_result[k];
		count += dist;
		/*
		for (int i = 0; i < result_vertex[k].size(); i++)
		{
		if (IS_SETP_VERTEX(result_vertex[k][i], k))
		//	printf("[%d]->", g_graph_newtoold_idmap[result_vertex[k][i]]);
		printf("[%d]->", result_vertex[k][i]);
		else
		//	printf("%d->", g_graph_newtoold_idmap[result_vertex[k][i]]);
		printf("%d->", result_vertex[k][i]);
		}
		printf("\n===============\n");
		*/
		if (result_vertex[k].size() < 2)
			continue;
		for (int i = 0; i < result_vertex[k].size() - 1; i++)
		{
			int edge = 0;
			if (result_vertex[k][i + 1] >= g_vertex_true_num)
			{
				edge = g_graph_backup_edgeid[result_vertex[k][i]][result_vertex[k][i + 2]];
				i += 1;
			}
			else
				edge = g_graph_edgeid[result_vertex[k][i]][result_vertex[k][i + 1]];
			//printf("%d->", edge - 1);
			//edge_cnt[edge]++;
			result_edge[k].push_back(edge);
			record_result((PATH_ID)(k + 1), edge - 1);
		}
		//printf("\n\n\n");
	}


#if 0
	int edge_counts[MAX_EDGE_NUM + 1];
	memset(edge_counts, 0, sizeof(edge_counts));
	int ww = 0;
	int www[2] = { 0, 0 };
	int chongbian = 0;
	//检查结果合法性
	for (int k = 0; k < MAX_DEMAND_NUM; k++)
	{
		int vertex_count[MAX_VERTEX_NUM];
		memset(vertex_count, 0, sizeof(vertex_count));
		int bijng_cnt = 0;
		int aa = 0;
		for (int i = 0; i < result_vertex[k].size() - 1; i++)
		{
			int in = result_vertex[k][i];
			int out = result_vertex[k][i + 1];
			int edge;
			vertex_count[in]++;
			if (IS_SETP_VERTEX(in, k))
			{
				//	printf("%d :%d \n", aa++, in);
				bijng_cnt++;
			}
			if (g_graph_weight[in][out] == 0)
			{
				//printf("不可到达:%d->%d,\n", in, out);
			}
			if (vertex_count[in] > 1)
			{
				//	printf("重点: %d  %d\n", k, in);
			}
			if (out >= g_vertex_true_num)
			{
				int nextout = result_vertex[k][i + 2];
				edge = g_graph_backup_edgeid[in][nextout];
				i += 1;
				ww += g_graph_backup_weight[in][nextout] - 1;
				www[k] += g_graph_backup_weight[in][nextout] - 1;
			}
			else
			{
				www[k] += g_graph_weight[in][out] - 1;
				ww += g_graph_weight[in][out] - 1;
				edge = g_graph_edgeid[in][out];
			}
			edge_counts[edge]++;
			if (edge_counts[edge] > 1)
			{
				//printf("重边  %d \n", edge);
				chongbian++;
			}
		}
		//printf("bijing:%d %d\n", bijng_cnt, g_vertex_setp_num[k]);
	}
	printf("weight:%d     %d+%d			%d\n", ww, www[0], www[1], chongbian);

#endif


}
