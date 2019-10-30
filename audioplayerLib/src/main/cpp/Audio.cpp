//
// Created by yangw on 2018-2-28.
//



#include "Audio.h"

Audio::Audio(Playstatus *playstatus, int sample_rate, CallJava *callJava) {
    this->callJava = callJava;
    this->playstatus = playstatus;
    this->sample_rate = sample_rate;

    this->isCut = false;
    this->end_time = 0;
    this->showPcm = false;


    queue = new PlayQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);

    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
}

Audio::~Audio() {

}

void *decodPlay(void *data)
{
    Audio *wlAudio = (Audio *) data;

    wlAudio->initOpenSLES();

    pthread_exit(&wlAudio->thread_play);
}

void *pcmCallBack(void *data)
{
    Audio *audio = static_cast<Audio *>(data);
    while(audio->playstatus != NULL && !audio->playstatus->exit)
    {
        PleyPcmBean *pcmBean = NULL;
        audio->bufferQueue->getBuffer(&pcmBean);
        if(pcmBean == NULL)
        {
            continue;
        }

        LOGD("pcmbean buffer size is %d", pcmBean->buffsize);

        if(pcmBean->buffsize <= audio->defaultPcmSize)//不用分包
        {
            if(audio->isRecordPcm)
            {
                audio->callJava->onCallPcmToAAc(CHILD_THREAD, pcmBean->buffsize, pcmBean->buffer);
            }
            if(audio->showPcm)
            {
                audio->callJava->onCallPcmInfo(pcmBean->buffer, pcmBean->buffsize);
            }
        } else{

            int pack_num = pcmBean->buffsize / audio->defaultPcmSize;
            int pack_sub = pcmBean->buffsize % audio->defaultPcmSize;

            for(int i = 0; i < pack_num; i++)
            {
                char *bf = static_cast<char *>(malloc(audio->defaultPcmSize));
                memcpy(bf, pcmBean->buffer + i * audio->defaultPcmSize, audio->defaultPcmSize);
                if(audio->isRecordPcm)
                {
                    audio->callJava->onCallPcmToAAc(CHILD_THREAD, audio->defaultPcmSize, bf);
                }
                if(audio->showPcm)
                {
                    audio->callJava->onCallPcmInfo(bf, audio->defaultPcmSize);
                }
                free(bf);
                bf = NULL;
            }

            if(pack_sub > 0)
            {
                char *bf = static_cast<char *>(malloc(pack_sub));
                memcpy(bf, pcmBean->buffer + pack_num * audio->defaultPcmSize, pack_sub);
                if(audio->isRecordPcm)
                {
                    audio->callJava->onCallPcmToAAc(CHILD_THREAD, pack_sub, bf);
                }
                if(audio->showPcm)
                {
                    audio->callJava->onCallPcmInfo(bf, pack_sub);
                }
            }
        }
        delete(pcmBean);
        pcmBean = NULL;
    }
    pthread_exit(&audio->pcmCallBackThread);
}

void Audio::play() {

    bufferQueue = new PlayBufferQueue(playstatus);
    pthread_create(&thread_play, NULL, decodPlay, this);
    pthread_create(&pcmCallBackThread, NULL, pcmCallBack, this);

}

