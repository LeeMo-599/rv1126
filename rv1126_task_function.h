#ifndef _RV1126_TASK_FUNCTION_H 
#define _RV1126_TASK_FUNCTION_H

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "rv1126_public.h"



int rv1126_system_init();

int rv1126_vi_init(S_VI_CONFIG *vi_config);

int rv1126_ai_init(S_AI_CONFIG *ai_config);

int rv1126_venc_init(S_VENC_CONFIG *venc_config);

int rv1126_aenc_init(S_AENC_CONFIG *venc_config);

//int init_rv1126_function();

#endif