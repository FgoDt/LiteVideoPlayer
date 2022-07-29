#ifndef _LVP_CPP_H_
#define _LVP_CPP_H_
int lvp_cpp_test();

void* lvp_create_soundtouch();

int lvp_soundtouch_set_tempo(void *st, double tempo);

int lvp_soundtouch_set_channels(void *st, int channels);

int lvp_soundtouch_set_rate(void *st, double rate);

int lvp_soundtouch_set_sample_rate(void *st, uint32_t rate);

int lvp_soundtouch_change_tempo(void *st, int tempo);

int lvp_soundtouch_send_sample(void *st, void *data,int nbsamples);

int lvp_soundtouch_recive_sample(void *st,void *data,uint32_t maxsample);

void  lvp_soundtouch_destroy(void *st);

#endif
