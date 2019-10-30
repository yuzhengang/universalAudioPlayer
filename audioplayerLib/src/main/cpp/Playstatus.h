  //
// Created by admin on 2019/10/22.
//

#ifndef UNIVERSALAUDIOPLAYER_PLAYSTATUS_H
#define UNIVERSALAUDIOPLAYER_PLAYSTATUS_H
class  Playstatus{
public:
    bool  exit= false;
    bool  load= true;
    bool  seek= false;
public:
    Playstatus( );
    ~Playstatus( );
};
#endif //UNIVERSALAUDIOPLAYER_PLAYSTATUS_H
