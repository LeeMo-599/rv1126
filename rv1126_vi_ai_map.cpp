#include <pthread.h>
#include <string.h>
#include "rv1126_vi_ai_map.h"

unsigned int g_vi_real_id;
unsigned int g_ai_real_id;
pthread_mutex_t g_vi_real_id_mutex;
pthread_mutex_t g_ai_real_id_mutex;

S_VI_AI_MAP g_vi_ai_map;
pthread_mutex_t g_vi_ai_map_mutex;

int init_vi_ai_map_function()
{
    pthread_mutex_init(&g_vi_ai_map_mutex, NULL);
    pthread_mutex_init(&g_vi_real_id_mutex, NULL);
    pthread_mutex_init(&g_ai_real_id_mutex, NULL);
    memset(&g_vi_ai_map, 0, sizeof(g_vi_ai_map));
    g_vi_real_id = 0;
    g_ai_real_id = 0;

    return 0;
}

int set_vi_real_id(unsigned int id)
{
    pthread_mutex_lock(&g_vi_real_id_mutex);
    g_vi_real_id = id;
    pthread_mutex_unlock(&g_vi_real_id_mutex);

    return 0;
}

int reset_vi_real_id()
{
    pthread_mutex_lock(&g_vi_real_id_mutex);
    g_vi_real_id = 0;
    pthread_mutex_unlock(&g_vi_real_id_mutex);

    return 0;
}

int update_vi_real_id()
{
    pthread_mutex_lock(&g_vi_real_id_mutex);
    g_vi_real_id++;
    pthread_mutex_unlock(&g_vi_real_id_mutex);

    return 0;
}

unsigned int get_vi_real_id()
{
    unsigned int id = 0;

    pthread_mutex_lock(&g_vi_real_id_mutex);
    id = g_vi_real_id;
    pthread_mutex_unlock(&g_vi_real_id_mutex);

    return id;
}

int set_ai_real_id(unsigned int id)
{
    pthread_mutex_lock(&g_ai_real_id_mutex);
    g_ai_real_id = id;
    pthread_mutex_unlock(&g_ai_real_id_mutex);

    return 0;
}

int reset_ai_real_id()
{
    pthread_mutex_lock(&g_ai_real_id_mutex);
    g_ai_real_id = 0;
    pthread_mutex_unlock(&g_ai_real_id_mutex);

    return 0;
}

int update_ai_real_id()
{
    pthread_mutex_lock(&g_ai_real_id_mutex);
    g_ai_real_id++;
    pthread_mutex_unlock(&g_ai_real_id_mutex);

    return 0;
}

unsigned int get_ai_real_id()
{
    unsigned int id = 0;

    pthread_mutex_lock(&g_ai_real_id_mutex);
    id = g_ai_real_id;
    pthread_mutex_unlock(&g_ai_real_id_mutex);

    return id;
}

int reset_vi_ai_map()
{
    pthread_mutex_lock(&g_vi_ai_map_mutex);
    memset(&g_vi_ai_map, 0, sizeof(g_vi_ai_map));
    pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int set_vi_ai_map_id(unsigned int config_id)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    g_vi_ai_map.config_id = config_id;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

unsigned int get_vi_ai_map_id()
{
    unsigned int config_id = 0;

    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    config_id = g_vi_ai_map.config_id;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return config_id;
}

int set_vi_map(unsigned int index, S_VI_MAP *vi_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    g_vi_ai_map.vi_maps[index] = *vi_map;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int get_vi_map(unsigned int index, S_VI_MAP *vi_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    *vi_map = g_vi_ai_map.vi_maps[index];
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int set_ai_map(unsigned int index, S_AI_MAP *ai_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    g_vi_ai_map.ai_maps[index] = *ai_map;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int get_ai_map(unsigned int index, S_AI_MAP *ai_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    *ai_map = g_vi_ai_map.ai_maps[index];
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}


int set_venc_map(unsigned int index, S_VENC_MAP *venc_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    g_vi_ai_map.venc_maps[index] = *venc_map;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int get_venc_map(unsigned int index, S_VENC_MAP *venc_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    *venc_map = g_vi_ai_map.venc_maps[index];
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int set_aenc_map(unsigned int index, S_AENC_MAP *aenc_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    g_vi_ai_map.aenc_maps[index] = *aenc_map;
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);

    return 0;
}

int get_aenc_map(unsigned int index, S_AENC_MAP *aenc_map)
{
    //pthread_mutex_lock(&g_vi_ai_map_mutex);
    *aenc_map = g_vi_ai_map.aenc_maps[index];
    //pthread_mutex_unlock(&g_vi_ai_map_mutex);
    return 0;
}




void print_vi_ai_map()
{
    printf("[VI AI MAP] config id: %u", g_vi_ai_map.config_id);
    for (int i = 0; i < MAX_VI_NUM; i++)
    {
        printf("[VI AI MAP][VI MAP] id: %u, real id: %u, used: %u\n", 
                g_vi_ai_map.vi_maps[i].id,
                g_vi_ai_map.vi_maps[i].real_id,
                g_vi_ai_map.vi_maps[i].used);
    }
    for (int i = 0; i < MAX_AI_NUM; i++)
    {
        printf("[VI AI MAP][AI MAP] id: %u, real id: %u, used: %u\n", 
                g_vi_ai_map.ai_maps[i].id,
                g_vi_ai_map.ai_maps[i].real_id,
                g_vi_ai_map.ai_maps[i].used);
    }
}
