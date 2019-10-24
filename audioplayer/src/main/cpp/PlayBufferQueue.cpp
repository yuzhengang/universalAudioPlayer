//
// Created by admin on 2019/10/22.
//
#include "PlayBufferQueue.h"
#include "AndroidLog.h"
PlayBufferQueue::PlayBufferQueue(Playstatus *playStatus) {
    playstatus=playStatus;
    pthread_mutex_init(&mutexBuffer,NULL);
    pthread_cond_init(&condBuffer,NULL);
}

PlayBufferQueue::~PlayBufferQueue() {
    playstatus=NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if(LOG_DEBUG){
        LOGE("PlayBufferQueue 释放完了");
    }
}

int PlayBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    PleyPcmBean  *pcmBean=new PleyPcmBean(buffer,size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int PlayBufferQueue::getBuffer(PleyPcmBean **pcmBean) {
     pthread_mutex_lock(&mutexBuffer);
     while (playstatus!=NULL&&!playstatus->exit){
         if(queueBuffer.size()>0){
             *pcmBean=queueBuffer.front();
             queueBuffer.pop_front();
         }else{
             if(!playstatus->exit){
                 pthread_cond_wait(&condBuffer,&mutexBuffer);
             }
         }
     }
    pthread_mutex_unlock(&mutexBuffer);
    return  0;
}

int  PlayBufferQueue::clearBuffer() {
    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while(!queueBuffer.empty()){
       PleyPcmBean  *pcmBean=queueBuffer.front();
       queueBuffer.pop_front();
       delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return  0;
}

int  PlayBufferQueue::getBufferSize() {
    int size=0;
    pthread_mutex_lock(&mutexBuffer);
    size=queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
}

int PlayBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return  0;
}















