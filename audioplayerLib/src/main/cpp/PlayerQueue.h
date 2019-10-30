//
// Created by admin on 2019/10/22.
//

#ifndef UNIVERSALAUDIOPLAYER_PLAYERQUEUE_H
#define UNIVERSALAUDIOPLAYER_PLAYERQUEUE_H

#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "Playstatus.h"

extern "C"{
    #include "include/libavcodec/avcodec.h"
};

class  PlayQueue{
public:
    std::queue<AVPacket *>  queuePacket;
    pthread_mutex_t  mutexPacket;
    pthread_cond_t   condPacket;
    Playstatus    *playstatus=NULL;

public:
    PlayQueue(Playstatus *playstatus);
    ~PlayQueue();

    int  putAvpacket(AVPacket *packet);

    int  getAvpacket(AVPacket *packet);

    int  getQueueSize( );

    void clearAvpacket( );
};

#endif //UNIVERSALAUDIOPLAYER_PLAYERQUEUE_H











