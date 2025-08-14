# Lib: Tegrine
Draw instances of 2D pixels/vertex/shapes over a terminal
## Setup
```shell
sudo ln -s $PWD /usr/include/tegrine
```
## Libs
- CJSON(optional)
- Math(optional)
## Example
```c
#define PIXELS

#include <tegrine/instances.c>

int main() {
	Tegrine te = { };
	D2 pos = { }, size = { };
	RGBA color = { };
	Instance *p = add_Instance(te.x, pos, size);

	add_Px(&p->pixels, pos, color);

	free_tegrine(&te);
}
```