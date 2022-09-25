#include "ffmpeg_module.h"
#include "ffmpeg_group.h"
#include "rv1126_map.h"
#include "ffmpeg_audio_queue.h"
#include "ffmpeg_video_queue.h"
#include "rv1126_task_manage.h"
#include "rv1126_isp_function.h"
#include "rv1126_vi_ai_function.h"

VIDEO_QUEUE * video_queue = NULL;
AUDIO_QUEUE * audio_queue = NULL;

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("Please Input ./rv1126_ffmpeg_main stream_type url_address. Notice URL_TYPE: 0-->RTMP  1-->SRT  2-->LOCAL\n");
        return -1;
    }

    int url_type = atoi(argv[1]);
    char * url_address = argv[2];

    video_queue = new VIDEO_QUEUE();
    audio_queue = new AUDIO_QUEUE();

    init_all_isp_function();
    init_ffmpeg_manage_function();
    init_vi_ai_function();
    init_rv1126_first_task(url_type, url_address);
    
    while (1)
    {
       sleep(20);
    }
    
    return 0;
}