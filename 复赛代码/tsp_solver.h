#ifndef _TSP_SOLVER_H_
#define _TSP_SOLVER_H_
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <list>
#include <algorithm>
#include <stack>
class tsp_cand_edge;
class tsp_segment;
typedef std::vector<tsp_cand_edge>::iterator cand_edge_iter;
class tsp_node{
public:
	int node_id;//1~N
	int node_heap_idx;
	int node_priority;
	int node_degree;
	int node_last_degree;
	int node_cost;
	int node_next_cost;
	int node_penalty;
	int node_best_penalty;
	tsp_node *node_pre;
	tsp_node *node_suc;
	tsp_node *node_old_pre;
	tsp_node *node_old_suc;
	tsp_node *node_best_suc;
	tsp_node *node_next_best_suc;
	tsp_node *node_tree_parent;
	tsp_node *node_next;
	tsp_node *node_fixto;
	tsp_segment *node_segment_parent;
	bool node_old_pre_excluded;
	bool node_old_suc_excluded;
	bool active;
	std::vector<int> node_cost_row;
	//tsp_cand_edge* node_cand_set;
	std::vector<tsp_cand_edge> node_cand_set;
	tsp_node() :node_id(0),
		node_heap_idx(0),
		node_priority(INT_MIN),
		node_degree(0),
		node_last_degree(0),
		node_cost(INT_MAX),
		node_next_cost(INT_MAX),
		node_penalty(0),
		node_best_penalty(0),
		node_cost_row(0),
		node_pre(0),
		node_suc(0),
		node_old_pre(0),
		node_old_suc(0),
		node_best_suc(0),
		node_next_best_suc(0),
		node_tree_parent(0),
		node_next(0),
		node_fixto(0),
		node_segment_parent(0),
		node_old_pre_excluded(false),
		node_old_suc_excluded(false),
		active(false)
	{}
};
class tsp_cand_edge {
public:
	tsp_node *edge_to;
	int edge_cost;
	int edge_alpha;
	tsp_cand_edge(): edge_to(0),
		edge_cost(INT_MAX), 
		edge_alpha(INT_MAX)
	{}
};
class tsp_segment {
public:
	int segment_reversed;
	tsp_node *segment_first_node, *segment_last_node;
	tsp_segment *segment_pre;
	tsp_segment *segment_suc;
	int segment_priority;
	int segment_size;
	tsp_segment() :segment_reversed(0),
		segment_first_node(0),
		segment_last_node(0),
		segment_pre(0),
		segment_suc(0),
		segment_priority(0),
		segment_size(0)
	{}
};
class tsp_swap_record {
public:
	tsp_node *t1, *t2, *t3, *t4;
	tsp_swap_record(tsp_node *tt1, tsp_node *tt2, tsp_node *tt3, tsp_node *tt4) :
		t1(tt1), t2(tt2), t3(tt3), t4(tt4)
	{}
};
#define HashSize 65521
typedef struct hash_elem {
	unsigned int key;
	int value;
	hash_elem() : key(UINT_MAX), value(INT_MIN)
	{}
} hash_elem;
class cost_hash
{
public:
	void init()
	{
		for (int i = 0; i < HashSize; i++)
			elems[i] = hash_elem();
		length = 0;
	}
	void insert(unsigned int g_cost_hash, int cost)
	{
		int i, p;
		i = g_cost_hash % HashSize;
		if (length >= HashSize) {
			if (cost > elems[i].value)
				return;
		}
		else {
			p = 16 - g_cost_hash % 16;
			while (elems[i].value != INT_MIN)
			if ((i -= p) < 0)
				i += HashSize;
			length++;
		}
		elems[i].key = g_cost_hash;
		elems[i].value = cost;
	}
	int search(unsigned int g_cost_hash, int cost)
	{
		int i, p;
		i = g_cost_hash % HashSize;
		p = 16 - g_cost_hash % 16;
		while ((elems[i].key != g_cost_hash ||
			elems[i].value != cost) &&
			elems[i].value != INT_MIN)
		if ((i -= p) < 0)
			i += HashSize;
		return elems[i].key == g_cost_hash;
	}
private:
	static hash_elem elems[HashSize];
	int length;
};
class bin_heap{
public:
	bin_heap(int size)
	{
		data = new tsp_node *[size + 1];
		length = 0;
	}
	~bin_heap()
	{
		delete[] data;
	}
	void siftup(tsp_node * n)
	{
		int node_heap_idx, p;
		tsp_node *temp;
		node_heap_idx = n->node_heap_idx;
		p = node_heap_idx / 2;
		while (p && data[node_heap_idx]->node_priority < data[p]->node_priority) {
			temp = data[node_heap_idx];
			data[node_heap_idx] = data[p];
			data[p] = temp;
			data[node_heap_idx]->node_heap_idx = node_heap_idx;
			data[p]->node_heap_idx = p;
			node_heap_idx = p;
			p /= 2;
		}
	}
	tsp_node* delete_min()
	{
		tsp_node *remove, *item;
		int ch, node_heap_idx;
		if (!length)
			return 0;
		remove = data[1];
		remove->node_heap_idx = 0;
		item = data[length];
		length--;
		node_heap_idx = 1;
		ch = 2 * node_heap_idx;
		while (ch <= length) {
			if (ch < length &&
				data[ch + 1]->node_priority < data[ch]->node_priority)
				ch++;
			if (data[ch]->node_priority >= item->node_priority)
				break;
			data[node_heap_idx] = data[ch];
			data[node_heap_idx]->node_heap_idx = node_heap_idx;
			node_heap_idx = ch;
			ch *= 2;
		}
		data[node_heap_idx] = item;
		item->node_heap_idx = node_heap_idx;
		return remove;
	}
	void insert(tsp_node * n)
	{
		int ch, p;
		ch = ++length;
		p = ch / 2;
		while (p && n->node_priority < data[p]->node_priority) {
			data[ch] = data[p];
			data[ch]->node_heap_idx = ch;
			ch = p;
			p /= 2;
		}
		data[ch] = n;
		n->node_heap_idx = ch;
	}
private:
	tsp_node **data;
	int length;
};
#define MAX_TSP_N 2000*2
class tsp_solver{
public:
	tsp_solver();
	~tsp_solver();
	int solve(int n, int dist[MAX_TSP_N][MAX_TSP_N], int result[MAX_TSP_N]);
private:
	int g_tsp_dim;
	int g_cand_max_num, g_ascent_cand_max;
	tsp_node *g_node_set, *g_first_node;
	tsp_segment *g_first_segment;
	std::vector<tsp_segment *> segments;
	std::stack<tsp_swap_record> g_tsp_swaps;
	int g_tree_norm, tsp_group_size, tsp_groups;
	unsigned int g_cost_hash;
	std::vector<int> tsp_rand_nums;
	std::vector<int> tsp_better_tour;
	std::vector<int> tsp_best_tour;
	int g_tsp_reversed;
	cost_hash g_cost_hashtable;
	std::list<tsp_node *> g_active_nodes;
	bin_heap *g_bin_heap;
	bool tsp_invalid(tsp_node*ta, tsp_node*tb);
	tsp_node* tsp_pre_node(tsp_node* node);
	tsp_node* tsp_suc_node(tsp_node* node);
	int tsp_distance(tsp_node *Na, tsp_node *Nb);
	void tsp_activate_node(tsp_node * t);
	void tsp_adjust_cand_set();
	int tsp_between(const tsp_node *ta, const tsp_node *tb, const tsp_node *tc);
	void tsp_flip_node(tsp_node * t1, tsp_node * t2, tsp_node * t3);
	void swap_node1(tsp_node *a1, tsp_node *a2, tsp_node *a3);
	void swap_node2(tsp_node *a1, tsp_node *a2, tsp_node *a3, tsp_node *b1, tsp_node *b2, tsp_node *b3);
	void swap_node3(tsp_node *a1, tsp_node *a2, tsp_node *a3, tsp_node *b1, tsp_node *b2, tsp_node *b3,
		tsp_node *c1, tsp_node *c2, tsp_node *c3);
	void tsp_2opt_move(tsp_node * t1, tsp_node * t2, tsp_node * t3, tsp_node * t4);
	void tsp_3opt_move(tsp_node * t1, tsp_node * t2, tsp_node * t3, tsp_node * t4,
		tsp_node * t5, tsp_node * t6);
	tsp_node *tsp_best_3opt_move(tsp_node * t1, tsp_node * t2, int *G0, int *Gain);
	void tsp_normalize_nodelist();
	void tsp_record_better();
	tsp_node * tsp_remove_first_active();
	void tsp_reset_cand_set();
	void tsp_restore_tour();
	void tsp_store_tour();
	int tsp_bridge_gain(tsp_node *s1, tsp_node *s2, tsp_node *s3, tsp_node *s4,	int G);
	int tsp_gain32();
	void tsp_min_span_tree(bool Sparse);
	void tsp_connect(tsp_node * N1, const int Max, bool Sparse);
	int tsp_min_tree_cost(bool Sparse);
	void tsp_generate_cands(const int cand_max_num, const int MaxAlpha, bool Symmetric);
	int tsp_ascent();
	void tsp_choose_initial_tour();
	int tsp_lk();
	int tsp_find_tour();
	void tsp_init_data(int n, int dist[MAX_TSP_N][MAX_TSP_N]);
};
#endif