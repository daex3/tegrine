static _Bool within_max(D2 *x, D2 *max) {
	return x->x >= 0 && x->y >= 0 && x->x < max->x && x->y < max->y;
}

void draw_point(D2 *ws, D2 pos, RGBA *o, Instance *ins) {
	// Adjust according to instance
	if (ins)
		pos.x += ins->pos.x,
		pos.y += ins->pos.y;

	if (within_max(&pos, ws))
		printf(
			"\x1b[48;2;%hhu;%hhu;%hhum\x1b[%d;%dH \x1b[m",
			o->r * o->a / 255,
			o->g * o->a / 255,
			o->b * o->a / 255,
			1 + pos.y,
			1 + pos.x
		);
}

void draw_line(D2 *ws, D2 a, D2 b, Instance *ins) {
	// Bresenham's Line Algorithm
	int	dx	= abs(b.x - a.x),	dy = abs(b.y - a.y),
		sx	= a.x < b.x ? 1 : -1,	sy = a.y < b.y ? 1 : -1,
		err	= dx - dy;

	for(;;) {
		draw_point(ws, a, &ins->color, ins);

		if (a.x == b.x && a.y == b.y)
			break;

		int e2 = err * 2;

		if (e2 > -dy)
			err -= dy,
			a.x += sx;

		if (e2 < dx)
			err += dx,
			a.y += sy;
	}
}


#ifdef VERTEX
#include <tegrine/render/vertex.c>
#endif
#ifdef PIXELS
#include <tegrine/render/pixels.c>
#endif
#ifdef SHAPES
#include <tegrine/render/shapes.c>
#endif


void draw(Tegrine *te, D2 *pos) {
	fputs("\x1b[2J", stdout);

	for(size_t i = 0; i < te->x.len; ++i) {
		Instance *ins = &te->x.x[i];

#ifdef SHAPES
		if (ins->shapes.x)
			draw_shapes(&te->ws, &ins->shapes, ins);
#endif

#ifdef VERTEX
		if (ins->vertex.x)
			draw_vertex(&te->ws, &ins->vertex, ins);
#endif

#ifdef PIXELS
		if (ins->pixels.x)
			draw_pixels(&te->ws, &ins->pixels, ins);
#endif
	}

	printf("\x1b[%d;%dH", 1 + pos->y, 1 + pos->x),
	fflush(stdout);
}
