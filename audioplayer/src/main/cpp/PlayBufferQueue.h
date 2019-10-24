//
// Created by admin on 2019/10/22.
//

#ifndef UNIVERSALAUDIOPLAYER_PLAYBUFFERQUEUE_H
#define UNIVERSALAUDIOPLAYER_PLAYBUFFERQUEUE_H

#include "deque"
#include "Playstatus.h"
#include "PlayPcmBean.h"

extern "C"{
   #include "include/libavcodec/avcodec.h"
   #include "pthread.h"
};

class  PlayBufferQueue{
public:
     std::deque<PleyPcmBean *>  queueBuffer;
     pthread_mutex_t mutexBuffer;
     pthread_cond_t  condBuffer;
     Playstatus   *playstatus=NULL;
public:
     PlayBufferQueue(Playstatus  *playStatus);
     ~PlayBufferQueue();

     int putBuffer(SAMPLETYPE *buffer, int size);
     int getBuffer(PleyPcmBean  **pcmBean);

     int  clearBuffer(  );
     void release( );
     int  getBufferSize( );
     int  noticeThread( );
};
#endif //UNIVERSALAUDIOPLAYER_PLAYBUFFERQUEUE_H



