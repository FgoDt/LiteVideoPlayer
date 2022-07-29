#include <iostream>
#include <SoundTouch.h>

extern "C"{
    #include "lvp_core.h"
}

using namespace std;
using namespace soundtouch;

extern "C"
int lvp_cpp_test(){
    std::cout<<"cpp ok" << std::endl;
    return LVP_OK;
}
extern "C"
void *lvp_create_soundtouch(){
    SoundTouch *st = new(SoundTouch);
    return st;
}

extern "C"
int lvp_soundtouch_set_tempo(void *st, double tempo){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->setTempo(tempo);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_set_channels(void *st, int channels){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->setChannels(channels);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_set_rate(void *st, double rate){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->setRate(rate);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_change_tempo(void *st, int tempo){
    SoundTouch *soundTouch = (SoundTouch*)st;
	soundTouch->setTempoChange(tempo);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_set_sample_rate(void *st, uint32_t rate){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->setSampleRate(rate);
    soundTouch->setSetting(SETTING_USE_QUICKSEEK,1);
    soundTouch->setSetting(SETTING_USE_AA_FILTER,1);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_send_sample(void *st, void *data,int nbsamples){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->putSamples((SAMPLETYPE*)data,nbsamples);
    return LVP_OK;
}

extern "C"
int lvp_soundtouch_recive_sample(void *st,void *data,uint32_t maxsample){
    SoundTouch *soundTouch = (SoundTouch*)st;
    int ret = soundTouch->receiveSamples((SAMPLETYPE*)data,maxsample);
    return ret;
}

extern "C"
void  lvp_soundtouch_destroy(void *st){
    SoundTouch *soundTouch = (SoundTouch*)st;
    soundTouch->clear();
    delete soundTouch;
}