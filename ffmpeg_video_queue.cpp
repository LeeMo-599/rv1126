#include "ffmpeg_video_queue.h"

VIDEO_QUEUE::VIDEO_QUEUE()
{
    pthread_mutex_init(&videoMutex, NULL);
    pthread_cond_init(&videoCond, NULL);
}

VIDEO_QUEUE ::~VIDEO_QUEUE()
{
    pthread_mutex_destroy(&videoMutex);
    pthread_cond_destroy(&videoCond);
}

int VIDEO_QUEUE::putVideoPacketQueue(video_data_packet_t *video_packet)
{
    pthread_mutex_lock(&videoMutex);
    video_packet_queue.push(video_packet);
    pthread_cond_broadcast(&videoCond);
    pthread_mutex_unlock(&videoMutex);
    return 0;
}

video_data_packet_t *VIDEO_QUEUE::getVideoPacketQueue()
{
    pthread_mutex_lock(&videoMutex);
    while (video_packet_queue.size() == 0)
    {
        pthread_cond_wait(&videoCond, &videoMutex);
    }
    video_data_packet_t *item = video_packet_queue.front();
    video_packet_queue.pop();
    pthread_mutex_unlock(&videoMutex);
    return item;
}

int VIDEO_QUEUE::getVideoQueueSize()
{
    unsigned int count = 0;
    pthread_mutex_lock(&videoMutex);
    count = video_packet_queue.size();
    pthread_mutex_unlock(&videoMutex);
    return count;
}
