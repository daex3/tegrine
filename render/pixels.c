void draw_pixels(D2 *ws, Pixels *px, Instance *ins) {
	for(size_t i = 0; i < px->len; ++i) {
		Px *in = &px->x[i];

		draw_point(ws, in->pos, &in->color, ins);
	}
}
