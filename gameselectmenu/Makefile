SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LFLAGS := $(shell sdl-config --libs)

build:
	gcc -c gameselectmenu.c $(SDL_CFLAGS)
	gcc gameselectmenu.o -o gameselectmenu $(SDL_LFLAGS)

clean:
	-rm gameselectmenu;
	-rm gameselectmenu.o;
