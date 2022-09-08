#ifndef _FFMPEG_GROUP_H                                               
#define _FFMPEG_GROUP_H  

#include "ffmpeg_public.h"
#include <pthread.h>

int init_ffmpeg_manage_function();                                  
int set_ffmpeg_group_url_id(unsigned int url_id);

int set_ffmpeg_group(unsigned int url_id, S_FFMPEG_GROUP *group);
unsigned int get_ffmpeg_group(unsigned int url_id, S_FFMPEG_GROUP *group);

void print_ffmpeg_group(char *message, S_FFMPEG_GROUP *group);

#endif