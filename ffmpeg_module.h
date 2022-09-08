#ifndef _FFMPEG_MODULE_H                                                |{
#define _FFMPEG_MODULE_H  

#include "ffmpeg_public.h"
#include <pthread.h>


int init_stream_format_context(S_FFMPEG_GROUP *group);
void close_stream(AVFormatContext *oc, OutputStream *ost);

#endif