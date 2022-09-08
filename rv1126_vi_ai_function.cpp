#include "rv1126_vi_ai_function.h"
#include "rv1126_vi_ai_map.h"

int init_vi_ai_function()
{
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
        S_VI_MAP vi_map = {0};
        vi_map.id = 0;
        vi_map.real_id = tmp_vi_config.id;
        set_vi_map(0, &vi_map);
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
        S_AI_MAP ai_map = {0};
        ai_map.id = 0;
        ai_map.real_id = tmp_ai_config.id;
        set_ai_map(0, &ai_map);
    }

    return 0;
}