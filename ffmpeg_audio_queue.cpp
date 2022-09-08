#include "ffmpeg_audio_queue.h"

AUDIO_QUEUE::AUDIO_QUEUE()
{
    pthread_mutex_init(&audioMutex, NULL);
    pthread_cond_init(&audioCond, NULL);
}

AUDIO_QUEUE ::~AUDIO_QUEUE()
{
    pthread_mutex_destroy(&audioMutex);
    pthread_cond_destroy(&audioCond);
}

int AUDIO_QUEUE::putAudioPacketQueue(audio_data_packet *audio_packet)
{
    pthread_mutex_lock(&audioMutex);
    audio_packet_queue.push(audio_packet);
    pthread_cond_broadcast(&audioCond);
    pthread_mutex_unlock(&audioMutex);
    return 0;
}

audio_data_packet *AUDIO_QUEUE::getAudioPacketQueue()
{
    pthread_mutex_lock(&audioMutex);
    while (audio_packet_queue.size() == 0)
    {
        pthread_cond_wait(&audioCond, &audioMutex);
    }
    audio_data_packet *item = audio_packet_queue.front();
    audio_packet_queue.pop();
    pthread_mutex_unlock(&audioMutex);
    return item;
}

int AUDIO_QUEUE::getAudioPacketQueueSize()
{
    unsigned int count = 0;
    pthread_mutex_lock(&audioMutex);
    count = audio_packet_queue.size();
    pthread_mutex_unlock(&audioMutex);
    return count;
}
