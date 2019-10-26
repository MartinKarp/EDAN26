#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "set.h"
#include "error.h"
#include "list.h"

typedef struct vertex_t	vertex_t;
typedef struct task_t	task_t;

/* cfg_t: a control flow graph. */
struct cfg_t {
	size_t			nvertex;	/* number of vertices		*/
	size_t			nsymbol;	/* width of bitvectors		*/
	vertex_t*		vertex;		/* array of vertex		*/
};

struct thread_args {
	cfg_t* 			cfg;
	size_t			indx;
	size_t 			nthread;
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
	size_t			index;		/* can be used for debugging	*/
	set_t*			set[NSETS];	/* live in from this vertex	*/
	set_t*			prev;		/* alternating with set[IN]	*/
	size_t			nsucc;		/* number of successor vertices */
	vertex_t**		succ;		/* successor vertices 		*/
	list_t*			pred;		/* predecessor vertices		*/
	bool			listed;		/* on worklist			*/
	pthread_mutex_t lock;
	pthread_mutex_t rlock;
};

static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);

//my functions
void* work(void* arg);

cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
	size_t		i;
	cfg_t*		cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (cfg == NULL)
		error("out of memory");

	cfg->nvertex = nvertex;
	cfg->nsymbol = nsymbol;

	cfg->vertex = calloc(nvertex, sizeof(vertex_t));
	if (cfg->vertex == NULL)
		error("out of memory");

	for (i = 0; i < nvertex; i += 1)
		init_vertex(&cfg->vertex[i], i, nsymbol, max_succ);

	return cfg;
}

static void clean_vertex(vertex_t* v)
{
	int		i;

	for (i = 0; i < NSETS; i += 1)
		free_set(v->set[i]);
	free_set(v->prev);
	free(v->succ);
	free_list(&v->pred);
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
	int		i;

	v->index	= index;
	v->succ		= calloc(max_succ, sizeof(vertex_t*));
	pthread_mutex_init(&v->lock, NULL);
	pthread_mutex_init(&v->rlock, NULL);

	if (v->succ == NULL)
		error("out of memory");

	for (i = 0; i < NSETS; i += 1)
		v->set[i] = new_set(nsymbol);

	v->prev = new_set(nsymbol);
}

void free_cfg(cfg_t* cfg)
{
	size_t		i;

	for (i = 0; i < cfg->nvertex; i += 1)
		clean_vertex(&cfg->vertex[i]);
	free(cfg->vertex);
	free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
	vertex_t*	u;
	vertex_t*	v;

	u = &cfg->vertex[pred];
	v = &cfg->vertex[succ];

	u->succ[u->nsucc++ ] = v;
	insert_last(&v->pred, u);
}

bool testbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	return test(cfg->vertex[v].set[type], index);
}

void setbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	set(cfg->vertex[v].set[type], index);
}

void or_opt(set_t* t,set_t* a, set_t* b)
{
	size_t	i;
	//uint64_t temp;

	//maybe add if statement?
	//maybe do fetch or instead
	//if statement breaks implementation in some way
	for (i = 0; i < t->n; ++i){
		if(~t->a[i]){
			t->a[i] |= b->a[i];
		}
	}
}

void propagate_opt(set_t* in, set_t* out, set_t* def, set_t* use)
{
	size_t	i;

	for (i = 0; (i + 3) < in->n; i += 4){
		in->a[i] = (out->a[i] & ~def->a[i]) | use->a[i];
		in->a[i+1] = (out->a[i+1] & ~def->a[i+1]) | use->a[i+1];
		in->a[i+2] = (out->a[i+2] & ~def->a[i+2]) | use->a[i+2];
		in->a[i+3] = (out->a[i+3] & ~def->a[i+3]) | use->a[i+3];
	}
	for (; i< in->n; ++i){
		in->a[i] = (out->a[i] & ~def->a[i]) | use->a[i];
	}

}
void 	insert_last_opt(list_t**, void*);
void*	remove_first_opt(list_t**);



