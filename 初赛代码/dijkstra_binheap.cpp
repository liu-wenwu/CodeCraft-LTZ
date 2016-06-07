#include "common.h"
#include <vector>
#include <string.h>

using namespace std;


struct MinBinaryHeap{
	struct node{
		int key;
		int value;
		inline bool operator<(const node &t)const{
			return value < t.value;
		}
	}*array;
	int *location, heap_size;
	MinBinaryHeap(int size){
		heap_size = 0;
		array = new node[size + 1];
		location = new int[size];
	}
	~MinBinaryHeap(){
		delete[] array;
		delete[] location;
	}
	inline void update(int loc, const node& x){ //put x into array[loc]   
		array[loc] = x;
		location[x.key] = loc;
	}
	void per_down(int hole){
		node tem = array[hole];
		while (1){
			int child = hole << 1;
			if (child > heap_size)
				break;
			if (child != heap_size && array[child + 1] < array[child])
				child++;
			if (array[child] < tem){
				update(hole, array[child]);
				hole = child;
			}
			else break;
		}
		update(hole, tem);
	}
	void per_up(int hole){
		node tem = array[hole];
		while (hole > 1){
			if (tem < array[hole >> 1]){
				update(hole, array[hole >> 1]);
				hole >>= 1;
			}
			else break;
		}
		update(hole, tem);
	}
	void build_heap(int *values, int n){
		heap_size = n;
		for (int i = 1; i <= n; i++){
			array[i].key = i - 1;
			array[i].value = *(values++);
			location[array[i].key] = i;
		}
		for (int i = heap_size >> 1; i >= 1; i--)
			per_down(i);
	}
	pair<int, int> pop(){
		pair<int, int> res = make_pair(array[1].key, array[1].value);
		array[1] = array[heap_size--];
		per_down(1);
		return res;;
	}
	void decrease_to(int key, int value){
		array[location[key]].value = value;
		per_up(location[key]);
	}
};

//单点到多点
void dijkstra_binheap(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, int source, int dis[]){

	bool known[MAX_VERTEX_NUM] = { 0 };

	for (int i = 0; i < N; i++)
		dis[i] = INFW;

	dis[source] = 0;
	MinBinaryHeap heap(N);
	heap.build_heap(dis, N);
	for (int k = 0; k < N; k++){
		pair<int, int> tem = heap.pop();
		int i = tem.first;
		known[i] = true;

		for (int vi = 0; vi < N; vi++)
		{
			if (G[i][vi]<INFW)
			{
				if (!known[vi] && dis[vi] > dis[i] + G[i][vi]){
					dis[vi] = dis[i] + G[i][vi];
					heap.decrease_to(vi, dis[vi]);
				}
			}
		}
	}
}


//单点到单点
int dijkstra_binheap(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, int source, int end, int path[])
{

	bool known[MAX_VERTEX_NUM] = { 0 };
	int dis[MAX_VERTEX_NUM] = { 0 };
	for (int i = 0; i < N; i++)
		dis[i] = INFW;

	dis[source] = 0;
	MinBinaryHeap heap(N);
	heap.build_heap(dis, N);
	for (int k = 0; k < N; k++){
		pair<int, int> tem = heap.pop();
		int i = tem.first;
		known[i] = true;

		for (int vi = 0; vi < N; vi++)
		{
			if (G[i][vi]<INFW)
			{
				if (!known[vi] && dis[vi] > dis[i] + G[i][vi]){
					dis[vi] = dis[i] + G[i][vi];
					heap.decrease_to(vi, dis[vi]);
					path[vi] = i;
				}
			}
		}
	}




	return dis[end];
}


//单点到单点,忽略某些点
int dijkstra_binheap(int G[MAX_VERTEX_NUM][MAX_VERTEX_NUM], int N, int source, int end, bool disable[], int path[])
{

	bool known[MAX_VERTEX_NUM] = { 0 };

	int dis[MAX_VERTEX_NUM] = { 0 };

	memset(dis, INFW, sizeof(dis));
	memset(known, false, sizeof(known));
	//memcpy(known, disable, sizeof(known));


	dis[source] = 0;
	MinBinaryHeap heap(N);
	heap.build_heap(dis, N);
	for (int k = 0; k < N; k++){
		pair<int, int> tem = heap.pop();
		int i = tem.first;
		known[i] = true;

		for (int vi = 0; vi < N; vi++)
		{
			if (disable[vi] && vi != end) continue;
			//if (G[i][vi]<INFW)
			{

				if (!known[vi] && dis[vi] > dis[i] + G[i][vi]){
					dis[vi] = dis[i] + G[i][vi];
					heap.decrease_to(vi, dis[vi]);
					path[vi] = i;

				}
			}
		}
	}



	return dis[end];
}

//-------------------------------dijkstra算法	END----------------------------


