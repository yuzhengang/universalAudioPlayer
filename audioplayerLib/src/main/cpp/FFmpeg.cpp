//
// Created by admin on 2019/10/29.
//
#include "FFmpeg.h"


FFmpeg::FFmpeg(Playstatus *playstatus, CallJava *callJava, const char *url) {
    this->playstatus=playstatus;
    this->callJava=callJava;
    this->url=url;
    exit= false;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}
void *decodeFFmpeg(void *data)
{
    LOGE("===============开始解码");
    FFmpeg *wlFFmpeg = (FFmpeg *) data;
    wlFFmpeg->decodeFFmpegThread();
    pthread_exit(&wlFFmpeg->decodeThread);
}


void FFmpeg::parpared() {
    LOGE("===============开始准备");
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

int avformat_callback(void *ctx)
{
    FFmpeg *fFmpeg = (FFmpeg *) ctx;
    if(fFmpeg->playstatus->exit)
    {
        return AVERROR_EOF;
    }
    return 0;
}

void FFmpeg::decodeFFmpegThread() {


    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;

    if(avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not open url :%s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not find streams from %s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not find streams from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
        {
            if(audio == NULL)
            {
                audio = new Audio(playstatus, pFormatCtx->streams[i]->codecpar->sample_rate, callJava);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
                callJava->onCallPcmRate(audio->sample_rate);
            }
        }
    }

    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);
    if(!dec)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(dec);
    if(!audio->avCodecContext)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not alloc new decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if(avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not fill decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "ccan not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if(avcodec_open2(audio->avCodecContext, dec, 0) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("cant not open audio strames");
        }
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if(callJava != NULL)
    {
        if(playstatus != NULL && !playstatus->exit)
        {
            callJava->onCallParpared(CHILD_THREAD);
        } else{
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

void FFmpeg::start() {

    if(audio == NULL)
    {
        return;
    }
    audio->play();
    int count = 0;
    while(playstatus != NULL && !playstatus->exit)
    {
        if(playstatus->seek)
        {
            av_usleep(1000 * 100);
            continue;
        }

        if(audio->queue->getQueueSize() > 40)
        {
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        if(av_read_frame(pFormatCtx, avPacket) == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                count++;
                LOGE("解码第 %d 帧", count);
                audio->queue->putAvpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            while(playstatus != NULL && !playstatus->exit)
            {
                if(audio->queue->getQueueSize() > 0)
                {
                    av_usleep(1000 * 100);
                    continue;
                } else{
                    playstatus->exit = true;
                    break;
                }
            }
            break;
        }
    }
    if(callJava != NULL)
    {
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;

}

void FFmpeg::pause() {
    if(audio != NULL)
    {
        audio->pause();
    }
}

void FFmpeg::resume() {
    if(audio != NULL)
    {
        audio->resume();
    }
}

void FFmpeg::release() {

    if(LOG_DEBUG)
    {
        LOGE("开始释放Ffmpe");
    }
    playstatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit)
    {
        if(sleepCount > 1000)
        {
            exit = true;
        }
        if(LOG_DEBUG)
        {
            LOGE("wait ffmpeg  exit %d", sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }

    if(LOG_DEBUG)
    {
        LOGE("释放 Audio");
    }

    if(audio != NULL)
    {
        audio->release();
        delete(audio);
        audio = NULL;
    }

    if(LOG_DEBUG)
    {
        LOGE("释放 封装格式上下文");
    }
    if(pFormatCtx != NULL)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 callJava");
    }
    if(callJava != NULL)
    {
        callJava = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("释放 playstatus");
    }
    if(playstatus != NULL)
    {
        playstatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

  FFmpeg::~FFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void FFmpeg::seek(int64_t secds) {

    if(duration <= 0)
    {
        return;
    }
    if(secds >= 0 && secds <= duration)
    {
        if(audio != NULL)
        {
            playstatus->seek = true;
            audio->queue->clearAvpacket();
            audio->clock = 0;
            audio->last_tiem = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = secds * AV_TIME_BASE;
            avcodec_flush_buffers(audio->avCodecContext);
            avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playstatus->seek = false;
        }
    }
}

void FFmpeg::setVolume(int percent) {
    if(audio != NULL)
    {
        audio->setVolume(percent);
    }
}

void FFmpeg::setMute(int mute) {
    if(audio != NULL)
    {
        audio->setMute(mute);
    }
}

void FFmpeg::setPitch(float pitch) {

    if(audio != NULL)
    {
        audio->setPitch(pitch);
    }

}

void FFmpeg::setSpeed(float speed) {

    if(audio != NULL)
    {
        audio->setSpeed(speed);
    }

}

int FFmpeg::getSampleRate() {
    if(audio != NULL)
    {
        return audio->avCodecContext->sample_rate;
    }
    return 0;
}

void FFmpeg::startStopRecord(bool start) {
    if(audio != NULL)
    {
        audio->startStopRecord(start);
    }

}

bool FFmpeg::cutAudioPlay(int start_time, int end_time, bool showPcm) {

    if(start_time >= 0 && end_time <= duration && start_time < end_time)
    {
        audio->isCut = true;
        audio->end_time = end_time;
        audio->showPcm = showPcm;
        seek(start_time);
        return true;
    }
    return false;
}
