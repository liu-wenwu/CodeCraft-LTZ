#include "tsp_solver.h"
hash_elem cost_hash::elems[HashSize];
inline bool tsp_fixed(tsp_node* a, tsp_node* b)
{
	return a->node_fixto == b;
}
inline void tsp_node_link(tsp_node* a, tsp_node* b)
{
	a->node_suc = b;
	b->node_pre = a;
}
inline void tsp_node_follow(tsp_node* b, tsp_node* a)
{
	if (a->node_suc == b)
		return;
	tsp_node_link((b)->node_pre, (b)->node_suc);
	tsp_node_link(b, (a)->node_suc);
	tsp_node_link(a, b);
}
inline void tsp_node_precede(tsp_node* a, tsp_node* b)
{
	if (b->node_pre == a)
		return;
	tsp_node_link((a)->node_pre, (a)->node_suc);
	tsp_node_link((b)->node_pre, a);
	tsp_node_link(a, b);
}
inline bool tsp_is_best_tour(tsp_node* a, tsp_node* b)
{
	return (a->node_best_suc == b || b->node_best_suc == a);
}
inline bool tsp_in_next_best_tour(tsp_node* a, tsp_node* b)
{
	return (a->node_next_best_suc == b || b->node_next_best_suc == a);
}
inline bool tsp_near(tsp_node* a, tsp_node* b)
{
	return ((a)->node_best_suc ? tsp_is_best_tour(a, b) : (a)->node_tree_parent == (b) || (b)->node_tree_parent == (a));
}
bool tsp_excludable(const tsp_node * ta, const tsp_node * tb)
{
	if (ta == tb->node_old_pre)
		return !tb->node_old_pre_excluded;
	if (ta == tb->node_old_suc)
		return !tb->node_old_suc_excluded;
	return false;
}
void tsp_exclude(tsp_node * ta, tsp_node * tb)
{
	if (ta == tb->node_pre || ta == tb->node_suc)
		return;
	if (ta == tb->node_old_pre)
		tb->node_old_pre_excluded = true;
	else if (ta == tb->node_old_suc)
		tb->node_old_suc_excluded = true;
	if (tb == ta->node_old_pre)
		ta->node_old_pre_excluded = true;
	else if (tb == ta->node_old_suc)
		ta->node_old_suc_excluded = true;
}
void tsp_split_segment(tsp_node * t1, tsp_node * t2)
{
	tsp_segment *segment_p = t1->node_segment_parent, *segment_q;
	tsp_node *t, *u;
	int i, count;
	if (t2->node_priority < t1->node_priority) {
		std::swap(t1, t2);
	}
	count = t1->node_priority - segment_p->segment_first_node->node_priority + 1;
	if (2 * count >= segment_p->segment_size)
	{
		if (segment_p->segment_reversed)
			segment_q = segment_p->segment_pre;
		else
			segment_q = segment_p->segment_suc;
		if (segment_p->segment_reversed == segment_q->segment_reversed)
			t = segment_q->segment_first_node;
		else
			t = segment_q->segment_last_node;
		i = t->node_priority;
		if (t != segment_q->segment_first_node)
		{
			for (t = segment_p->segment_last_node;; t = u)
			{
				if (t == t1) break;
				t->node_segment_parent = segment_q;
				t->node_priority = ++i;
				u = t->node_pre;
				t->node_pre = t->node_suc;
				t->node_suc = u;
			}
			segment_q->segment_last_node = t2;
		}
		else
		{
			bool flag = t == segment_q->segment_last_node && t->node_pre != segment_p->segment_last_node;
			if (flag)
			{
				u = t->node_suc;
				t->node_suc = t->node_pre;
				t->node_pre = u;
				segment_q->segment_reversed ^= 1;
			}
			for (t = segment_p->segment_last_node;; t = t->node_pre) {
				if (t == t1)break;
				t->node_segment_parent = segment_q;
				t->node_priority = --i;
			}
			segment_q->segment_first_node = t2;
		}
		count = segment_p->segment_size - count;
		segment_p->segment_last_node = t1;
	}
	else {
		segment_q = segment_p->segment_reversed ? segment_p->segment_suc : segment_p->segment_pre;
		t = segment_p->segment_reversed == segment_q->segment_reversed ? segment_q->segment_last_node : segment_q->segment_first_node;
		i = t->node_priority;
		if (t != segment_q->segment_last_node)
		{
			for (t = segment_p->segment_first_node;; t = u) {
				if (t == t2) break;
				t->node_segment_parent = segment_q;
				t->node_priority = --i;
				u = t->node_suc;
				t->node_suc = t->node_pre;
				t->node_pre = u;
			}
			segment_q->segment_first_node = t1;
		}
		else
		{
			if (t == segment_q->segment_first_node && t->node_suc != segment_p->segment_first_node) {
				u = t->node_suc;
				t->node_suc = t->node_pre;
				t->node_pre = u;
				segment_q->segment_reversed ^= 1;
			}
			for (t = segment_p->segment_first_node;; t = t->node_suc) {
				if (t == t2) break;
				t->node_segment_parent = segment_q;
				t->node_priority = ++i;
			}
			segment_q->segment_last_node = t1;
		}
		segment_p->segment_first_node = t2;
	}
	segment_p->segment_size -= count;
	segment_q->segment_size += count;
}
inline bool tsp_solver::tsp_invalid(tsp_node*ta, tsp_node*tb)
{
	return ((ta->node_id <= g_tsp_dim / 2) == (tb->node_id <= g_tsp_dim / 2));
}
inline tsp_node* tsp_solver::tsp_pre_node(tsp_node* node)
{
	return (g_tsp_reversed == node->node_segment_parent->segment_reversed ? node->node_pre : node->node_suc);
}
inline tsp_node* tsp_solver::tsp_suc_node(tsp_node* node)
{
	return (g_tsp_reversed == node->node_segment_parent->segment_reversed ? node->node_suc : node->node_pre);
}
int tsp_solver::tsp_distance(tsp_node *node_a, tsp_node *node_b)
{
	int dist = 0;
	if (tsp_fixed(node_a, node_b))
		dist = 0;
	else
	{
		int n = g_tsp_dim / 2;
		if ((node_a->node_id <= n) == (node_b->node_id <= n))
			dist = 0x3F3F3F3F;
		else if (abs(node_a->node_id - node_b->node_id) == n)
			dist = 0;
		else dist = node_a->node_id < node_b->node_id ? node_a->node_cost_row[node_b->node_id - n] : node_b->node_cost_row[node_a->node_id - n];
	}
	return dist + node_a->node_penalty + node_b->node_penalty;
}
void tsp_solver::tsp_activate_node(tsp_node * t)
{
	if (t->active)
		return;
	g_active_nodes.push_back(t);
	t->active = true;
}
void tsp_solver::tsp_adjust_cand_set()
{
	tsp_node *edge_to;
	tsp_node *node_from = g_first_node;
	while (true)
	{
		edge_to = node_from->node_pre;
		while (true)
		{
			if (!edge_to) break;
			int count = 0;
			cand_edge_iter edge_from;
			edge_from = node_from->node_cand_set.begin();
			while (true)
			{
				bool flag = edge_from->edge_to && edge_from->edge_to != edge_to;
				if (!flag) break;
				count++;
				edge_from++;
			}
			if (!edge_from->edge_to)
			{
				edge_from->edge_cost = tsp_distance(node_from, edge_to);
				edge_from->edge_to = edge_to;
				edge_from->edge_alpha = INT_MAX;
				node_from->node_cand_set.resize(count + 2);
				node_from->node_cand_set[count + 1].edge_to = 0;
			}
			edge_to = edge_to == node_from->node_pre ? node_from->node_suc : 0;
		}
		node_from = node_from->node_suc;
		if (node_from == g_first_node)
			break;
	};
	while (true)
	{
		for (cand_edge_iter edge_from = node_from->node_cand_set.begin() + 1; edge_to = edge_from->edge_to; edge_from++)
		if (tsp_is_best_tour(node_from, edge_to) && tsp_in_next_best_tour(node_from, edge_to)) {
			tsp_cand_edge temp = *edge_from;
			cand_edge_iter edge_nn;
			for (edge_nn = edge_from - 1; edge_nn >= node_from->node_cand_set.begin(); edge_nn--)
				*(edge_nn + 1) = *edge_nn;
			*(edge_nn + 1) = temp;
		}
		node_from = node_from->node_suc;
		if (node_from == g_first_node)
			break;
	};
}
int tsp_solver::tsp_between(const tsp_node *ta, const tsp_node *tb, const tsp_node *tc)
{
	if (tb == ta || tb == tc)
		return 1;
	if (ta == tc)
		return 0;
	tsp_segment *segment_a, *segment_b, *segment_c;
	segment_a = ta->node_segment_parent;
	segment_b = tb->node_segment_parent;
	segment_c = tc->node_segment_parent;
	bool reversed;
	bool priority;
	if (segment_a == segment_c)
	{
		if (segment_b == segment_a)
		{
			reversed = g_tsp_reversed == segment_a->segment_reversed;
			priority = (ta->node_priority < tc->node_priority ?
				tb->node_priority > ta->node_priority && tb->node_priority < tc->node_priority :
			tb->node_priority > ta->node_priority || tb->node_priority < tc->node_priority);
			return reversed == priority;
		}
		reversed = g_tsp_reversed == segment_a->segment_reversed;
		priority = ta->node_priority > tc->node_priority;
		return reversed == priority;
	}
	if (segment_b == segment_c)
	{
		reversed = g_tsp_reversed == segment_b->segment_reversed;
		priority = tb->node_priority < tc->node_priority;
		return reversed == priority;
	}
	if (segment_a == segment_b)
	{
		reversed = g_tsp_reversed == segment_a->segment_reversed;
		priority = ta->node_priority < tb->node_priority;
		return reversed == priority;
	}
	reversed = !g_tsp_reversed;
	priority = (segment_a->segment_priority < segment_c->segment_priority ?
		segment_b->segment_priority > segment_a->segment_priority && segment_b->segment_priority < segment_c->segment_priority :
	segment_b->segment_priority > segment_a->segment_priority || segment_b->segment_priority < segment_c->segment_priority);
	return reversed == priority;
}
void tsp_solver::tsp_flip_node(tsp_node * t1, tsp_node * t2, tsp_node * t3)
{
	tsp_node *t4, *a, *b, *c, *d;
	tsp_segment *segment_p1, *segment_p2, *segment_p3, *segment_p4, *segment_q1, *segment_q2;
	tsp_node *s1, *s2;
	int i;
	if (t3 == t2->node_pre || t3 == t2->node_suc)
		return;
	if (t2 == tsp_suc_node(t1))
		t4 = tsp_pre_node(t3);
	else
		t4 = tsp_suc_node(t3);

	segment_p1 = t1->node_segment_parent;
	segment_p2 = t2->node_segment_parent;
	segment_p3 = t3->node_segment_parent;
	segment_p4 = t4->node_segment_parent;

	if (segment_p1 != segment_p3 && segment_p2 != segment_p4)
	{
		if (segment_p1 == segment_p2) {
			tsp_split_segment(t1, t2);
			segment_p1 = t1->node_segment_parent;
			segment_p2 = t2->node_segment_parent;
		}
		if (segment_p3 == segment_p4 && segment_p1 != segment_p3 && segment_p2 != segment_p4) {
			tsp_split_segment(t3, t4);
			segment_p3 = t3->node_segment_parent;
			segment_p4 = t4->node_segment_parent;
		}
	}
	else
	{
		bool flag = (segment_p1 == segment_p3 && abs(t3->node_priority - t1->node_priority) > 3 * tsp_group_size / 4) ||
			(segment_p2 == segment_p4 && abs(t4->node_priority - t2->node_priority) > 3 * tsp_group_size / 4);
		if (flag)
		{
			if (segment_p1 == segment_p2) {
				tsp_split_segment(t1, t2);
				segment_p1 = t1->node_segment_parent;
				segment_p2 = t2->node_segment_parent;
				segment_p3 = t3->node_segment_parent;
				segment_p4 = t4->node_segment_parent;
			}
			if (segment_p3 == segment_p4) {
				tsp_split_segment(t3, t4);
				segment_p1 = t1->node_segment_parent;
				segment_p2 = t2->node_segment_parent;
				segment_p3 = t3->node_segment_parent;
				segment_p4 = t4->node_segment_parent;
			}

		}
	}
	a = 0;
	if (segment_p1 == segment_p3) {
		if (t1->node_priority < t3->node_priority)
		{
			(segment_p1 == segment_p2 && segment_p1 == segment_p4 && t2->node_priority > t1->node_priority) ?
				(a = t1, b = t2, c = t3, d = t4) : (a = t2, b = t1, c = t4, d = t3);
		}
		else
		{
			(segment_p1 == segment_p2 && segment_p1 == segment_p4 && t2->node_priority < t1->node_priority) ?
				(a = t3, b = t4, c = t1, d = t2) : (a = t4, b = t3, c = t2, d = t1);
		}
	}
	else if (segment_p2 == segment_p4) {
		(t4->node_priority < t2->node_priority) ?
			(a = t3, b = t4, c = t1, d = t2) : (a = t1, b = t2, c = t3, d = t4);
	}
	if (a) {
		i = d->node_priority;
		d->node_suc = 0;
		s2 = b;
		for (; s1 = s2;) {
			s2 = s1->node_suc;
			s1->node_suc = s1->node_pre;
			s1->node_pre = s2;
			s1->node_priority = i--;
		}
		d->node_pre = a;
		b->node_suc = c;

		(a->node_suc == b) ? (a->node_suc = d) : (a->node_pre = d);
		(c->node_pre == d) ? (c->node_pre = b) : (c->node_suc = b);


		if (b->node_segment_parent->segment_first_node == b)
			b->node_segment_parent->segment_first_node = d;
		else if (d->node_segment_parent->segment_first_node == d)
			d->node_segment_parent->segment_first_node = b;
		if (b->node_segment_parent->segment_last_node == b)
			b->node_segment_parent->segment_last_node = d;
		else if (d->node_segment_parent->segment_last_node == d)
			d->node_segment_parent->segment_last_node = b;
	}
	else {
		if (segment_p1->segment_suc != segment_p2) {
			std::swap(t1, t2);
			std::swap(t3, t4);
			segment_q1 = segment_p1; segment_p1 = segment_p2; segment_p2 = segment_q1;
			segment_q1 = segment_p3; segment_p3 = segment_p4; segment_p4 = segment_q1;
		}
		if ((i = segment_p2->segment_priority - segment_p3->segment_priority) <= 0)
			i += tsp_groups;
		if (2 * i > tsp_groups) {
			std::swap(t3, t2);
			std::swap(t1, t4);
			segment_q1 = segment_p3; segment_p3 = segment_p2; segment_p2 = segment_q1;
			segment_q1 = segment_p1; segment_p1 = segment_p4; segment_p4 = segment_q1;
		}
		i = segment_p1->segment_priority;
		segment_p1->segment_suc = 0;
		segment_q2 = segment_p3;
		for (; segment_q1 = segment_q2;)
		{
			segment_q2 = segment_q1->segment_suc;
			segment_q1->segment_suc = segment_q1->segment_pre;
			segment_q1->segment_pre = segment_q2;
			segment_q1->segment_priority = i--;
			segment_q1->segment_reversed ^= 1;
		}
		segment_p3->segment_suc = segment_p2;
		segment_p2->segment_pre = segment_p3;
		segment_p1->segment_pre = segment_p4;
		segment_p4->segment_suc = segment_p1;
		(t3->node_suc == t4) ? (t3->node_suc = t2) : (t3->node_pre = t2);
		(t2->node_suc == t1) ? (t2->node_suc = t3) : (t2->node_pre = t3);
		(t1->node_pre == t2) ? (t1->node_pre = t4) : (t1->node_suc = t4);
		(t4->node_pre == t3) ? (t4->node_pre = t1) : (t4->node_suc = t1);
	}
	g_tsp_swaps.push(tsp_swap_record(t1, t2, t3, t4));
	g_cost_hash ^= (tsp_rand_nums[t1->node_id] * tsp_rand_nums[t2->node_id]) ^
		(tsp_rand_nums[t3->node_id] * tsp_rand_nums[t4->node_id]) ^
		(tsp_rand_nums[t2->node_id] * tsp_rand_nums[t3->node_id]) ^
		(tsp_rand_nums[t4->node_id] * tsp_rand_nums[t1->node_id]);
}
inline void tsp_solver::swap_node1(tsp_node *a1, tsp_node *a2, tsp_node *a3)
{
	tsp_flip_node(a1, a2, a3);
}
inline void tsp_solver::swap_node2(tsp_node *a1, tsp_node *a2, tsp_node *a3, tsp_node *b1, tsp_node *b2, tsp_node *b3)
{
	swap_node1(a1, a2, a3); swap_node1(b1, b2, b3);
}
inline void tsp_solver::swap_node3(tsp_node *a1, tsp_node *a2, tsp_node *a3, tsp_node *b1, tsp_node *b2, tsp_node *b3,
	tsp_node *c1, tsp_node *c2, tsp_node *c3)
{
	swap_node1(a1, a2, a3); swap_node1(b1, b2, b3); swap_node1(c1, c2, c3);
}
inline void tsp_solver::tsp_2opt_move(tsp_node * t1, tsp_node * t2, tsp_node * t3, tsp_node * t4)
{
	swap_node1(t1, t2, t3);
}
inline void tsp_solver::tsp_3opt_move(tsp_node * t1, tsp_node * t2, tsp_node * t3, tsp_node * t4,
	tsp_node * t5, tsp_node * t6)
{
	swap_node3(t1, t2, t4, t6, t5, t4, t6, t2, t3);
}
void tsp_solver::tsp_normalize_nodelist()
{
	tsp_node *t1, *t2;
	t1 = g_first_node;
	do {
		t2 = tsp_suc_node(t1);
		t1->node_pre = tsp_pre_node(t1);
		t1->node_suc = t2;
	} while ((t1 = t2) != g_first_node);
}
void tsp_solver::tsp_record_better()
{
	tsp_node *N;
	int i, k;
	for (i = 1, N = g_first_node, k = 0; i <= g_tsp_dim; i++, N = N->node_suc) {
		if (N->node_id <= g_tsp_dim / 2) {
			k++;
			if (N->node_suc->node_id != N->node_id + g_tsp_dim / 2)
				tsp_better_tour[k] = N->node_id;
			else
				tsp_better_tour[g_tsp_dim / 2 - k + 1] = N->node_id;
		}
		N->node_next_best_suc = N->node_best_suc;
		N->node_best_suc = N->node_suc;
	}
}
tsp_node *tsp_solver::tsp_remove_first_active()
{
	if (g_active_nodes.empty())
		return 0;
	tsp_node *t = g_active_nodes.front();
	g_active_nodes.pop_front();
	t->active = false;
	return t;
}
void tsp_solver::tsp_reset_cand_set()
{
	cand_edge_iter edge_from;
	tsp_node *node_from;
	node_from = g_first_node;
	do {
		for (edge_from = node_from->node_cand_set.begin(); edge_from->edge_to; edge_from++);
		edge_from--;
		while (edge_from->edge_alpha == INT_MAX)
			edge_from--;
		edge_from++;
		edge_from->edge_to = 0;
	} while ((node_from = node_from->node_suc) != g_first_node);
}
void tsp_solver::tsp_restore_tour()
{
	for (; !g_tsp_swaps.empty();) {
		tsp_swap_record rec = g_tsp_swaps.top();
		g_tsp_swaps.pop();
		swap_node1(rec.t3, rec.t2, rec.t1);
		g_tsp_swaps.pop();
		rec.t1->node_old_pre_excluded = false;
		rec.t1->node_old_suc_excluded = false;
		rec.t2->node_old_pre_excluded = false;
		rec.t2->node_old_suc_excluded = false;
		rec.t3->node_old_pre_excluded = false;
		rec.t3->node_old_suc_excluded = false;
		rec.t4->node_old_pre_excluded = false;
		rec.t4->node_old_suc_excluded = false;
	}
}
void tsp_solver::tsp_store_tour()
{
	tsp_node *t, *u;

	int i;
	for (; !g_tsp_swaps.empty();)
	{
		tsp_swap_record rec = g_tsp_swaps.top();
		g_tsp_swaps.pop();
		i = 0;
		while (++i <= 4)
		{
			switch (i)
			{
			case 1:t = rec.t1; break;
			case 2:t = rec.t2; break;
			case 3:t = rec.t3; break;
			case 4:t = rec.t4; break;
			}
			tsp_activate_node(t);
			t->node_old_pre = t->node_pre;
			t->node_old_suc = t->node_suc;
			t->node_old_pre_excluded = t->node_old_suc_excluded = false;
			t->node_cost = INT_MAX;
			for (cand_edge_iter Nt = t->node_cand_set.begin();; Nt++)
			{
				u = Nt->edge_to;
				if (!u)break;
				if (u != t->node_pre && u != t->node_suc && Nt->edge_cost < t->node_cost)
					t->node_cost = Nt->edge_cost;
			}
		}
	}
}
tsp_node * tsp_solver::tsp_best_3opt_move(tsp_node * t1, tsp_node * t2, int *G0, int *Gain)
{
	tsp_node  *t3, *t4, *t5, *t6, *T3, *T4, *T5, *T6 = 0;
	int G1, G3, BestG4 = INT_MIN;
	if (tsp_suc_node(t1) != t2)
		g_tsp_reversed ^= 1;
	for (cand_edge_iter Nt2 = t2->node_cand_set.begin();; Nt2++)
	{
		t3 = Nt2->edge_to;

		if (!t3) break;

		if (t3 == t2->node_pre || t3 == t2->node_suc || (G1 = *G0 - Nt2->edge_cost) <= 0)
			continue;
		for (int X4 = 1; X4 <= 2; X4++)
		{
			if (X4 == 1)
				t4 = tsp_pre_node(t3);
			else
				t4 = tsp_suc_node(t3);

			if (tsp_fixed(t3, t4))
				continue;
			int G2 = G1 + tsp_distance(t3, t4);
			if (X4 == 1 && !tsp_invalid(t4, t1) && (*Gain = G2 - tsp_distance(t4, t1)) > 0) {
				swap_node1(t1, t2, t3);
				*G0 = G2;
				return t4;
			}
			for (cand_edge_iter Nt4 = t4->node_cand_set.begin();; Nt4++)
			{
				t5 = Nt4->edge_to;
				if (!t5) break;

				if (t5 == t4->node_pre || t5 == t4->node_suc || (G3 = G2 - Nt4->edge_cost) <= 0 || (X4 == 2 && !tsp_between(t2, t5, t3)))
					continue;
				for (int X6 = 1; X6 <= X4; X6++)
				{
					if (X4 == 1) {
						if (tsp_between(t2, t5, t4))
							t6 = tsp_suc_node(t5);
						else
							t6 = tsp_pre_node(t5);
					}
					else {
						if (X6 == 1)
							t6 = tsp_suc_node(t5);
						else
							t6 = tsp_pre_node(t5);
						if (t6 == t1)
							continue;
					}
					if (tsp_fixed(t5, t6))
						continue;
					int G4 = G3 + tsp_distance(t5, t6);
					if (!tsp_invalid(t6, t1) && (*Gain = G4 - tsp_distance(t6, t1)) > 0)
					{
						tsp_3opt_move(t1, t2, t3, t4, t5, t6);
						*G0 = G4;
						return t6;
					}
					if (G4 > BestG4 && g_tsp_swaps.size() < g_tsp_dim && G4 - t6->node_cost >= 1 && tsp_excludable(t5, t6)) {
						if (G3 + t5->node_penalty == G1 + t3->node_penalty)
							continue;
						T3 = t3;
						T4 = t4;
						T5 = t5;
						T6 = t6;
						BestG4 = G4;
					}
				}
			}
		}
	}
	*Gain = 0;
	if (T6) {
		tsp_3opt_move(t1, t2, T3, T4, T5, T6);
		tsp_exclude(t1, t2);
		tsp_exclude(T3, T4);
		tsp_exclude(T5, T6);
		*G0 = BestG4;
	}
	return T6;
}
int tsp_solver::tsp_bridge_gain(tsp_node *s1, tsp_node *s2, tsp_node *s3, tsp_node *s4, int G)
{
	tsp_node *t1, *t2, *t3, *t4, *u2, *u3;
	int G0, G1, G2, Gain, i;

	if ((i = !g_tsp_reversed ? s3->node_degree - s2->node_degree : s2->node_degree - s3->node_degree) < 0)
		i += g_tsp_dim;

	(2 * i <= g_tsp_dim) ? (u2 = s2, u3 = s3) : (u2 = s4, u3 = s1);
	for (t1 = u2; t1 != u3; t1 = t2)
	{

		t2 = tsp_suc_node(t1);
		if ((t1 == s1 && t2 == s2) ||(t1 == s2 && t2 == s1) ||(t1 == s3 && t2 == s4) ||(t1 == s4 && t2 == s3) ||tsp_fixed(t1, t2))
			continue;
		G0 = G + tsp_distance(t1, t2);
		for (cand_edge_iter Nt2 = t2->node_cand_set.begin(); ; Nt2++)
		{
			t3 = Nt2->edge_to;
			if (!t3) break;
			if (t3 == t2->node_pre || t3 == t2->node_suc || tsp_between(u2, t3, u3))
				continue;
			G1 = G0 - Nt2->edge_cost;
			t4 = tsp_suc_node(t3);
			if (t4 == t2)
				continue;
			if ((t3 == s1 && t4 == s2) ||(t3 == s2 && t4 == s1) ||(t3 == s3 && t4 == s4) ||(t3 == s4 && t4 == s3) ||tsp_fixed(t3, t4))
				continue;
			G2 = G1 + tsp_distance(t3, t4);
			if (!tsp_invalid(t4, t1) && (Gain = G2 - tsp_distance(t4, t1)) > 0) {
				swap_node3(s1, s2, s4, t3, t4, t1, s1, s3, s2);
				return Gain;
			}
		}
	}
	return 0;
}
int tsp_solver::tsp_gain32()
{
	tsp_node *s1, *s2, *s3, *s4, *s5, *s6;
	int  G1, G3, Gain, Gain6, i;
	g_tsp_reversed = 0;
	i = 0;
	s1 = g_first_node;
	do
	s1->node_degree = ++i;
	while ((s1 = tsp_suc_node(s1)) != g_first_node);
	for (int X2 = 1; X2 <= 2; X2++) {
		g_tsp_reversed ^= 1;
		do {
			s2 = tsp_suc_node(s1);
			if (tsp_fixed(s1, s2))
				continue;
			int G0 = tsp_distance(s1, s2);
			for (cand_edge_iter Ns2 = s2->node_cand_set.begin(); s3 = Ns2->edge_to; Ns2++) {
				if (s3 == s1 || (s4 = tsp_suc_node(s3)) == s1 || tsp_fixed(s3, s4))
					continue;
				if ((i = !g_tsp_reversed ? s3->node_degree - s2->node_degree : s2->node_degree - s3->node_degree) <= 0)
					i += g_tsp_dim;
				if (2 * i > g_tsp_dim)
					continue;
				G1 = G0 - Ns2->edge_cost;
				int G2 = G1 + tsp_distance(s3, s4);
				if (!tsp_invalid(s4, s1) &&
					(G3 = G2 - tsp_distance(s4, s1)) > 0 &&
					(Gain = tsp_bridge_gain(s1, s2, s3, s4, G3)) > 0)
					return Gain;
			}
		} while ((s1 = s2) != g_first_node);
	}
	for (int X2 = 1; X2 <= 2; X2++) {
		g_tsp_reversed ^= 1;
		do {
			s2 = tsp_suc_node(s1);
			if (tsp_fixed(s1, s2))
				continue;
			int G0 = tsp_distance(s1, s2);
			for (cand_edge_iter Ns2 = s2->node_cand_set.begin(); s3 = Ns2->edge_to; Ns2++) {
				if (s3 == s2->node_pre || s3 == s2->node_suc ||
					(G1 = G0 - Ns2->edge_cost) <= 0)
					continue;
				s4 = tsp_suc_node(s3);
				if (tsp_fixed(s3, s4))
					continue;
				int G2 = G1 + tsp_distance(s3, s4);
				for (cand_edge_iter Ns4 = s4->node_cand_set.begin(); s5 = Ns4->edge_to; Ns4++) {
					if (s5 == s4->node_pre || s5 == s4->node_suc ||
						(G3 = G2 - Ns4->edge_cost) <= 0)
						continue;
					if (tsp_between(s2, s5, s3)) {
						s6 = tsp_suc_node(s5);
						if (s6 == s1)
							continue;
					}
					else {
						s6 = tsp_pre_node(s5);
					}
					if (tsp_fixed(s5, s6))
						continue;
					int G4 = G3 + tsp_distance(s5, s6);
					Gain6 = 0;
					if (!tsp_invalid(s6, s1) &&
						(Gain6 = G4 - tsp_distance(s6, s1)) > 0) {
						tsp_3opt_move(s1, s2, s3, s4, s5, s6);
						return Gain6;
					}
				}
			}
		} while ((s1 = s2) != g_first_node);
	}
	return 0;
}
void tsp_solver::tsp_min_span_tree(bool Sparse)
{
	tsp_node* Blue;
	tsp_node* NextBlue;
	tsp_node* N;
	int d;
	Blue = N = g_first_node;
	Blue->node_tree_parent = 0;
	Blue->node_heap_idx = 0;
	if (!(Sparse && !Blue->node_cand_set.empty()))
	{
		int Min;
		while ((N = N->node_suc) != g_first_node)
			N->node_cost = INT_MAX;
		while (true)
		{
			N = Blue->node_suc;
			if (N == g_first_node) break;
			Min = INT_MAX;
			while (true)
			{
				if (tsp_fixed(Blue, N)) {
					N->node_tree_parent = Blue;
					N->node_cost = tsp_distance(N, Blue);
					NextBlue = N;
					Min = INT_MIN;
				}
				else {
					bool flag = !tsp_invalid(N, Blue) && (d = tsp_distance(Blue, N)) < N->node_cost;
					if (flag)
					{
						N->node_cost = d;
						N->node_tree_parent = Blue;
					}
					if (N->node_cost < Min)
					{
						Min = N->node_cost;
						NextBlue = N;
					}
				}
				N = N->node_suc;
				if (N == g_first_node)
					break;
			};
			tsp_node_follow(NextBlue, Blue);
			Blue = NextBlue;
		}
	}
	else
	{
		while (true)
		{
			N = N->node_suc;
			if (N == g_first_node) break;
			N->node_tree_parent = Blue;
			N->node_cost = N->node_priority = INT_MAX;
			g_bin_heap->insert(N);
		}
		for (cand_edge_iter NBlue = Blue->node_cand_set.begin();; NBlue++)
		{
			N = NBlue->edge_to;
			if (!N)break;
			if (!tsp_fixed(N, Blue))
			{
				N->node_tree_parent = Blue;
				N->node_cost = N->node_priority = NBlue->edge_cost + N->node_penalty + Blue->node_penalty;
				g_bin_heap->siftup(N);
			}
			else
			{
				int newcost = NBlue->edge_cost + N->node_penalty + Blue->node_penalty;
				N->node_cost = newcost;
				N->node_tree_parent = Blue;
				N->node_priority = INT_MIN;
				g_bin_heap->siftup(N);
			}
		}
		while (true)
		{
			NextBlue = g_bin_heap->delete_min();
			if (!NextBlue) break;
			tsp_node_follow(NextBlue, Blue);
			Blue = NextBlue;
			for (cand_edge_iter NBlue = Blue->node_cand_set.begin();; NBlue++)
			{
				N = NBlue->edge_to;
				if (!N) break;
				if (!N->node_heap_idx)
					continue;
				if (tsp_fixed(N, Blue))
				{
					int newcost = NBlue->edge_cost + N->node_penalty + Blue->node_penalty;
					N->node_cost = newcost;
					N->node_tree_parent = Blue;
					N->node_priority = INT_MIN;
					g_bin_heap->siftup(N);
				}
				else
				{
					if ((d = NBlue->edge_cost + N->node_penalty + Blue->node_penalty) < N->node_cost)
					{
						N->node_tree_parent = Blue;
						N->node_cost = N->node_priority = d;
						g_bin_heap->siftup(N);
					}
				}
			}
		}
	}
}
void tsp_solver::tsp_connect(tsp_node * N1, const int Max, bool Sparse)
{
	tsp_node *N;
	cand_edge_iter NN1;
	int d;
	N1->node_next = 0;
	N1->node_next_cost = INT_MAX;
	if (!(!Sparse || N1->node_cand_set.empty()))
	{
		for (NN1 = N1->node_cand_set.begin();; NN1++)
		{
			N = NN1->edge_to;
			if (!N)
				break;
			bool flag = N == N1->node_tree_parent || N1 == N->node_tree_parent;
			if (flag)
				continue;
			flag = tsp_fixed(N1, N);
			if (flag)
			{
				N1->node_next_cost = NN1->edge_cost + N1->node_penalty + N->node_penalty;
				N1->node_next = N;
				return;
			}
			flag = (d = NN1->edge_cost + N1->node_penalty + N->node_penalty) < N1->node_next_cost;
			if (flag)
			{
				N1->node_next_cost = d;
				if (d <= Max)
					return;
				N1->node_next = N;
			}
		}
	}
	else
	{
		N = g_first_node;
		while (true)
		{
			bool flag = N == N1 || N == N1->node_tree_parent || N1 == N->node_tree_parent;
			if (flag)
			{
				N = N->node_suc;
				if (N == g_first_node)
					break;
				continue;
			}
			flag = tsp_fixed(N1, N);
			if (flag)
			{
				N1->node_next_cost = tsp_distance(N1, N);
				N1->node_next = N;
				return;
			}
			flag = !tsp_invalid(N1, N) && (d = tsp_distance(N1, N)) < N1->node_next_cost;
			if (flag)
			{
				N1->node_next_cost = d;
				if (d <= Max)
					return;
				N1->node_next = N;
			}
			N = N->node_suc;
			if (N == g_first_node)
				break;
		};
	}
}
int tsp_solver::tsp_min_tree_cost(bool Sparse)
{
	tsp_node *N, *N1;
	int Sum = 0;
	int Max;
	tsp_min_span_tree(Sparse);
	N = g_first_node;
	while (true)
	{
		N->node_degree = -2;
		Sum += N->node_penalty;
		N = N->node_suc;
		if (N == g_first_node)
			break;
	}
	Sum *= -2;
	while ((N = N->node_suc) != g_first_node) {
		N->node_degree++;
		N->node_tree_parent->node_degree++;
		Sum += N->node_cost;
		N->node_next = 0;
	}
	g_first_node->node_tree_parent = g_first_node->node_suc;
	g_first_node->node_cost = g_first_node->node_suc->node_cost;
	Max = INT_MIN;
	while (true)
	{
		if (N->node_degree == -1) {
			tsp_connect(N, Max, Sparse);
			if (N->node_next_cost > Max) {
				N1 = N;
				Max = N->node_next_cost;
			}
		}
		N = N->node_suc;
		if (N == g_first_node)
			break;
	}
	N1->node_next->node_degree++;
	N1->node_degree++;
	Sum += N1->node_next_cost;
	g_tree_norm = 0;
	do
	g_tree_norm += N->node_degree * N->node_degree;
	while ((N = N->node_suc) != g_first_node);
	if (N1 != g_first_node) {
		g_first_node->node_tree_parent = 0;
		tsp_node_precede(N1, g_first_node);
		g_first_node = N1;
	}
	else
	{
		N1->node_suc->node_tree_parent = 0;
	}
	if (g_tree_norm == 0)
	{
		for (N = g_first_node->node_tree_parent; N; N1 = N, N = N->node_tree_parent)
			tsp_node_follow(N, N1);
	}
	return Sum;
}
void tsp_solver::tsp_generate_cands(const int cand_max_num, const int MaxAlpha, bool Symmetric)
{
	tsp_node *From, *edge_to;
	cand_edge_iter NFrom, edge_nn, NTo;
	int a, d, Count;
	From = g_first_node;
	while (true)
	{
		if (From->node_cand_set.empty())
		{
			From->node_cand_set.clear();
		}
		From->node_next = 0;
		From = From->node_suc;
		if (From == g_first_node)
			break;
	};
	if (cand_max_num <= 0)
		return;
	From = g_first_node;
	while (true)
	{
		From->node_cand_set.resize(cand_max_num + 1);
		From = From->node_suc;
		if (From == g_first_node)
			break;
	};
	while (true)
	{
		NFrom = From->node_cand_set.begin();
		NFrom->edge_to = 0;
		if (From != g_first_node) {
			From->node_next_cost = INT_MIN;
			edge_to = From;
			for (; edge_to->node_tree_parent != 0; edge_to = edge_to->node_tree_parent)
			{
				if (!tsp_fixed(edge_to, edge_to->node_tree_parent))
					edge_to->node_tree_parent->node_next_cost = std::max(edge_to->node_next_cost, edge_to->node_cost);
				else
					edge_to->node_tree_parent->node_next_cost = edge_to->node_next_cost;
				edge_to->node_tree_parent->node_next = From;
			}
		}
		Count = 0;
		edge_to = g_first_node;
		do {
			if (edge_to == From)
				continue;
			d = tsp_distance(From, edge_to);
			if (From == g_first_node)
			{
				a = 0;
				if (edge_to != From->node_tree_parent)
					a = d - From->node_next_cost;
			}
			else if (edge_to == g_first_node)
			{
				a = 0;
				if (From != edge_to->node_tree_parent)
					a = d - edge_to->node_next_cost;
			}
			else {
				if (edge_to->node_next != From)
				{
					if (!tsp_fixed(edge_to, edge_to->node_tree_parent))
						edge_to->node_next_cost = std::max(edge_to->node_tree_parent->node_next_cost, edge_to->node_cost);
					else
						edge_to->node_next_cost = edge_to->node_tree_parent->node_next_cost;
				}
				a = d - edge_to->node_next_cost;
			}
			if (tsp_fixed(From, edge_to))
				a = INT_MIN;
			else {
				if (edge_to->node_next_cost == INT_MIN ||
					tsp_invalid(From, edge_to))
					continue;
			}
			if (a <= MaxAlpha) {
				edge_nn = NFrom;
				while (true)
				{
					--edge_nn;
					if (edge_nn < From->node_cand_set.begin()) break;
					bool flag = a > edge_nn->edge_alpha || (a == edge_nn->edge_alpha && d >= edge_nn->edge_cost);
					if (flag)
						break;
					*(edge_nn + 1) = *edge_nn;
				}
				edge_nn++;
				(edge_nn->edge_cost = d, edge_nn->edge_to = edge_to, edge_nn->edge_alpha = a);
				if (Count < cand_max_num)
				{
					Count++; NFrom++;
				}
				NFrom->edge_to = 0;
			}
		} while ((edge_to = edge_to->node_suc) != g_first_node);
		From = From->node_suc;
		if (From == g_first_node)
			break;
	};
	if (!Symmetric)
		return;
	edge_to = g_first_node;
	while (true)
	{
		for (NTo = edge_to->node_cand_set.begin();; NTo++)
		{
			From = NTo->edge_to;
			if (!From)break;
			Count = 0;
			for (edge_nn = NFrom = From->node_cand_set.begin();; edge_nn++)
			{
				if (!edge_nn->edge_to || edge_nn->edge_to == edge_to) break;
				Count++;
			}
			if (!edge_nn->edge_to)
			{
				a = NTo->edge_alpha;
				d = NTo->edge_cost;
				for (; --edge_nn >= NFrom;)
				{
					bool flag = a > edge_nn->edge_alpha || (a == edge_nn->edge_alpha && d >= edge_nn->edge_cost);
					if (flag)
						break;
					*(edge_nn + 1) = *edge_nn;
				}
				edge_nn++;
				edge_nn->edge_to = edge_to;
				edge_nn->edge_cost = d;
				edge_nn->edge_alpha = a;
				From->node_cand_set.resize(Count + 2);
				From->node_cand_set[Count + 1].edge_to = 0;
			}
		}
		edge_to = edge_to->node_suc;
		if (edge_to == g_first_node)
			break;
	};
}
int tsp_solver::tsp_ascent()
{
	tsp_node *t;
	int BestW, W, W0;
	int T;
	t = g_first_node;
	while (true)
	{
		t->node_best_penalty = t->node_penalty = 0;
		t = t->node_suc;
		if (t == g_first_node)break;
	}
	W = tsp_min_tree_cost(false);
	if (!g_tree_norm)
		return W;
	tsp_generate_cands(g_ascent_cand_max, INT_MAX, 1);
	t = g_first_node;
	do
	t->node_last_degree = t->node_degree;
	while ((t = t->node_suc) != g_first_node);
	BestW = W0 = W;
	int InitialPhase = 1;
	int InitialPeriod = g_tsp_dim / 2;
	if (InitialPeriod < 100)
		InitialPeriod = 100;
	for (int Period = InitialPeriod, T = 1;; Period /= 2, T /= 2)
	{
		if (!(Period > 0 && T > 0 && g_tree_norm != 0))break;
		for (int P = 1;; P++)
		{
			if (!(T && P <= Period && g_tree_norm != 0))
				break;
			t = g_first_node;
			while (true)
			{
				if (t->node_degree != 0)
					t->node_penalty += T * (7 * t->node_degree + 3 * t->node_last_degree) / 10;
				t->node_last_degree = t->node_degree;
				t = t->node_suc;
				if (t == g_first_node)
					break;
			};
			W = tsp_min_tree_cost(true);
			if (W > BestW)
			{
				if (W > 2 * W0 && g_ascent_cand_max < g_tsp_dim)
				{
					W = tsp_min_tree_cost(false);
					if (W < W0)
					{
						if ((g_ascent_cand_max *= 2) > g_tsp_dim)
							g_ascent_cand_max = g_tsp_dim;
						return tsp_ascent();
					}
					W0 = W;
				}
				BestW = W;
				t = g_first_node;
				while (true)
				{
					t->node_best_penalty = t->node_penalty;
					t = t->node_suc;
					if (t == g_first_node)break;
				}
				if (InitialPhase)
					T *= 2;
				if (P == Period && (Period *= 2) > InitialPeriod)
					Period = InitialPeriod;
			}
			else
			{
				bool flag = InitialPhase && P > Period / 2;
				if (flag)
				{
					InitialPhase = 0;
					P = 0;
					T = 3 * T / 4;
				}
			}
		}
	}
	t = g_first_node;
	do {
		if (!t->node_cand_set.empty())
		{
			t->node_cand_set.clear();

		}
		t->node_penalty = t->node_best_penalty;
	} while ((t = t->node_suc) != g_first_node);
	W = tsp_min_tree_cost(0);
	return W;
}
void tsp_solver::tsp_choose_initial_tour()
{
	tsp_node *N, *NextN, *FirstAlternative;
	cand_edge_iter NN;
	int i;
	N = g_first_node = &g_node_set[1 + rand() % g_tsp_dim];

	while (true)
	{
		N->node_degree = 0;
		N = N->node_suc;
		if (N == g_first_node) break;
	}


	g_first_node->node_degree = 1;
	while (N->node_suc != g_first_node)
	{

		for (NN = N->node_cand_set.begin();; NN++)
		{
			NextN = NN->edge_to;
			if (!NextN) break;
			if (!NextN->node_degree && tsp_fixed(N, NextN))
				break;
		}

		if (NextN == 0)
		{
			FirstAlternative = 0;
			i = 0;
			for (NN = N->node_cand_set.begin(); NextN = NN->edge_to; NN++) {
				if (!NextN->node_degree  &&
					NN->edge_alpha == 0 && tsp_is_best_tour(N, NextN)) {
					i++;
					NextN->node_next = FirstAlternative;
					FirstAlternative = NextN;
				}
			}
			if (i == 0) {
				for (NN = N->node_cand_set.begin(); ; NN++) 
				{
					NextN = NN->edge_to;
					if (!NextN)
						break;

					if (!NextN->node_degree) {
						i++;
						NextN->node_next = FirstAlternative;
						FirstAlternative = NextN;
					}
				}
			}
			if (i != 0)
			{
				NextN = FirstAlternative;
				if (i > 1) {
					i = rand() % i;
					while (i--)
						NextN = NextN->node_next;
				}
			}
			else
			{
				NextN = N->node_suc;
				while (tsp_invalid(N, NextN) && NextN->node_suc != g_first_node)
					NextN = NextN->node_suc;
			}

		}
		tsp_node_follow(NextN, N);
		N = NextN;
		N->node_degree = 1;
	}
	N = g_first_node;
}
int tsp_solver::tsp_lk()
{
	tsp_node *t1, *t2, *SUCt1;
	int Gain, gain0, i;
	int Cost;
	tsp_segment *S;
	time_t Last_time = time(0);
	g_tsp_reversed = 0;
	S = g_first_segment;
	i = 0;
	while (true)
	{
		S->segment_size = 0;
		S->segment_priority = ++i;
		S->segment_reversed = 0;
		S->segment_first_node = S->segment_last_node = 0;
		S = S->segment_suc;
		if (S == g_first_segment)break;
	};
	i = 0;
	g_cost_hash = 0;
	while (!g_tsp_swaps.empty())
		g_tsp_swaps.pop();
	g_active_nodes.clear();
	Cost = 0;
	t1 = g_first_node;
	while (true)
	{
		t2 = t1->node_old_suc = t1->node_next = t1->node_suc;
		t1->node_priority = ++i;
		t1->node_old_pre = t1->node_pre;
		Cost += tsp_distance(t1, t2) - t1->node_penalty - t2->node_penalty;
		t1->node_cost = INT_MAX;
		g_cost_hash ^= tsp_rand_nums[t1->node_id] * tsp_rand_nums[t2->node_id];
		for (cand_edge_iter Nt1 = t1->node_cand_set.begin();; Nt1++)
		{
			t2 = Nt1->edge_to;
			if (!t2) break;
			if (t2 != t1->node_pre && t2 != t1->node_suc && Nt1->edge_cost < t1->node_cost)
				t1->node_cost = Nt1->edge_cost;
		}
		t1->node_segment_parent = S;
		S->segment_size++;
		if (S->segment_size == 1)
			S->segment_first_node = t1;
		S->segment_last_node = t1;
		if (S->segment_size == tsp_group_size)
			S = S->segment_suc;
		t1->node_old_pre_excluded = t1->node_old_suc_excluded = false;
		t1->node_next = 0;
		tsp_activate_node(t1);
		t1 = t1->node_suc;
		if (t1 == g_first_node)
			break;
	};
	if (g_cost_hashtable.search(g_cost_hash, Cost))
		return Cost;
	do {
		while (t1 = tsp_remove_first_active()) {
			SUCt1 = tsp_suc_node(t1);
			bool tonext = false;
			for (int X2 = 1; X2 <= 2; X2++) {
				t2 = X2 == 1 ? tsp_pre_node(t1) : SUCt1;
				if (tsp_near(t1, t2) || tsp_fixed(t1, t2))
					continue;
				gain0 = tsp_distance(t1, t2);
				while (t2 = tsp_best_3opt_move(t1, t2, &gain0, &Gain))
				{
					if (Gain > 0) {
						Cost -= Gain;
						tsp_store_tour();
						if (g_cost_hashtable.search(g_cost_hash, Cost))
						{
							tsp_normalize_nodelist();
							return Cost;
						}
						tsp_activate_node(t1);
						tonext = true;
						break;
					}
				}
				if (tonext) break;
				tsp_restore_tour();
			}
		}
		if (!g_cost_hashtable.search(g_cost_hash, Cost))
			g_cost_hashtable.insert(g_cost_hash, Cost);
		if ((Gain = tsp_gain32()) > 0) {
			Cost -= Gain;
			tsp_store_tour();
			if (g_cost_hashtable.search(g_cost_hash, Cost))
			{
				tsp_normalize_nodelist();
				return Cost;
			}
		}
	} while (Gain > 0);
	tsp_normalize_nodelist();
	return Cost;
}
int tsp_solver::tsp_find_tour()
{
	int cost;
	tsp_node *t;
	time_t Last_time = time(0);
	t = g_first_node;
	do
	t->node_old_pre = t->node_old_suc = t->node_next_best_suc = t->node_best_suc = 0;
	while ((t = t->node_suc) != g_first_node);
	g_cost_hashtable.init();
	int better_cost = INT_MAX;
	for (int t = 0; t < g_tsp_dim; t++) {
		tsp_choose_initial_tour();
		cost = tsp_lk();
		if (cost < better_cost) {
			better_cost = cost;
			tsp_record_better();
			tsp_adjust_cand_set();
			g_cost_hashtable.init();
			g_cost_hashtable.insert(g_cost_hash, cost);
		}
	}
	tsp_reset_cand_set();
	return better_cost;
}
void tsp_solver::tsp_init_data(int n, int dist[MAX_TSP_N][MAX_TSP_N])
{
	int i;
	g_tsp_dim = n * 2;
	tsp_node *N;
	N = g_first_node;
	tsp_node *Ni, *Nj;
	int  j, W;
	if (!g_first_node)
	{
		tsp_node *Prev, *N;
		int i;
		g_node_set = new tsp_node[g_tsp_dim + 1];
		for (i = 1; i <= g_tsp_dim; i++, Prev = N) {
			N = &g_node_set[i];
			if (i == 1)
				g_first_node = N;
			else
				tsp_node_link(Prev, N);
			N->node_id = i;
		}
		tsp_node_link(N, g_first_node);
	}
	n = g_tsp_dim / 2;
	for (Ni = g_first_node; Ni->node_id <= n; Ni = Ni->node_suc)
		Ni->node_cost_row.resize(n + 1);
	for (i = 1; i <= n; i++) {
		Ni = &g_node_set[i];
		for (j = 1; j <= n; j++) {
			W = dist[i - 1][j - 1];
			if (W == 0) W = 0x3F3F3F3F;
			Ni->node_cost_row[j] = W;
		}
		Nj = &g_node_set[i + n];
		if (!Ni->node_fixto)
			Ni->node_fixto = Nj;
		if (!Nj->node_fixto)
			Nj->node_fixto = Ni;
	}
	tsp_best_tour.resize(g_tsp_dim + 1);
	tsp_better_tour.resize(g_tsp_dim + 1);
	g_bin_heap = new bin_heap(g_tsp_dim);
	while (!g_tsp_swaps.empty())
		g_tsp_swaps.pop();
	tsp_rand_nums.resize(g_tsp_dim + 1);
	srand(1);
	for (i = 1; i <= g_tsp_dim; i++)
		tsp_rand_nums[i] = rand();
	g_cost_hashtable.init();
	srand(1);
	if (g_cand_max_num > g_tsp_dim)
		g_cand_max_num = g_tsp_dim;
	if (g_ascent_cand_max > g_tsp_dim)
		g_ascent_cand_max = g_tsp_dim;
	tsp_group_size = sqrt(1.0 * g_tsp_dim);
	tsp_groups = 0;
	tsp_segment *segment, *segment_pre;
	for (i = g_tsp_dim, segment_pre = 0; i > 0; i -= tsp_group_size, segment_pre = segment) {
		segment = new tsp_segment;
		segments.push_back(segment);
		segment->segment_priority = ++tsp_groups;
		if (!segment_pre)
			g_first_segment = segment;
		else
		{
			segment_pre->segment_suc = segment;
			segment->segment_pre = segment_pre;
		}
	}
	segment->segment_suc = g_first_segment;
	g_first_segment->segment_pre = segment;
}
int tsp_solver::solve(int n, int dist[MAX_TSP_N][MAX_TSP_N], int result[MAX_TSP_N])
{
	int min_weight = INT_MAX;
	tsp_init_data(n, dist);
	g_cand_max_num = n;
	g_ascent_cand_max = n;
	int cost;
	cost = tsp_ascent();
	if (g_tree_norm != 0) {
		tsp_generate_cands(g_cand_max_num, fabs(cost*1.0 / g_tsp_dim), 0);
		int weight = tsp_find_tour();
		if (weight < min_weight) {
			tsp_best_tour = tsp_better_tour;
			min_weight = weight;
		}
	}
	else {
		tsp_record_better();
		tsp_best_tour = tsp_better_tour;
		min_weight = cost;
	}
	for (int i = 1; i <= g_tsp_dim / 2; i++)
	{
		result[i - 1] = tsp_best_tour[i] - 1;
	}
	return min_weight;
}
tsp_solver::tsp_solver()
{
	g_first_node = 0;
}
tsp_solver::~tsp_solver()
{
	if (g_node_set)
		delete[] g_node_set;
	for (int i = 0; i < segments.size(); i++)
		delete segments[i];
}