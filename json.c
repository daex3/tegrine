#include <cjson/cJSON.h>
#include <string.h>
#include <sys/stat.h>

char * read_file(FILE *f) {
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

FILE * open_saved(char *message, char *name, char *mode) {
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

// TODO: Make the file even smaller by doing some complex shit B)
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

		if (ins->pos.x || ins->pos.y) {
			if (ins->pos.x == DELETED)
				continue;

			pos = cJSON_CreateObject();

			if (ins->pos.x)
				cJSON_AddNumberToObject(pos, "x", ins->pos.x);

			if (ins->pos.y)
				cJSON_AddNumberToObject(pos, "y", ins->pos.y);

			cJSON_AddItemToObject(ins_r, "pos", pos);
		}

		if (ins->color.r || ins->color.g || ins->color.b) {
			color = cJSON_CreateObject();

			if (ins->color.r)
				cJSON_AddNumberToObject(color, "r", ins->color.r);

			if (ins->color.g)
				cJSON_AddNumberToObject(color, "g", ins->color.g);

			if (ins->color.b)
				cJSON_AddNumberToObject(color, "b", ins->color.b);

			cJSON_AddItemToObject(ins_r, "color", color);
		}

		if (ins->vertex.x) {
			/*
				x:	Vertex [ .. ],
				len:	Number
			*/
			cJSON	*ve_r	= cJSON_CreateObject(),
				*ve_x	= cJSON_CreateArray();
			Vertex	*ve	= &ins->vertex;

			for(size_t i = 0; i < ve->len; ++i) {
				/*
					pos:		Pos { x, y },
					neighbor:	Number
				*/
				cJSON	*v_r	= cJSON_CreateObject();
				cJSON	*v_pos	= cJSON_CreateObject();
				Vertice	*v	= &ve->x[i];

				if (v->pos.x == DELETED)
					continue;

				cJSON_AddNumberToObject(v_pos, "x", v->pos.x),
				cJSON_AddNumberToObject(v_pos, "y", v->pos.y);

				if (v->neighbor < ve->len)
					cJSON_AddNumberToObject(v_r, "neighbor", v->neighbor);

				cJSON_AddItemToObject(v_r, "pos", v_pos),

				cJSON_AddItemToArray(ve_x, v_r);
			}

			cJSON_AddItemToObject(ve_r, "x", ve_x),
			cJSON_AddNumberToObject(ve_r, "len", ve->len),

			cJSON_AddItemToObject(ins_r, "vertex", ve_r);
		}

		if (ins->pixels.x) {
			/*
				x:	Px [ .. ],
				len:	Number
			*/
			cJSON	*px_r	= cJSON_CreateObject(),
				*px_x	= cJSON_CreateArray();
			Pixels	*px	= &ins->pixels;

			for(size_t i = 0; i < px->len; ++i) {
				/*
					pos:	{ x, y }
					color:	{ r, g, b }
				*/
				cJSON	*p_r		= cJSON_CreateObject(),
					*p_pos		= cJSON_CreateObject(),
					*p_color;
				Px	*p		= &px->x[i];

				if (p->pos.x == DELETED)
					continue;

				cJSON_AddNumberToObject(p_pos, "x", p->pos.x),
				cJSON_AddNumberToObject(p_pos, "y", p->pos.y);

				if (p->color.r || p->color.g || p->color.b) {
					p_color = cJSON_CreateObject();

					if (p->color.r)
						cJSON_AddNumberToObject(p_color, "r", p->color.r);

					if (p->color.g)
						cJSON_AddNumberToObject(p_color, "g", p->color.g);

					if (p->color.b)
						cJSON_AddNumberToObject(p_color, "b", p->color.b);

					cJSON_AddItemToObject(p_r, "color", p_color);
				}

				cJSON_AddItemToObject(p_r, "pos", p_pos),

				cJSON_AddItemToArray(px_x, p_r);
			}

			cJSON_AddItemToObject(px_r, "x", px_x),
			cJSON_AddNumberToObject(px_r, "len", px->len),

			cJSON_AddItemToObject(ins_r, "pixels", px_r);
		}

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

static void m_assert(_Bool condition, char *message) {
	if (!condition)
		puts(message),
		exit(-1);
}

void load_from_json(Tegrine *te, char *name) {
	char *s = read_file(open_saved("JSON file to load: ", name, "r"));

	cJSON	*root	= cJSON_Parse(s),
		*x	= cJSON_GetObjectItem(root, "x"),
		*len	= cJSON_GetObjectItem(root, "len"),
		*ins_x;

	m_assert(root && x && len, ERROR"Invalid JSON"),

	free_tegrine(te),
	free(s),
	SET(te->x., len->valueint, Instance);

	cJSON_ArrayForEach(ins_x, x) {
		cJSON	*ve_r		= cJSON_GetObjectItem(ins_x, "vertex"),
			*ve_x		= cJSON_GetObjectItem(ve_r, "x"),
			*ve_len		= cJSON_GetObjectItem(ve_r, "len"),
			*v_r,
			*px_r		= cJSON_GetObjectItem(ins_x, "pixels"),
			*px_x		= cJSON_GetObjectItem(px_r, "x"),
			*px_len		= cJSON_GetObjectItem(px_r, "len"),
			*p_r,
			*pos		= cJSON_GetObjectItem(ins_x, "pos"),
			*pos_x		= cJSON_GetObjectItem(pos, "x"),
			*pos_y		= cJSON_GetObjectItem(pos, "y"),
			*color		= cJSON_GetObjectItem(ins_x, "color"),
			*color_r	= cJSON_GetObjectItem(color, "r"),
			*color_g	= cJSON_GetObjectItem(color, "g"),
			*color_b	= cJSON_GetObjectItem(color, "b");
		Instance *ins = &te->x.x[te->x.len++];

		if (pos && (pos_x || pos_y)) {
			if (pos_x)
				ins->pos.x = pos_x->valueint;

			if (pos_y)
				ins->pos.y = pos_y->valueint;
		}

		if (color && (color_r || color_g || color_b)) {
			if (color_r)
				ins->color.r = color_r->valueint;

			if (color_g)
				ins->color.g = color_g->valueint;

			if (color_b)
				ins->color.b = color_b->valueint;
		}

		if (ve_x) {
			m_assert(ve_len, ERROR"ve_len missing");

			Vertex *ve = &ins->vertex;

			SET(ve->, ve_len->valueint, Vertice);

			cJSON_ArrayForEach(v_r, ve_x) {
				cJSON	*v_pos		= cJSON_GetObjectItem(v_r, "pos"),
					*pos_x		= cJSON_GetObjectItem(v_pos, "x"),
					*pos_y		= cJSON_GetObjectItem(v_pos, "y"),
					*v_neighbor	= cJSON_GetObjectItem(v_r, "neighbor");
				Vertice *v = &ve->x[ve->len++];

				m_assert(
					v_pos &&
					pos_x &&
					pos_y,
					ERROR"Broken vertex in JSON"
				),
				
				v->pos.x = pos_x->valueint,
				v->pos.y = pos_y->valueint,

				v->neighbor = v_neighbor ? v_neighbor->valueint : -1;
			}
		}

		if (px_x) {
			m_assert(px_len, ERROR"px_len missing"),

			SET(ins->pixels., px_len->valueint, Px);

			cJSON_ArrayForEach(p_r, px_x) {
				cJSON	*p_pos		= cJSON_GetObjectItem(p_r, "pos"),
					*pos_x		= cJSON_GetObjectItem(p_pos, "x"),
					*pos_y		= cJSON_GetObjectItem(p_pos, "y"),
					*p_color	= cJSON_GetObjectItem(p_r, "color"),
					*color_r	= cJSON_GetObjectItem(p_color, "r"),
					*color_g	= cJSON_GetObjectItem(p_color, "g"),
					*color_b	= cJSON_GetObjectItem(p_color, "b");

				m_assert(pos_x && pos_y, ERROR"Px.pos invalid");
				
				D2	pos	= {
					pos_x->valueint,
					pos_y->valueint
				};
				RGBA	o	= {
					color_r ? color_r->valueint : 0,
					color_g ? color_g->valueint : 0,
					color_b ? color_b->valueint : 0,
				};

				add_Px(&ins->pixels, &pos, &o);
			}
		}
	}
}
