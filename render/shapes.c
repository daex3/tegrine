void draw_shapes(D2 *ws, Shapes *sha, Instance *ins) {
	for(size_t i = 0; i < sha->len; ++i) {
		Shape  *sh	= &sha->x[i];
		D2 	b	= { sh->pos.x, sh->pos.y },
			lim	= {
			sh->size.x + b.x,
			sh->size.y + b.y
		};

		switch (sh->type) {
			case 1: {
				b.y -= sh->size.y;

				int o_x	= b.x - sh->size.x;

				for(; b.y < lim.y; ++b.y)
					for(b.x = o_x; b.x < lim.x; ++b.x)
						draw_point(ws, b, &ins->color, ins);
			}
			break;
			// Triangle with draw_line and circle with math.h
		}
	}
}
