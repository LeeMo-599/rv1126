#include "rkmedia_assignment_manage.h"
#include "rkmedia_data_process.h"
#include "rkmedia_ffmpeg_config.h"
#include "rkmedia_module.h"

#include "rkmedia_ffmpeg_config.h"
#include "rkmedia_container.h"

int init_rv1126_first_assignment(int protocol_type, char * network_address)
{
    int ret;
    RKMEDIA_FFMPEG_CONFIG *ffmpeg_config = (RKMEDIA_FFMPEG_CONFIG *)malloc(sizeof(RKMEDIA_FFMPEG_CONFIG));
    if (ffmpeg_config == NULL)
    {
        printf("malloc ffmpeg_config failed\n");
    }

    ffmpeg_config->config_id = 0;
    ffmpeg_config->protocol_type = protocol_type;
    ffmpeg_config->video_codec = AV_CODEC_ID_H264;
    ffmpeg_config->audio_codec = AV_CODEC_ID_AAC;
    memcpy(ffmpeg_config->network_addr, network_address, strlen(network_address));
    
    //初始化ffmpeg输出模块
    init_rkmedia_ffmpeg_context(ffmpeg_config);

    MPP_CHN_S vi_channel;
    MPP_CHN_S ai_channel;
    MPP_CHN_S venc_channel;
    MPP_CHN_S aenc_channel;

    //从VI容器里面获取VI_ID
    RV1126_VI_CONTAINTER vi_container;
    get_vi_container(0, &vi_container);

    //从VENC容器里面获取VENC_ID
    RV1126_VENC_CONTAINER venc_container;
    get_venc_container(0, &venc_container);

    vi_channel.enModId = RK_ID_VI;  //VI模块ID
    vi_channel.s32ChnId = vi_container.vi_id;//VI通道ID
    
    venc_channel.enModId = RK_ID_VENC;//VENC模块ID
    venc_channel.s32ChnId = venc_container.venc_id;//VENC通道ID

    //绑定VI和VENC节点
    ret = RK_MPI_SYS_Bind(&vi_channel, &venc_channel);
    if (ret != 0)
    {
        printf("bind venc error\n");
        return -1;
    }
    else
    {
        printf("bind venc error\n");
    }

    //从AI容器里面获取AI_ID
    RV1126_AI_CONTAINTER ai_container;
    get_ai_container(0, &ai_container);

    //从AENC容器里面获取AENC_ID
    RV1126_AENC_CONTAINER aenc_container;
    get_aenc_container(0, &aenc_container);

    ai_channel.enModId = RK_ID_AI; //AI模块ID
    ai_channel.s32ChnId = ai_container.ai_id; //AI通道ID

    aenc_channel.enModId = RK_ID_AENC; //AENC模块ID
    aenc_channel.s32ChnId = aenc_container.aenc_id; //AENC通道ID
    ret = RK_MPI_SYS_Bind(&ai_channel, &aenc_channel);
    if (ret != 0)
    {
        printf("bind aenc id error\n");
        return -1;
    }
    else
    {
        printf("bind aenc id success\n");
    }

    //VENC线程的参数
    VENC_PROC_PARAM *venc_arg_params = (VENC_PROC_PARAM *)malloc(sizeof(VENC_PROC_PARAM));
    if (venc_arg_params == NULL)
    {
        printf("malloc venc arg error\n");
        free(venc_arg_params);
    }

    venc_arg_params->vencId = venc_channel.s32ChnId;

    //创建VENC线程，获取摄像头编码数据
    pthread_t pid;
    ret = pthread_create(&pid, NULL, camera_venc_thread, (void *)venc_arg_params);
    if (ret != 0)
    {
        printf("create camera_venc_thread failed\n");
    }

    //创建AENC线程，获取麦克风编码数据
    AENC_PROC_PARAM *aenc_arg = (AENC_PROC_PARAM *)malloc(sizeof(AENC_PROC_PARAM));
    if (aenc_arg == NULL)
    {
        printf("malloc aenc arg error\n");
        free(aenc_arg);
    }

    aenc_arg->aencId = aenc_channel.s32ChnId;
    ret = pthread_create(&pid, NULL, audio_aenc_thread, (void *)aenc_arg);
    if (ret != 0)
    {
        printf("create audio_aenc_thread error\n");
    }

    //创建AENC线程，获取麦克风编码数据
    ret = pthread_create(&pid, NULL, push_server_thread, (void *)ffmpeg_config);
    if (ret != 0)
    {
        
        printf("push_server_thread error\n");
    }

    return 0;
}
