#ifndef _RV1126_DATA_PROCESS_H
#define _RV1126_DATA_PROCESS_H

#include "rv1126_public.h"

typedef struct
{
    unsigned int config_id;
    unsigned int task_id;
    unsigned int venc_id;
}S_VENC_PROC_ARG;

typedef struct
{
    unsigned int config_id;
    unsigned int task_id;
    unsigned int aenc_id;
}S_AENC_PROC_ARG;

void * video_venc_thread(void *args);
void * audio_aenc_thread(void *args);
void * push_server_thread(void *args);


#endif