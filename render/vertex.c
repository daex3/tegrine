// TODO: Should support drawing faces by a filling algorithm like Scanline
void draw_vertex(D2 *ws, Vertex *ve, Instance *ins) {
	for(size_t i = 0; i < ve->len; ++i) {
		Vertice	*v = &ve->x[i];

		if (v->neighbor < ve->len)
			draw_line(ws, v->pos, ve->x[v->neighbor].pos, &v->color, ins);
	}
}
