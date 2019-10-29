//
// Created by admin on 2019/10/23.
//

#ifndef UNIVERSALAUDIOPLAYER_FFMPEG_H
#define UNIVERSALAUDIOPLAYER_FFMPEG_H
#include "CallJava.h"
#include "pthread.h"
#include "Playstatus.h"
#include "Audio .h"
extern  "C"
{
    #include "include/libavformat/avformat.h"
    #include "include/libavutil/time.h"
};
class FFmpeg {

public:
    CallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    Audio *audio = NULL;
    Playstatus *playstatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;
public:
    FFmpeg(Playstatus *playstatus, CallJava *callJava, const char *url);
    ~FFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t secds);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startStopRecord(bool start);

    bool cutAudioPlay(int start_time, int end_time, bool showPcm);

};
#endif //UNIVERSALAUDIOPLAYER_FFMPEG_H














