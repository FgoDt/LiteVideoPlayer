#include "lvp_core.h"
#include "lvp.h"
#include <SDL2/SDL.h>

int main(int argc , char *argv[]){
	int j = 2;
	//while (j>0) {
		//j--;
		printf("%s\n", LVP_VER);
		LVPCore* core = lvp_core_alloc();

		int ret = lvp_core_init(core);
		if (ret != LVP_OK) {
			lvp_error(NULL, "lvp core init error\n", NULL);
			return -1;
		}

		if (argc < 2 || argc > 3) {
			printf("use LiteVideoPlayer input.xx options \n");
				lvp_core_set_url(core, "d:/src.mp4");
		//	lvp_core_set_option(core, "-lvp_vcodec h264_cuvid");
			//    return -1;
		}

		if (argc == 3) {
			lvp_core_set_option(core, argv[2]);
		}
		if (argc > 1) {
			lvp_core_set_url(core, argv[1]);
		}
		lvp_core_play(core);
		int i = 0;
		while (i<100000)
		{
			SDL_Event ev;
			while (SDL_PollEvent(&ev));
			if (ev.type == SDL_QUIT) {
				break;
			}
			lvp_sleep(10);
			if (i == 500) {
			lvp_core_set_url(core, "d:/fd.mp4");
				lvp_core_play(core);
				lvp_core_seek(core, 5);
			}
			if (i == 1000) {
				lvp_core_set_url(core, "d:/zelda.mp4");
				lvp_core_play(core);
				lvp_core_seek(core, 6);
			}
			i++;
			
		}
		lvp_core_free(core);
		SDL_Quit();
	//}
    return 0;
}