#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pixels_px.c>

size_t min_size(size_t x, size_t m) {
	return x < m ? m : x;
}

#define FUNC_VEC(name, inner_type, m0, m1, m2, ...)						\
	inner_type * add_##inner_type(name *x, __VA_ARGS__) {					\
		if (x->len >= x->max)								\
			px_assert(								\
				x->x = realloc(							\
					x->x,							\
					(x->max = min_size(x->len * 3, 64)) * sizeof(inner_type)\
				),								\
				ERROR"realloc"							\
			);									\
												\
		inner_type *v = &x->x[x->len++];						\
												\
		*v = (inner_type){ m0, m1, m2 };						\
												\
		return v;									\
	}

#define INIT_VEC(name, inner_type, m0, m1, m2, ...)	\
	typedef struct {				\
		inner_type *x;				\
		size_t len, max;			\
	} name;						\
							\
	FUNC_VEC(name, inner_type, m0, m1, m2, __VA_ARGS__)


#ifdef VERTEX
// Easily specify vertices
typedef struct {
	D2 pos;
	size_t neighbor;
} Vertice;


INIT_VEC(
	Vertex,
	Vertice,
	*pos,
	neighbor,,
	D2 *pos,
	size_t neighbor
)
#endif

#ifdef PIXELS
#include <pixels_px.c>


FUNC_VEC(
	Pixels,
	Px,
	*pos,
	*color,,
	D2 *pos,
	RGBA *color
)
#endif

#ifdef SHAPES
// Just a shape :P
typedef struct {
	D2	pos,
		size;
	int type;
} Shape;


INIT_VEC(
	Shapes,
	Shape,
	*pos,
	*size,
	type,
	D2 *pos,
	D2 *size,
	int type
)
#endif


// Root: Most important
typedef struct {
#ifdef VERTEX
	Vertex vertex;
#endif
#ifdef PIXELS
	Pixels pixels;
#endif
#ifdef SHAPES
	Shapes shapes;
#endif

	D2	pos,
		size;

	RGBA color;
} Instance;


INIT_VEC(
	Instances,
	Instance,
	.color = *color,
	.pos   = *pos,
	.size  = *size,
	D2 *pos,
	D2 *size,
	RGBA *color
)


// Terminal window size (grid)
void set_ws(D2 *ws) {
	struct winsize w;

	px_assert(ioctl(0, TIOCGWINSZ, &w) != -1, ERROR"ioctl"),
	ws->y = w.ws_row,
	ws->x = w.ws_col;
}

void resize_instance(Instance *x, D2 *new) {
	x->size.x = new->x,
	x->size.y = new->y,

	resize_pixels(&x->pixels, new);
}


typedef struct {
	D2 ws;
	Instances x;
} Tegrine;

void free_instance(Instance *x) {
#ifdef VERTEX
	if (x->vertex.x)
		free(x->vertex.x);
#endif

#ifdef PIXELS
	if (x->pixels.x)
		free(x->pixels.x);
#endif

#ifdef SHAPES
	if (x->shapes.x)
		free(x->shapes.x);
#endif

	*x = (Instance){ };
}

// Deallocate
void free_tegrine(Tegrine *x) {
	for(size_t i = 0; i < x->x.len; ++i)
		free_instance(&x->x.x[i]);

	free(x->x.x);
}

void remove_instance(Tegrine *x, size_t i) {
	if (i >= x->x.len)
		return;

	// Shifting to the left
	free_instance(&x->x.x[i]),
	memcpy(
		x->x.x		+ i,
		x->x.x		+ i + 1,
		--x->x.len	- i * sizeof(Instance)
	);
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

#define DELETED 999999999

#include <tegrine/render/mod.c>
