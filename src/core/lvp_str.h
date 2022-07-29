#ifndef _LVP_STR_H_
#define _LVP_STR_H_

#include "lvp_core.h"

int lvp_str_dump_with_size(const char *src, char **dst,size_t size);

int lvp_str_dump(const char *src, char **dst);

int lvp_str_parse_to_options(const char *option_str, LVPMap *options);

#endif