#include "common.h"
#include <vector>
#include <queue>
#include <string.h>

using namespace std;



int spfa(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, vector<int> gMap[MAX_VERTEX_NUM], int start, int end, bool disable[], int path[])
{
	int d[MAX_VERTEX_NUM];
	bool used[MAX_VERTEX_NUM];


	queue<int>q;
	q.push(start);

	memcpy(used, disable, sizeof(used));


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
		for (i = 0; i<gMap[node].size(); i++)   //����
		{
			t = gMap[node][i];

			dis = G[node][t];

			if (d[t]>d[node] + dis)
			{
				d[t] = d[node] + dis;  //�ɳڲ���

				//printf("%d\n", node);

				path[t] = node;
				if (!used[t])
				{
					used[t] = true;
					q.push(t);     //t�㲻�ڵ�ǰ�����У������β��
				}
			}

		}

	}
	/*
	int weight = 0;
	std::vector<int> pp;
	int pos = end;

	while (pos != start)
	{
	pp.push_back(pos);
	pos = path[pos];
	}
	pp.push_back(start);
	std::reverse(pp.begin(), pp.end());


	for (int i = 0; i < pp.size() - 1; i++)
	{
	weight += G[pp[i]][pp[i + 1]];
	}

	printf("spfa:%d \n", weight); */

	return d[end];
}





void spfa_allpair(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, std::vector<int> gMap[MAX_VERTEX_NUM], int start,
	bool disable[MAX_VERTEX_NUM], int dist[MAX_VERTEX_NUM], int path[])

{
	int d[MAX_VERTEX_NUM];
	bool used[MAX_VERTEX_NUM];


	queue<int>q;
	q.push(start);

	memcpy(used, disable, sizeof(used));


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
		for (i = 0; i<gMap[node].size(); i++)   //����
		{
			t = gMap[node][i];

			dis = G[node][t];

			if (d[t]>d[node] + dis)
			{
				d[t] = d[node] + dis;  //�ɳڲ���

				//printf("%d\n", node);
				path[t] = node;

				if (!used[t])
				{
					used[t] = true;
					q.push(t);     //t�㲻�ڵ�ǰ�����У������β��
				}
			}

		}

	}
	
	memcpy(dist, d,sizeof(d));



	return;
}