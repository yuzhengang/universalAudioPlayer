//
// Created by admin on 2019/10/28.
//

#ifndef UNIVERSALAUDIOPLAYER_AUDIO_H
#define UNIVERSALAUDIOPLAYER_AUDIO_H

#include "PlayerQueue.h"
#include "Playstatus.h"
#include "CallJava.h"
#include "soundtouch/include/SoundTouch.h"
#include "PlayBufferQueue.h"
#include "PlayPcmBean.h"
using namespace soundtouch;

extern "C"
{
#include "include/libavcodec/avcodec.h"
#include "include/libswresample/swresample.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "include/libavutil/time.h"
};
class  Audio{
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    PlayQueue *queue=NULL;
    Playstatus  *playstatus=NULL;
    CallJava *callJava=NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double clock;//总的播放时长
    double now_time;//当前frame时间
    double last_tiem; //上一次调用时间

    int volumePercent = 100;
    int mute = 2;

    float pitch = 1.0f;
    float speed = 1.0f;

    bool isRecordPcm = false;
    bool readFrameFinished = true;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf  pcmMutePlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    //SoundTouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool finished = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;
    bool isCut = false;
    int end_time = 0;
    bool showPcm = false;

    pthread_t pcmCallBackThread;
    PlayBufferQueue *bufferQueue = NULL;
    int defaultPcmSize = 4096;

public:

    Audio(Playstatus *playstatus, int sample_rate,CallJava  *callJava);
    ~Audio();

    void play();
    int resampleAudio(void **pcmbuf);

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPCMDB(char *pcmcata, size_t pcmsize);

    void startStopRecord(bool start);


};
#endif //UNIVERSALAUDIOPLAYER_AUDIO_H
















