#include "ffmpeg_group.h"

S_FFMPEG_GROUP g_ffmpeg_groups[MAX_URL_NUM];
pthread_mutex_t g_ffmpeg_groups_mutex;

int init_ffmpeg_manage_function()
{
    pthread_mutex_init(&g_ffmpeg_groups_mutex, NULL);
    memset(g_ffmpeg_groups, 0, sizeof(g_ffmpeg_groups));
    return 0;
}

int set_ffmpeg_group(unsigned int url_id, S_FFMPEG_GROUP *group)
{
    pthread_mutex_lock(&g_ffmpeg_groups_mutex);
    g_ffmpeg_groups[url_id] = *group;
    pthread_mutex_unlock(&g_ffmpeg_groups_mutex);
    return 0;
}

unsigned int get_url_ffmpeg_group(unsigned int url_id, S_FFMPEG_GROUP *group)
{
    pthread_mutex_lock(&g_ffmpeg_groups_mutex);
    *group = g_ffmpeg_groups[url_id];
    pthread_mutex_unlock(&g_ffmpeg_groups_mutex);
    return 0;
}