int Audio::resampleAudio(void **pcmbuf) {
    data_size = 0;
    while(playstatus != NULL && !playstatus->exit)
    {
        if(playstatus->seek)
        {
            av_usleep(1000 * 100);
            continue;
        }

        if(queue->getQueueSize() == 0)//加载中
        {
            if(!playstatus->load)
            {
                playstatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if(playstatus->load)
            {
                playstatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        if(readFrameFinished)
        {
            avPacket = av_packet_alloc();
            if(queue->getAvpacket(avPacket) != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            ret = avcodec_send_packet(avCodecContext, avPacket);
            if(ret != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if(ret == 0)
        {
            readFrameFinished = false;
            if(avFrame->channels && avFrame->channel_layout == 0)
            {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            }
            else if(avFrame->channels == 0 && avFrame->channel_layout > 0)
            {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(
                                NULL,
                                AV_CH_LAYOUT_STEREO,
                                AV_SAMPLE_FMT_S16,
                                avFrame->sample_rate,
                                avFrame->channel_layout,
                                (AVSampleFormat) avFrame->format,
                                avFrame->sample_rate,
                                NULL, NULL
            );
            if(!swr_ctx || swr_init(swr_ctx) <0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                readFrameFinished = true;
                continue;
            }

            nb = swr_convert(
                swr_ctx,
                &buffer,
                avFrame->nb_samples,
                (const uint8_t **) avFrame->data,
                avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(time_base);
            if(now_time < clock)
            {
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else{
            readFrameFinished = true;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}


int Audio::getSoundTouchData() {

    while(playstatus != NULL && !playstatus->exit)
    {
        out_buffer = NULL;
        if(finished)
        {
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if(data_size > 0)
            {
                for(int i = 0; i < data_size / 2 + 1; i++)
                {
                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else{
                soundTouch->flush();
            }
        }
        if(num == 0)
        {
            finished = true;
            continue;
        } else{
            if(out_buffer == NULL)
            {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if(num == 0)
                {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    Audio *wlAudio = (Audio *) context;
    if(wlAudio != NULL)
    {
        int buffersize = wlAudio->getSoundTouchData();
        if(buffersize > 0)
        {
            wlAudio->clock += buffersize / ((double)(wlAudio->sample_rate * 2 * 2));
            if(wlAudio->clock - wlAudio->last_tiem >= 0.1)
            {
                wlAudio->last_tiem = wlAudio->clock;
                //回调应用层
                wlAudio->callJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }

            wlAudio->bufferQueue->putBuffer(wlAudio->sampleBuffer, buffersize * 4);
            wlAudio->callJava->onCallValumeDB(CHILD_THREAD,
            wlAudio->getPCMDB(reinterpret_cast<char *>(wlAudio->sampleBuffer), buffersize * 4));
            (* wlAudio-> pcmBufferQueue)->Enqueue( wlAudio->pcmBufferQueue, (char *) wlAudio->sampleBuffer, buffersize * 2 * 2);

            if(wlAudio->isCut)
            {
                if(wlAudio->clock > wlAudio->end_time)
                {
                    LOGE("裁剪退出...");
                    wlAudio->playstatus->exit = true;
                }
            }

        }
    }
}

void Audio::initOpenSLES() {

    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[4] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE, SL_IID_MUTESOLO};
    const SLboolean req[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 4, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
//   获取声音接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    //获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    setVolume(volumePercent);
    setMute(mute);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);


}

SLuint32 Audio::getCurrentSampleRateForOpensles(int sample_rate) {
    SLuint32 rate = 0000;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void Audio::pause() {
    if(pcmPlayerPlay != NULL)
    {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PAUSED);
    }
}

void Audio::resume() {
    if(pcmPlayerPlay != NULL)
    {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PLAYING);
    }
}

void Audio::release() {

    if(bufferQueue != NULL)
    {
        bufferQueue->noticeThread();
        pthread_join(pcmCallBackThread, NULL);
        bufferQueue->release();
        delete(bufferQueue);
        bufferQueue = NULL;
    }

    if(queue != NULL)
    {
        delete(queue);
        queue = NULL;
    }

    if(pcmPlayerObject != NULL)
    {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
        pcmMutePlay = NULL;
        pcmVolumePlay = NULL;
    }

    if(outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if(engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if(buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    if(out_buffer != NULL)
    {
        out_buffer = NULL;
    }
    if(soundTouch == NULL)
    {
        delete soundTouch;
        soundTouch = NULL;
    }
    if(sampleBuffer != NULL)
    {
        free(sampleBuffer);
        sampleBuffer = NULL;
    }
    if(avCodecContext != NULL)
    {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if(playstatus != NULL)
    {
        playstatus = NULL;
    }
    if(callJava != NULL)
    {
        callJava = NULL;
    }

}

void Audio::stop() {
    if(pcmPlayerPlay != NULL)
    {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_STOPPED);
    }
}

void Audio::setVolume(int percent) {
    volumePercent = percent;
    if(pcmVolumePlay != NULL)
    {
        if(percent > 30)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }
        else if(percent > 25)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }
        else if(percent > 20)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }
        else if(percent > 15)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }
        else if(percent > 10)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }
        else if(percent > 5)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }
        else if(percent > 3)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }
        else if(percent > 0)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }
        else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void Audio::setMute(int mute) {
    this->mute = mute;
    if(pcmMutePlay != NULL)
    {
        if(mute == 0)//right
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        }
        else if(mute == 1)//left
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
        else if(mute == 2)//center
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }


    }

}

void Audio::setPitch(float pitch) {
    this->pitch = pitch;
    if(soundTouch != NULL)
    {
        soundTouch->setPitch(pitch);
    }
}

void Audio::setSpeed(float speed) {
    this->speed = speed;
    if(soundTouch != NULL)
    {
        soundTouch->setTempo(speed);
    }
}

int Audio::getPCMDB(char *pcmcata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for(int i = 0; i < pcmsize; i+= 2)
    {
        memcpy(&pervalue, pcmcata+i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if(sum > 0)
    {
        db = (int)20.0 *log10(sum);
    }
    return db;
}

void Audio::startStopRecord(bool start) {

    this->isRecordPcm = start;

}

