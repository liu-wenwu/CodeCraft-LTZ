#include "route.h"
#include "common.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>



bool init(char * topo_path, char * demand_path)
{

	FILE *fp_topo = NULL;
	FILE *fp_demand = NULL;
	fp_topo = fopen(topo_path, "r");
	fp_demand = fopen(demand_path, "r");

	if (!fp_topo || !fp_demand)
		return false;

	char demand_str[1000];
	fgets(demand_str, 1000, fp_demand);

	//解析条件
	sscanf(demand_str, "%d,%d", &gVertexStart, &gVertexEnd);
	putIDStartEnd(gVertexStart, gVertexEnd);
	gVertexStart = 0;
	gVertexEnd = 1;


	gVertexType[gVertexStart] = VT_START;
	gVertexType[gVertexEnd] = VT_END;
	bool findCommma = false;
	int numVLine = 0;
	int posVLine[MAX_SETS_NUM];
	for (unsigned int i = 0; i < strlen(demand_str); i++)
	{
		if (demand_str[i] == ',')
		{
			if (!findCommma)findCommma = true;
			else {

				posVLine[numVLine] = i;
				numVLine++;
			}
		}
		else if (demand_str[i] == '|'){
			posVLine[numVLine] = i;
			numVLine++;
		}
	}
	gVertexNumVSet = numVLine;



	for (int i = 0; i < gVertexNumVSet; i++)
	{

		sscanf(demand_str + posVLine[i] + 1, "%d", &gVertexsVSet[i]);

		gVertexsVSet[i] = putID(gVertexsVSet[i]);
		gVertexType[gVertexsVSet[i]] = VT_VSET;
	}


	char topo_str[1000];
	while (!feof(fp_topo))
	{
		int edge, start, end, weight;

		fgets(topo_str, 1000, fp_topo);
		if (topo_str[0] == 0)
			continue;


		sscanf(topo_str, "%d,%d,%d,%d", &edge, &start, &end, &weight);

		start = putID(start);
		end = putID(end);


		if (start == gVertexEnd || end == gVertexStart)
			continue;

		if (gVertexType[start] == VT_NONE)
			gVertexType[start] = VT_FREE;
		if (gVertexType[end] == VT_NONE)
			gVertexType[end] = VT_FREE;


		if (gVertexType[start] != VT_FREE && gVertexType[start] != VT_NONE)
		{
			gDisableSPFA[start] = true;
		}
		if (gVertexType[end] != VT_FREE && gVertexType[end] != VT_NONE)
		{
			gDisableSPFA[end] = true;
		}



		if (gGraphWeight[start][end] == 0)
		{

			gGraphEdge[start][end] = edge;

			gGraphWeight[start][end] = weight;


			gMap[start].push_back(end);

			gVertexInputNum[end]++;

			gVertexOutputNum[start]++;

			gEdgeNum++;
		}
		else if (weight < gGraphWeight[start][end])//重边
		{
			gGraphEdge[start][end] = edge;
			gGraphWeight[start][end] = weight;
		}

	}

	gVertexNum = gIndexCount;


	if (gVertexOutputNum[gVertexStart] == 0 || gVertexInputNum[gVertexEnd] == 0)
	{
		return false;
	}


	for (int i = 0; i < gVertexNumVSet; i++)
	{
		if (gVertexInputNum[gVertexsVSet[i]] == 0 || gVertexOutputNum[gVertexsVSet[i]] == 0)
		{
			return false;
		}
	}


	//
	memcpy(gVertexTypeLKH, gVertexType, sizeof(gVertexType));



	memset(vset_keymap, -1, sizeof(vset_keymap));
	vset_keymap[gVertexStart] = 0;
	vset_keymap[gVertexEnd] = 0;

	return true;
}





void search_route(char * topo_path, char * demand_path)
{

	timer_start();

	bool ret = init(topo_path, demand_path);
	//if (!ret) return;

	int weight = INFW;
	while (true)
	{


		if (timer_elapse() > 6000)
		{
			break;
		}

		ret = init_tsp(); //这里没问题

		ret = sovle_by_lkh(weight);
		if (ret) break;

	}


	if (weight < INFW)
	{

		if (gShortestPath.size()>2)
		{

			get_all_path(gShortestPath);
			ret = write_result();
		}
		else record_result(1234567);

	}
	else
	{
		record_result(1234567);
	}


}