void insert_last_opt(list_t** list1, void *p)
{
	list_t*		tmp;
	list_t*		list2;

	list2 = new_list(p);

	if (*list1 == NULL)
		*list1 = list2;
	else if (list2 != NULL) {
		(*list1)->pred->succ = list2;
		list2->pred->succ = *list1;
		tmp	= (*list1)->pred;
		(*list1)->pred = list2->pred;
		list2->pred = tmp;
	}
}
void* remove_first_opt(list_t** list)
{
	void*		data;
	list_t*		p;

	if (*list == NULL)
		return NULL;
	p = *list;
	data = p->data;

	if (*list == (*list)->succ)
		*list = NULL;
	else
		*list = p->succ;
	delete_list(p);

	return data;
}

void liveness(cfg_t* cfg)
{
	size_t		i;
	//int 		result;
	int nthread = 8;
	pthread_t thread[nthread];
	struct thread_args cfgs[nthread];

	printf("Inlined version with if\n");
	for (i = 0; i < nthread; ++i){
		cfgs[i].cfg = cfg;
		cfgs[i].nthread = nthread;
		cfgs[i].indx = i;
		//printf("IN MAIN: Creating thread %lu.\n", i);

		pthread_create(&thread[i], NULL, work, &cfgs[i]);
		//printf("%lu.\n", nvthread * i);
		//printf("%lu.\n", cfgs[i].nvertex);
		//assert(!result);
	}
	for (i = 0; i < nthread; i ++){
		pthread_join(thread[i], NULL);
		//assert(!result);
		//printf("IN MAIN: Thread %lu has ended.\n", i);
	}
}

void* work(void *arg){
	struct thread_args* args = (struct thread_args*)arg;
	cfg_t* cfg = args->cfg;
	vertex_t*	u;
	vertex_t*	v;
	set_t*		prev;
	size_t		j;
	list_t*		p;
	list_t*		h;
	list_t* 	worklist;

	worklist = NULL;

	for (j = args->indx; j < cfg->nvertex; j += args->nthread) {
		u = &cfg->vertex[j];
		pthread_mutex_lock(&u->lock);
		insert_last(&worklist, u);
		u->listed = true;
		pthread_mutex_unlock(&u->lock);
	}

	while ((u = remove_first_opt(&worklist)) != NULL) {
		pthread_mutex_lock(&u->lock);
		//printf("%lu \n", j);
		u->listed = false;

		reset(u->set[OUT]);
		//What to do? possible data race reading successors
		for (j = 0; j < u->nsucc; ++j) {
			pthread_mutex_lock(&(u->succ[j])->rlock);
			or_opt(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
			pthread_mutex_unlock(&(u->succ[j])->rlock);
		}
		pthread_mutex_lock(&u->rlock);
		prev = u->prev;
		u->prev = u->set[IN];
		u->set[IN] = prev;

		/* in our case liveness information... */
		propagate_opt(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

		pthread_mutex_unlock(&u->rlock);
		if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
			p = h = u->pred;
			pthread_mutex_unlock(&u->lock);
			do {
				v = p->data;
				pthread_mutex_lock(&v->lock);
				if (!v->listed) {
					v->listed = true;
					insert_last_opt(&worklist, v);
				}
				pthread_mutex_unlock(&v->lock);
				p = p->succ;
			} while (p != h);
		} else{
			pthread_mutex_unlock(&u->lock);
		}
	}
	return NULL;
}

void print_sets(cfg_t* cfg, FILE *fp)
{
	size_t		i;
	vertex_t*	u;

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
		fprintf(fp, "use[%zu] = ", u->index);
		print_set(u->set[USE], fp);
		fprintf(fp, "def[%zu] = ", u->index);
		print_set(u->set[DEF], fp);
		fputc('\n', fp);
		fprintf(fp, "in[%zu] = ", u->index);
		print_set(u->set[IN], fp);
		fprintf(fp, "out[%zu] = ", u->index);
		print_set(u->set[OUT], fp);
		fputc('\n', fp);
	}
}
