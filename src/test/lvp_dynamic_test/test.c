#include "../../core/lvp_dynamic_help.h"
#include <dlfcn.h>
#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argc, char** argv) {
	void* hanle = dlopen("/home/fgodt/worke/LiteVideoPlayer/build/liblvp.so",RTLD_LAZY);
	
	if(hanle == NULL){
		fprintf(stderr,"%s\n",dlerror());
		return -1;
	}
	dlerror();
	lvp_core_alloc_d core_alloc = dlsym(hanle,"lvp_core_alloc");
	void * core  = core_alloc();


	if(core == NULL){
		fprintf(stderr,"core open error");
	}
	lvp_core_set_url_d set_url = dlsym(hanle,"lvp_core_set_url");
	lvp_core_play_d core_play = dlsym(hanle,"lvp_core_play");
	int ret = set_url(core,"/home/fgodt/gd.mkv");
	if(ret != 0){
		fprintf(stderr,"set url error");
	}
	ret = core_play(core);
	if(ret != 0){
		fprintf(stderr,"play error");
	}
	//SDL_Init(SDL_INIT_EVERYTHING);
	while (1)
	{	
		SDL_Event ev;
		while(SDL_PollEvent(&ev));
		if(ev.type == SDL_QUIT){
			break;
		}
		SDL_Delay(10);
	}
	
	//getchar();
	return 0;
}