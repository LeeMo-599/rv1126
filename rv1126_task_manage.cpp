#include "rv1126_task_manage.h"
#include "rv1126_data_process.h"
#include "sample_common.h"
#include "ffmpeg_module.h"
#include "ffmpeg_group.h"
#include "rv1126_vi_ai_map.h"

int init_rv1126_first_task()
{

    //int ret;

#if 1
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    SAMPLE_COMM_ISP_Init(hdr_mode, RK_FALSE);
    SAMPLE_COMM_ISP_Run();
    SAMPLE_COMM_ISP_SetFrameRate(30);
#endif

    const char * local_str = "test_camera.ts";

    S_FFMPEG_GROUP *group = (S_FFMPEG_GROUP *)malloc(sizeof(S_FFMPEG_GROUP));
    if (group == NULL)
    {
        printf("malloc ffmpeg_group failed\n");
    }

    group->url_id = 0;
    group->url_type = RTMP;
    group->video_codec = AV_CODEC_ID_H264;
    group->audio_codec = AV_CODEC_ID_AAC;
    //memcpy(group->url_addr, local_str, strlen(local_str));
    memcpy(group->url_addr, "rtmp://10.0.0.87:1935/live/cz01", strlen("rtmp://10.0.0.87:1935/live/cz01"));
    //memcpy(group->url_addr, "srt://10.0.0.65:8080?streamid=uplive.sls.com/live/test", strlen("srt://10.0.0.65:8080?streamid=uplive.sls.com/live/test"));
    
    init_stream_format_context(group);
    set_ffmpeg_group(group->url_id, group);

#if 1
    S_VI_CONFIG tmp_vi_config;
    memset(&tmp_vi_config, 0, sizeof(tmp_vi_config));
    tmp_vi_config.id = 1;
    tmp_vi_config.attr.u32BufCnt = 3;
    tmp_vi_config.attr.u32Width = 1920;
    tmp_vi_config.attr.u32Height = 1080;
    tmp_vi_config.attr.enPixFmt = IMAGE_TYPE_NV12;
    tmp_vi_config.attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
    tmp_vi_config.attr.enWorkMode = VI_WORK_MODE_NORMAL;
    int ret = rv1126_vi_init(&tmp_vi_config);
    if (ret != 0)
    {
        printf("vi init error\n");
    }
    else
    {
        printf("vi init success\n");
    }

    S_VENC_CONFIG venc_config = {0};
    memset(&venc_config, 0, sizeof(venc_config));
    venc_config.id = 0;
    venc_config.attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
    venc_config.attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_config.attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_config.attr.stRcAttr.stH264Cbr.u32BitRate = 1920 * 1080;
    venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    venc_config.attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
    venc_config.attr.stVencAttr.u32PicWidth = 1920;
    venc_config.attr.stVencAttr.u32PicHeight = 1080;
    venc_config.attr.stVencAttr.u32VirWidth = 1920;
    venc_config.attr.stVencAttr.u32VirHeight = 1080;
    venc_config.attr.stVencAttr.u32Profile = 77;
    venc_config.enable = true;
    ret = rv1126_venc_init(&venc_config);
    if (ret != 0)
    {
        printf("venc init error, ret = %d\n", ret);
    }
    else
    {
        printf("venc init success ret = %d\n", ret);
    }

    S_AI_CONFIG tmp_ai_config = {0};
    tmp_ai_config.id = 0;
    tmp_ai_config.attr.enSampleFormat = RK_SAMPLE_FMT_S16;
    tmp_ai_config.attr.enAiLayout = AI_LAYOUT_NORMAL;
    tmp_ai_config.attr.u32NbSamples = AAC_NB_SAMPLES;
    tmp_ai_config.attr.u32SampleRate = 48000;
    tmp_ai_config.attr.u32Channels = 2;
    tmp_ai_config.enable = true;
    ret = rv1126_ai_init(&tmp_ai_config);
    if (ret != 0)
    {
        printf("ai init error, ret = %d\n", ret);
    }
    else
    {
        printf("ai init success ret = %d\n", ret);
    }

    S_AENC_CONFIG tmp_aenc_config;
    tmp_aenc_config.id = 0;
    tmp_aenc_config.attr.enCodecType = RK_CODEC_TYPE_AAC;
    tmp_aenc_config.attr.u32Bitrate = 153600;
    tmp_aenc_config.attr.u32Quality = 1;
    tmp_aenc_config.attr.stAencAAC.u32Channels = 2;
    tmp_aenc_config.attr.stAencAAC.u32SampleRate = 48000;
    ret = rv1126_aenc_init(&tmp_aenc_config);
    if(ret != 0)
    {
        printf("aenc init error, ret = %d\n", ret);
    }
    else
    {
        printf("aenc init success, ret = %d\n", ret);
    }

    MPP_CHN_S vi_chn;
    MPP_CHN_S ai_chn;
    MPP_CHN_S rga_chn;
    MPP_CHN_S venc_chn;
    MPP_CHN_S aenc_chn;

    //S_VI_MAP s_vi_map ;
    //get_vi_map(0, &s_vi_map);

    vi_chn.enModId = RK_ID_VI;
    vi_chn.s32ChnId = tmp_vi_config.id;
    //vi_chn.s32ChnId = s_vi_map.real_id;

    venc_chn.enModId = RK_ID_VENC;
    venc_chn.s32ChnId = venc_config.id;

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

   // S_AI_MAP s_ai_map ;
   // get_ai_map(0, &s_ai_map);

    ai_chn.enModId = RK_ID_AI;
    //ai_chn.s32ChnId = s_ai_map.real_id;
    ai_chn.s32ChnId = tmp_ai_config.id;

    aenc_chn.enModId = RK_ID_AENC;
    aenc_chn.s32ChnId = tmp_aenc_config.id;

    ret = RK_MPI_SYS_Bind(&ai_chn, &aenc_chn);
    if (ret != 0)
    {
        printf("[AI] vi id: %d bind aenc id: %d, ret: %d error\n", ai_chn.s32ChnId, aenc_chn.s32ChnId, ret);
        return -1;
    }
    else
    {
        printf("[AI] vi id: %d bind aenc id: %d success\n", ai_chn.s32ChnId, aenc_chn.s32ChnId);
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
#endif
    return 0;
}