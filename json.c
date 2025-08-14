#include <cjson/cJSON.h>
#include <string.h>
#include <sys/stat.h>

static char * read_file(FILE *f) {
	px_assert(!fseek(f, 0, SEEK_END), ERROR"fseek");

	long len = ftell(f);
	char *s = malloc(len);

	px_assert(len != -1, ERROR"ftell"),
	px_assert(s, ERROR"malloc"),
	px_assert(!fseek(f, 0, SEEK_SET), ERROR"fseek"),

	px_assert(fread(s, len, 1, f) != -1U, ERROR"fread"),

	fclose(f);

	return s;
}

static FILE * open_saved(char *message, char *name, char *mode) {
	char	*home		= getenv("HOME"),
		 path[1024]	= { };

	px_assert(home, ERROR"getenv(HOME)"),

	strlcat(path, home, 1024),
	strlcat(path, "/.tegrine", 1024),
	mkdir(path, 0700),
	strlcat(path, "/saved/", 1024),
	mkdir(path, 0700),
	strlcat(path, name, 1024);

	FILE *f = fopen(path, mode);

	px_assert(f, ERROR"fopen");

	return f;
}

static void write_color(RGBA *x, cJSON *r) {
	if (x->r || x->g || x->b || x->a) {
		cJSON *color = cJSON_CreateObject();

		if (x->r)
			cJSON_AddNumberToObject(color, "r", x->r);

		if (x->g)
			cJSON_AddNumberToObject(color, "g", x->g);

		if (x->b)
			cJSON_AddNumberToObject(color, "b", x->b);

		if (x->a)
			cJSON_AddNumberToObject(color, "a", x->a);

		cJSON_AddItemToObject(r, "color", color);
	}
}

static void write_d2(D2 *x, cJSON *r, char *name) {
	if (x->x || x->y) {
		cJSON *p = cJSON_CreateObject();

		if (x->x)
			cJSON_AddNumberToObject(p, "x", x->x);

		if (x->y)
			cJSON_AddNumberToObject(p, "y", x->y);

		cJSON_AddItemToObject(r, name, p);
	}
}

