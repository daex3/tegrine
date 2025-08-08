CFLAGS	= -o out -lcjson -lpng -Wall -Wextra -Werror
MAIN	= a.c
CC	= gcc

all: rel r

dbg: d
	gdb ./out

bin: rel
	doas cp -i out /usr/bin/$(name)

rel:
	$(CC) -O3 $(MAIN) $(CFLAGS)
d:
	$(CC) -g $(MAIN) $(CFLAGS)
mac:
	$(CC) -dM -E $(MAIN)
sca:
	scan-build $(CC) $(MAIN) $(CFLAGS)

r:
	./out

gin:
	rm -rf .git
	git init
	git add .
	git commit -m ':)'
	git remote add origin git@github.com:daex3/$(name).git

git:
	git push -u origin master
