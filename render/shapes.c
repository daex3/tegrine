#include <math.h>

#define PI_2 M_PI * 2

void draw_shapes(D2 *ws, Shapes *sha, Instance *ins) {
	for(size_t i = 0; i < sha->len; ++i) {
		Shape  *sh	= &sha->x[i];
		D2 	b	= { sh->pos.x, sh->pos.y },
			lim	= {
			sh->size.x + b.x,
			sh->size.y + b.y
		};

		switch (sh->type) {
			// Full rectangle |=|
			case 1: {
				b.y -= sh->size.y;

				int o_x	= b.x - sh->size.x;

				for(; b.y < lim.y; ++b.y)
					for(b.x = o_x; b.x < lim.x; ++b.x)
						draw_point(ws, b, &ins->color, ins);
			}
			break;
			// Triangle /\_
			case 2: {
				D2 d = { b.x - sh->size.y, b.y + sh->size.y };

				b.y -= sh->size.y,

				draw_line(ws, b, d, ins),

				d.x = sh->pos.x + sh->size.x,

				draw_line(ws, b, d, ins),

				b.x = sh->pos.x - sh->size.x,
				b.y = sh->pos.y + sh->size.y;

				for(; b.x < lim.x; ++b.x)
					draw_point(ws, b, &ins->color, ins);
			}
			break;
			// Circle O
			case 3:
				for(float i = 0; i < PI_2; i += .01)
					b.x = sh->pos.x + cos(i) * sh->size.x,
					b.y = sh->pos.y + sin(i) * sh->size.y,

					draw_point(ws, b, &ins->color, ins);

				break;
		}
	}
}
