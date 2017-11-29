#include <SDL.h>
#include <stdio.h>

unsigned int callback(unsigned int interval, void * param) {

	printf("0\n");
        fflush(stdout);
	exit(0);
	return 0;
}


/* Main */
int main(int argn,char **argv)
{
	SDL_Event ev;
	int active;
	/* Initialize SDL */
	if(SDL_Init(SDL_INIT_TIMER || SDL_INIT_JOYSTICK) != 0)
		fprintf(stderr,"Could not initialize SDL: %s\n",SDL_GetError());
	/* Main loop */
	active = 1;

	SDL_TimerID timer;
	timer = SDL_AddTimer (2000 , callback , NULL);

	SDL_Joystick *joystick;
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);

	while(active)
	{
		/* Handle events */
		while(SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_JOYBUTTONDOWN) {
				if ( ev.jbutton.button == 0 ) {
					printf("1\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 1 ) {
					printf("2\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 2 ) {
					printf("3\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 3 ) {
					printf("4\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 4 ) {
					printf("5\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 5 ) {
					printf("6\n");
        				fflush(stdout);
					exit(0);
				}
				if ( ev.jbutton.button == 6 ) {
					printf("7\n");
        				fflush(stdout);
					exit(0);
				}
			}
			if(ev.type == SDL_QUIT)
				active = 0; /* End */
		}
	}
	/* Exit */
	SDL_Quit();
	return 0;
}