/*
	x:	[ .. ],
	len:	Number
*/
#define WRITE_JSON_VEC(ve, type, inner_t, ...)			\
	if (ve.x) {						\
		cJSON	*ve_r	= cJSON_CreateObject(),		\
			*ve_x	= cJSON_CreateArray();		\
		type	*x	= &ve;				\
								\
		for(size_t i = 0; i < x->len; ++i) {		\
			cJSON	*v_r	= cJSON_CreateObject();	\
			inner_t	*v	= &x->x[i];		\
								\
			write_d2(&v->pos, v_r, "pos"),		\
			write_color(&v->color, v_r);		\
								\
			__VA_ARGS__;				\
								\
			cJSON_AddItemToArray(ve_x, v_r);	\
		}						\
								\
		cJSON_AddItemToObject(ve_r, "x", ve_x),	\
		cJSON_AddNumberToObject(ve_r, "len", x->len),	\
								\
		cJSON_AddItemToObject(ins_r, #type, ve_r);	\
	}

void save_to_json(Tegrine *te, char *name) {
	/*
		x:	Instance [ .. ],
		len:	Number
	*/
	cJSON	*root	= cJSON_CreateObject(),
		*x	= cJSON_CreateArray();

	for(size_t i = 0; i < te->x.len; ++i) {
		cJSON	 *ins_r	= cJSON_CreateObject(),
			 *pos	= NULL,
			 *color	= NULL;
		Instance *ins	= &te->x.x[i];

		write_d2(&ins->pos, ins_r, "pos"),
		write_d2(&ins->size, ins_r, "size");

#ifdef VERTEX
		WRITE_JSON_VEC(
			ins->vertex,
			Vertex,
			Vertice,
			if (v->neighbor < x->len)
				cJSON_AddNumberToObject(
					v_r,
					"neighbor",
					v->neighbor
				)
		)
#endif

#ifdef PIXELS
		WRITE_JSON_VEC(ins->pixels, Pixels, Px)
#endif

#ifdef SHAPES
		WRITE_JSON_VEC(
			ins->shapes,
			Shapes,
			Shape,
			if (v->n)
				cJSON_AddNumberToObject(
					v_r,
					"n",
					v->n
				),
			write_d2(&v->size, v_r, "size")
		)
#endif

		cJSON_AddItemToArray(x, ins_r);
	}

	cJSON_AddItemToObject(root, "x", x),
	cJSON_AddNumberToObject(root, "len", te->x.len);

	char *s = cJSON_Print(root);
	FILE *f = open_saved("JSON file to save as: ", name, "w");

	px_assert(fputs(s, f) != -1, ERROR"fputs"),
	fclose(f),
	cJSON_free(s),
	cJSON_Delete(root);
}

static void read_color(RGBA *x, cJSON *r) {
	cJSON *o = cJSON_GetObjectItem(r, "color");

	if (o) {
		cJSON	*o_r = cJSON_GetObjectItem(o, "r"),
			*o_g = cJSON_GetObjectItem(o, "g"),
			*o_b = cJSON_GetObjectItem(o, "b"),
			*o_a = cJSON_GetObjectItem(o, "a");

		if (o_r)
			x->r = o_r->valueint;

		if (o_g)
			x->g = o_g->valueint;

		if (o_b)
			x->b = o_b->valueint;

		if (o_a)
			x->a = o_a->valueint;
	}
}

static void read_d2(D2 *x, cJSON *r, char *name) {
	cJSON *p = cJSON_GetObjectItem(r, name);

	if (p) {
		cJSON	*p_x = cJSON_GetObjectItem(p, "x"),
			*p_y = cJSON_GetObjectItem(p, "y");

		if (p_x)
			x->x = p_x->valueint;

		if (p_y)
			x->y = p_y->valueint;
	}
}

#define READ_JSON_ERROR ERROR"Broken JSON bullshit"

#define READ_JSON_VEC(px, type, inner_t, e, ...) {					\
	cJSON	*ve_r	= cJSON_GetObjectItem(ins_x, #type),				\
		*ve_x	= cJSON_GetObjectItem(ve_r, "x"),				\
		*ve_len	= cJSON_GetObjectItem(ve_r, "len"),				\
		*v_r;									\
											\
	if (ve_x && ve_len) {								\
		type *ve = &px;								\
											\
		SET(ve->, ve_len->valueint, type);					\
											\
		cJSON_ArrayForEach(v_r, ve_x) {						\
			inner_t *v = &ve->x[ve->len++];					\
											\
			read_d2(&v->pos, v_r, "pos"),					\
			read_color(&v->color, v_r);					\
											\
			e;								\
			__VA_ARGS__;							\
		}									\
	}										\
}

void load_from_json(Tegrine *te, char *name) {
	char *s = read_file(open_saved("JSON file to load: ", name, "r"));

	cJSON	*root	= cJSON_Parse(s),
		*x	= cJSON_GetObjectItem(root, "x"),
		*len	= cJSON_GetObjectItem(root, "len"),
		*ins_x;

	px_assert(root && x && len, READ_JSON_ERROR),

	// TODO: Append to existing one instead
	free_tegrine(te),
	free(s),
	SET(te->x., len->valueint, Instance);

	cJSON_ArrayForEach(ins_x, x) {
		Instance *ins = &te->x.x[te->x.len++];

		read_d2(&ins->pos, ins_x, "pos"),
		read_d2(&ins->pos, ins_x, "size");

#ifdef VERTEX
		READ_JSON_VEC(
			ins->vertex,
			Vertex,
			Vertice,
			cJSON *v_neighbor = cJSON_GetObjectItem(v_r, "neighbor"),
			v->neighbor = v_neighbor ? v_neighbor->valueint : -1
		)
#endif

#ifdef PIXELS
		READ_JSON_VEC(ins->pixels, Pixels, Px,)
#endif

#ifdef SHAPES
		READ_JSON_VEC(
			ins->shapes,
			Shapes,
			Shape,
			cJSON *v_n = cJSON_GetObjectItem(v_r, "n"),
			v->n = v_n ? v_n->valueint : 0,
			read_d2(&v->size, ins_x, "size")
		)
#endif
	}
}
