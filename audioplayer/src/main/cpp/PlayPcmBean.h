//
// Created by admin on 2019/10/22.
//

#ifndef UNIVERSALAUDIOPLAYER_PLAYPCMBEAN_H
#define UNIVERSALAUDIOPLAYER_PLAYPCMBEAN_H

#include "soundtouch/include/SoundTouch.h"
using namespace  soundtouch;
class  PleyPcmBean{
public:
    char  *buffer;
    int  buffsize;
public:
    PleyPcmBean(SAMPLETYPE *buffer, int  size);
    ~PleyPcmBean();
};
#endif //UNIVERSALAUDIOPLAYER_PLAYPCMBEAN_H
