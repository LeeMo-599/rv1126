#include "rkmedia_ffmpeg_config.h"
#include "rkmedia_container.h"
#include "ffmpeg_audio_queue.h"
#include "ffmpeg_video_queue.h"
#include "rkmedia_module_function.h"
#include "rkmedia_assignment_manage.h"

VIDEO_QUEUE * video_queue = NULL;
AUDIO_QUEUE * audio_queue = NULL;

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("Please Input ./rv1126_ffmpeg_main stream_type url_address. Notice URL_TYPE: 0-->FLV  1-->TS\n");
        return -1;
    }

    int protocol_type = atoi(argv[1]);
    char * network_address = argv[2];

    video_queue = new VIDEO_QUEUE(); //初始化所有VIDEO队列
    audio_queue = new AUDIO_QUEUE(); //初始化所有AUDIO队列

    init_rkmedia_ffmpeg_function();  //
    init_rkmedia_module_function();  //初始化所有rkmedia的模块
    init_rv1126_first_assignment(protocol_type, network_address);  //开启推流任务
    
    while (1)
    {
       sleep(20);
    }
    
    return 0;
}
