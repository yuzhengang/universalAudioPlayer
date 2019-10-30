package com.yzg.audioplayer;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.text.TextUtils;

import com.yzg.audioplayer.listener.WlOnCompleteListener;
import com.yzg.audioplayer.listener.WlOnErrorListener;
import com.yzg.audioplayer.listener.WlOnLoadListener;
import com.yzg.audioplayer.listener.WlOnParparedListener;
import com.yzg.audioplayer.listener.WlOnPauseResumeListener;
import com.yzg.audioplayer.listener.WlOnPcmInfoListener;
import com.yzg.audioplayer.listener.WlOnRecordTimeListener;
import com.yzg.audioplayer.listener.WlOnTimeInfoListener;
import com.yzg.audioplayer.listener.WlOnValumeDBListener;
import com.yzg.audioplayer.log.PlayerLog;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by admin on 2019/10/29.
 */

public class UniversalPlayer {

    public   UniversalPlayer( ){

    }

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

    private static String source;//数据源
    private static PlayTimeInfoBean wlTimeInfoBean;
    private static boolean playNext = false;
    private static int duration = -1;
    private static int volumePercent = 100;
    private static float speed = 1.0f;
    private static float pitch = 1.0f;
    private static boolean initmediacodec = false;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private WlOnParparedListener wlOnParparedListener;
    private WlOnLoadListener wlOnLoadListener;
    private WlOnPauseResumeListener wlOnPauseResumeListener;
    private WlOnTimeInfoListener wlOnTimeInfoListener;
    private WlOnErrorListener wlOnErrorListener;
    private WlOnCompleteListener wlOnCompleteListener;
    private WlOnValumeDBListener wlOnValumeDBListener;
    private WlOnRecordTimeListener wlOnRecordTimeListener;
    private WlOnPcmInfoListener wlOnPcmInfoListener;

    /**
     * 设置数据源
     *
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    /**
     * 设置准备接口回调
     *
     * @param wlOnParparedListener
     */
    public void setWlOnParparedListener(WlOnParparedListener wlOnParparedListener) {
        this.wlOnParparedListener = wlOnParparedListener;
    }

    public void setWlOnLoadListener(WlOnLoadListener wlOnLoadListener) {
        this.wlOnLoadListener = wlOnLoadListener;
    }

    public void setWlOnPauseResumeListener(WlOnPauseResumeListener wlOnPauseResumeListener) {
        this.wlOnPauseResumeListener = wlOnPauseResumeListener;
    }

    public void setWlOnTimeInfoListener(WlOnTimeInfoListener wlOnTimeInfoListener) {
        this.wlOnTimeInfoListener = wlOnTimeInfoListener;
    }

    public void setWlOnErrorListener(WlOnErrorListener wlOnErrorListener) {
        this.wlOnErrorListener = wlOnErrorListener;
    }

    public void setWlOnCompleteListener(WlOnCompleteListener wlOnCompleteListener) {
        this.wlOnCompleteListener = wlOnCompleteListener;
    }

    public void setWlOnValumeDBListener(WlOnValumeDBListener wlOnValumeDBListener) {
        this.wlOnValumeDBListener = wlOnValumeDBListener;
    }

    public void setWlOnRecordTimeListener(WlOnRecordTimeListener wlOnRecordTimeListener) {
        this.wlOnRecordTimeListener = wlOnRecordTimeListener;
    }

    public void setWlOnPcmInfoListener(WlOnPcmInfoListener wlOnPcmInfoListener) {
        this.wlOnPcmInfoListener = wlOnPcmInfoListener;
    }

    public void parpared() {
        if (TextUtils.isEmpty(source)) {
            PlayerLog.d("source not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();
    }

    private native void n_parpared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int secds);

    private native int n_duration();

    private native void n_volume(int percent);

    private native void n_mute(int mute);

    private native void n_pitch(float pitch);

    private native void n_speed(float speed);

    private native int n_samplerate();

    private native void n_startstoprecord(boolean start);

    private native boolean n_cutaudioplay(int start_time, int end_time, boolean showPcm);


    //mediacodec
    private MediaFormat encoderFormat=null;
    private MediaCodec  encoder=null;
    private FileOutputStream   outputStream=null;
    private MediaCodec.BufferInfo  info=null;
    private int perpcmsize=0;
    private byte[]  outByteBuffer=null;
    private int aacsamplerate = 4;
    private  double recordTime=0;
    private int audioSamplerate=0;


   @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
   private void initMediacodec(int samperate, File   outfile){
       aacsamplerate=getADTSsamplerate(samperate);
       encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samperate, 2);
       encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE,96000);
       encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
       encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
       try {
           encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
           info = new MediaCodec.BufferInfo();
           if(encoder==null){
               PlayerLog.d("create encoder wrong");
               return;
           }
           recordTime=0;
           encoder.configure(encoderFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
           outputStream = new FileOutputStream(outfile);
           encoder.start();
       } catch (IOException e) {
           e.printStackTrace();
       }

   }


    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private void encodecPcmToAAc(int size, byte[] buffer)
    {
        if(buffer != null && encoder != null)
        {
            recordTime += size * 1.0 / (audioSamplerate * 2 * (16 / 8));
            if(wlOnRecordTimeListener != null)
            {
                wlOnRecordTimeListener.onRecordTime((int) recordTime);
            }
            int inputBufferindex = encoder.dequeueInputBuffer(0);
            if(inputBufferindex >= 0)
            {
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferindex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferindex, 0, size, 0, 0);
            }
            int index = encoder.dequeueOutputBuffer(info, 0);
            while(index >= 0)
            {
                try {
                    perpcmsize = info.size + 7;
                    outByteBuffer = new byte[perpcmsize];

                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);

                    addADtsHeader(outByteBuffer, perpcmsize, aacsamplerate);

                    byteBuffer.get(outByteBuffer, 7, info.size);
                    byteBuffer.position(info.offset);
                    outputStream.write(outByteBuffer, 0, perpcmsize);

                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(info, 0);
                    outByteBuffer = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private void releaseMedicacodec()
    {
        if(encoder == null)
        {
            return;
        }
        try {
            recordTime = 0;
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            info = null;
            initmediacodec = false;
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            if(outputStream != null)
            {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate)
    {
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }








    private int getADTSsamplerate(int samplerate)
    {
        int rate = 4;
        switch (samplerate)
        {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }



}











