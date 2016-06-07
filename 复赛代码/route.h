
#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"


#define INFW 0x3f3f3f3f //权值无穷大
#define MAX_SETS_NUM 100//必经点个数

enum VertexType{
	VT_FREE,//自由点
	VT_START,//起始点
	VT_END,//结束点
	VT_SETP,//必经点
	VT_VIRTUAL//虚拟点，替换重边
};




void search_route(char *graph[MAX_EDGE_NUM], int edge_num, char *condition[MAX_DEMAND_NUM], int demand_num);

#endif
