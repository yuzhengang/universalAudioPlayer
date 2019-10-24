//
// Created by admin on 2019/10/22.
//
#include "PlayPcmBean.h"
PleyPcmBean::PleyPcmBean(SAMPLETYPE *buffer, int size) {
    this->buffer= (char *) malloc(size);
    this->buffsize=size;
    memcpy(this->buffer,buffer,size);
}

PleyPcmBean::~PleyPcmBean() {
    free(buffer);
    buffer=NULL;
}
