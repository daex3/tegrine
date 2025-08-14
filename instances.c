#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pixels_px.c>


int min_size(int x, int m) {
	return x < m ? m : x;
}


#define DOT		\
	D2	pos;	\
			\
	RGBA color;


#ifdef VERTEX
typedef struct {
	DOT

	int neighbor;
} Vertice;
#endif


#ifdef SHAPES
typedef struct {
	DOT

	D2 size;
	int n;
} Shape;
#endif


#define FUNC_VEC(name, inner_type, free_e, m1, m2, m3, m4, ...)								\
	static void realloc_##name(name *x) {										\
		if (x->len >= x->max)											\
			px_assert(											\
				(x->x = realloc(									\
					x->x,										\
					(x->max = min_size(								\
						x->len * 3,								\
						64									\
					)) * sizeof(inner_type))							\
				),											\
				ERROR"realloc"										\
			);												\
	}														\
															\
	inner_type * add_##inner_type(name *x, __VA_ARGS__) {								\
		realloc_##name(x);											\
															\
		inner_type *p = &x->x[x->len++];									\
															\
		*p = (inner_type){ };											\
															\
		m1;													\
		m2;													\
		m3;													\
		m4;													\
															\
		return p;												\
	}														\
															\
	void remove_##inner_type(name *x, int i) {									\
		free_e;													\
		/* Fuck double free now i'd rather SEGFAULT */								\
		x->x[i] = (inner_type){ };										\
															\
		memmove(i + x->x, i + x->x + 1, (--x->len - i) * sizeof(inner_type));					\
	}

#define INIT_VEC(name, inner_type, free_e, m1, m2, m3, m4, ...)	\
	typedef struct {					\
		inner_type *x;					\
		int len, max;				\
	} name;							\
								\
	FUNC_VEC(name, inner_type, free_e, m1, m2, m3, m4, __VA_ARGS__)

#ifdef PIXELS
FUNC_VEC(Pixels,	Px,,		p->pos = *pos, p->color = *color,,,				D2 *pos, RGBA *color)
#endif
#ifdef VERTEX
INIT_VEC(Vertex,	Vertice,,	p->pos = *pos, p->color = *color, p->neighbor = neighbor,,	D2 *pos, RGBA *color, int neighbor)
#endif
#ifdef SHAPES
INIT_VEC(Shapes,	Shape,,		p->pos = *pos, p->color = *color, p->n = n, p->size = *size,	D2 *pos, D2 *size, RGBA *color, int n)
#endif

// Forward declaration can't fix this chaos :)
typedef struct {
	D2	pos,
		size;

#ifdef PIXELS
	Pixels pixels;
#endif
#ifdef VERTEX
	Vertex vertex;
#endif
#ifdef SHAPES
	Shapes shapes;
#endif
} Instance;

void free_instance(Instance *x) {
#ifdef PIXELS
	free(x->pixels.x),
#endif
#ifdef VERTEX
	free(x->vertex.x),
#endif
#ifdef SHAPES
	free(x->shapes.x),
#endif

	*x = (Instance){ };
}

INIT_VEC(Instances,	Instance,	free_instance,	p->pos = *pos, p->size = *size,,,		D2 *pos, D2 *size)


// Terminal window size (grid)
void set_ws(D2 *ws) {
	struct winsize w;

	px_assert(ioctl(0, TIOCGWINSZ, &w) != -1, ERROR"ioctl"),
	ws->y = w.ws_row,
	ws->x = w.ws_col;
}

void resize_instance(Instance *x, D2 *new) {
	x->size.x = new->x,
	x->size.y = new->y;

#ifdef PIXELS
	resize_pixels(&x->pixels, new);
#endif
}


typedef struct {
	D2 ws;
	Instances x;
} Tegrine;

// Deallocate
void free_tegrine(Tegrine *x) {
	Instances *y = &x->x;

	for(int i = 0; i < y->len; ++i)
		free_instance(y->x);

	free(y->x),
	*y = (Instances){ };
	// :shrugh:
	//*x = (Tegrine){ };
}


#ifdef VERTEX
void draw_vertex(D2 *, Vertex *, Instance *);
#endif
#ifdef PIXELS
void draw_pixels(D2 *, Pixels *, Instance *);
#endif
#ifdef SHAPES
void draw_shapes(D2 *, Shapes *, Instance *);
#endif


// Initialize dinamically allocated structure
#define SET(s, size, type)				\
	s x   = calloc((s max = size), sizeof(type)),	\
	s len = 0,					\
	px_assert(s x, ERROR"calloc")

#include <tegrine/render/mod.c>
