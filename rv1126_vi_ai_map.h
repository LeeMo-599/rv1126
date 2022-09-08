#ifndef _RV1126_VI_AI_MAP_H
#define _RV1126_VI_AI_MAP_H

#define MAX_VI_NUM 2
#define MAX_AI_NUM 2

#include <stdbool.h>
#include "rv1126_public.h"
//#include "rv1126_config.h"

typedef struct
{
    unsigned int id;
    unsigned int real_id;
    bool used;
}S_VI_MAP;

typedef struct
{
    unsigned int id;
    unsigned int real_id;
    bool used;
}S_AI_MAP;

typedef struct
{
    unsigned int config_id;
    S_VI_MAP vi_maps[MAX_VI_NUM];
    S_AI_MAP ai_maps[MAX_AI_NUM];
}S_VI_AI_MAP;

int init_vi_ai_map_function();

int set_vi_real_id(unsigned int id);
int reset_vi_real_id();
int update_vi_real_id();
unsigned int get_vi_real_id();
int set_ai_real_id(unsigned int id);
int reset_ai_real_id();
int update_ai_real_id();
unsigned int get_ai_real_id();

int reset_vi_ai_map();
int set_vi_ai_map_id(unsigned int config_id);
unsigned int get_vi_ai_map_id();
int set_vi_map(unsigned int index, S_VI_MAP *vi_map);
int get_vi_map(unsigned int index, S_VI_MAP *vi_map);
int set_ai_map(unsigned int index, S_AI_MAP *ai_map);
int get_ai_map(unsigned int index, S_AI_MAP *ai_map);

void print_vi_ai_map();

#endif
