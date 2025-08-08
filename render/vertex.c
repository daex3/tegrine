// TODO: Should support drawing faces by a filling algorithm like Scanline
void draw_vertex(D2 *ws, Vertex *v, Instance *ins) {
	for(size_t i = 0; i < v->len; ++i) {
		Vertice	*in = &v->x[i];

		if (in->neighbor < v->len)
			draw_line(ws, in->pos, v->x[in->neighbor].pos, ins);
	}
}
