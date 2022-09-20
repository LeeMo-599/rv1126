#include "rv1126_task_manage.h"
#include "rv1126_data_process.h"
#include "sample_common.h"
#include "ffmpeg_module.h"
#include "ffmpeg_group.h"
#include "rv1126_vi_ai_manage.h"
#include "rv1126_vi_ai_map.h"

int init_rv1126_first_task()
{
    int ret;
    S_FFMPEG_GROUP *group = (S_FFMPEG_GROUP *)malloc(sizeof(S_FFMPEG_GROUP));
    if (group == NULL)
    {
        printf("malloc ffmpeg_group failed\n");
    }

    group->url_id = 0;
    group->url_type = LOCAL;
    group->video_codec = AV_CODEC_ID_H264;
    group->audio_codec = AV_CODEC_ID_AAC;
    memcpy(group->url_addr, "test.ts", strlen("test.ts"));
    //memcpy(group->url_addr, "rtmp://10.0.0.88:1935/live/cz01", strlen("rtmp://10.0.0.88:1935/live/cz01"));
    //memcpy(group->url_addr, "srt://10.0.0.88:8080?streamid=uplive.sls.com/live/cz_01", strlen("srt://10.0.0.88:8080?streamid=uplive.sls.com/live/cz_01"));
    init_stream_format_context(group);
    set_ffmpeg_group(group->url_id, group);
  
    MPP_CHN_S vi_chn;
    MPP_CHN_S ai_chn;
    MPP_CHN_S venc_chn;
    MPP_CHN_S aenc_chn;

    S_VI_MAP s_vi_map;
    get_vi_map(0, &s_vi_map);

    S_VENC_MAP s_venc_map;
    get_venc_map(0, &s_venc_map);

    vi_chn.enModId = RK_ID_VI;
    vi_chn.s32ChnId = s_vi_map.real_id;
    venc_chn.enModId = RK_ID_VENC;
    venc_chn.s32ChnId = s_venc_map.real_id;
    ret = RK_MPI_SYS_Bind(&vi_chn, &venc_chn);
    if (ret != 0)
    {
        printf("[VI] vi id: %d bind venc id: %d, ret: %d error\n", vi_chn.s32ChnId, venc_chn.s32ChnId, ret);
        return -1;
    }
    else
    {
        printf("[VI] vi id: %d bind venc id: %d success\n", vi_chn.s32ChnId, venc_chn.s32ChnId);
    }

    S_AI_MAP s_ai_map;
    get_ai_map(0, &s_ai_map);

    S_AENC_MAP s_aenc_map;
    get_aenc_map(0, &s_aenc_map);

    ai_chn.enModId = RK_ID_AI;
    ai_chn.s32ChnId = s_ai_map.real_id;
    aenc_chn.enModId = RK_ID_AENC;
    aenc_chn.s32ChnId = s_aenc_map.real_id;
    ret = RK_MPI_SYS_Bind(&ai_chn, &aenc_chn);
    if (ret != 0)
    {
        printf("[AI] ai id: %d bind aenc id: %d, ret: %d error\n", ai_chn.s32ChnId, aenc_chn.s32ChnId, ret);
        return -1;
    }
    else
    {
        printf("[AI] ai id: %d bind aenc id: %d success\n", ai_chn.s32ChnId, aenc_chn.s32ChnId);
    }

    S_VENC_PROC_ARG *venc_arg = (S_VENC_PROC_ARG *)malloc(sizeof(S_VENC_PROC_ARG));
    if (venc_arg == NULL)
    {
        printf("[TASK CONFIG][config][create] , malloc venc arg error\n");
        free(venc_arg);
    }

    venc_arg->venc_id = venc_chn.s32ChnId;
    pthread_t pid;
    ret = pthread_create(&pid, NULL, video_venc_thread, (void *)venc_arg);
    if (ret != 0)
    {
        printf("create video_venc_thread failed\n");
    }

    S_AENC_PROC_ARG *aenc_arg = (S_AENC_PROC_ARG *)malloc(sizeof(S_AENC_PROC_ARG));
    if (venc_arg == NULL)
    {
        printf("[TASK CONFIG][config][create] , malloc aenc arg error\n");
        free(aenc_arg);
    }

    aenc_arg->aenc_id = aenc_chn.s32ChnId;
    ret = pthread_create(&pid, NULL, audio_aenc_thread, (void *)aenc_arg);
    if (ret != 0)
    {
        printf("[TASK CONFIG][config][create]  audio_aenc_thread error\n");
    }

    ret = pthread_create(&pid, NULL, push_server_thread, (void *)group);
    if (ret != 0)
    {
        
        printf("[TASK CONFIG][config][create] , push_server_thread error\n");
    }

    return 0;
}