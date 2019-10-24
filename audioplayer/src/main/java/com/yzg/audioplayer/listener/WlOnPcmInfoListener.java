package com.yzg.audioplayer.listener;

public interface WlOnPcmInfoListener {

    void onPcmInfo(byte[] buffer, int buffersize);

    void onPcmRate(int samplerate, int bit, int channels);

}
