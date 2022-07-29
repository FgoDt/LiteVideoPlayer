#include "../../core/lvp_core.h"
#include <SDL2/SDL.h>

int main(int argc, char** argv) {

	int count = 0;
	LVPCore* core = lvp_core_alloc();
	lvp_core_set_option(core, "-readers abcreader -lvphw cuda ");
	lvp_core_set_url(core, "d:/bb.mkv");
	lvp_core_play(core);
	SDL_Event ev;
	int seek = 0;
	while (1)
	{
		while(SDL_PollEvent(&ev)){
		}
		if (ev.type == SDL_QUIT)
		{
			break;
		}
		if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_A)
		{
			lvp_core_pause(core);
		}
		if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_B)
		{
			lvp_core_resume(core);
		}

		if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_S)
		{
			if (seek == 0)
			{
				lvp_core_seek(core, 200.0);
				printf("Seek\n");
			}
				seek=50;
		}
		if (seek>0)
		{
			seek--;
		}
		SDL_Delay(30);
	}
	lvp_core_free(core);
	count++;
	return 0;
}
