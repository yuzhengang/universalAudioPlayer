package com.yzg.universalaudioplayer.activity;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.yzg.audioplayer.MuteEnum;
import com.yzg.audioplayer.PlayTimeInfoBean;
import com.yzg.audioplayer.UniversalPlayer;
import com.yzg.audioplayer.listener.OnCompleteListener;
import com.yzg.audioplayer.listener.OnErrorListener;
import com.yzg.audioplayer.listener.OnLoadListener;
import com.yzg.audioplayer.listener.OnParparedListener;
import com.yzg.audioplayer.listener.OnPauseResumeListener;
import com.yzg.audioplayer.listener.OnRecordTimeListener;
import com.yzg.audioplayer.listener.OnTimeInfoListener;
import com.yzg.audioplayer.listener.OnValumeDBListener;
import com.yzg.audioplayer.log.PlayerLog;

import com.yzg.universalaudioplayer.R;
import com.yzg.universalaudioplayer.utils.TimeUtil;

import java.io.File;


public class MainActivity extends AppCompatActivity {
    private UniversalPlayer wlPlayer;
    private TextView tvTime;
    private TextView tvVolume;
    private SeekBar seekBarSeek;
    private SeekBar seekBarVolume;
    private int position = 0;
    private boolean isSeekBar = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = (TextView) findViewById(R.id.tv_time);
        seekBarSeek = (SeekBar) findViewById(R.id.seekbar_seek);
        seekBarVolume = (SeekBar) findViewById(R.id.seekbar_volume);
        tvVolume = (TextView) findViewById(R.id.tv_volume);
        wlPlayer = new UniversalPlayer();
        wlPlayer.setVolume(80);
        wlPlayer.setPitch(1.0f);
        wlPlayer.setSpeed(1.0f);
        wlPlayer.setMute(MuteEnum.MUTE_LEFT);
        tvVolume.setText("音量：" + wlPlayer.getVolumePercent() + "%");
        seekBarVolume.setProgress(wlPlayer.getVolumePercent());
        wlPlayer.setOnParparedListener(new OnParparedListener() {
            @Override
            public void onParpared() {
                Log.e("======================","准备好了，可以开始播放声音了");
                wlPlayer.start();
            }
        });
        wlPlayer.setWlOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if(load)
                {
                    PlayerLog.e("加载中...");
                }
                else
                {
                    PlayerLog.e("播放中...");
                }
            }
        });

        wlPlayer.setOnPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause)
                {
                    PlayerLog.e("暂停中...");
                }
                else
                {
                    PlayerLog.e("播放中...");
                }
            }
        });

        wlPlayer.setOnTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTimeInfo(PlayTimeInfoBean timeInfoBean) {
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfoBean;
                handler.sendMessage(message);
            }
        });
        wlPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                PlayerLog.e("code:" + code + ", msg:" + msg);
            }
        });
        wlPlayer.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                PlayerLog.e("播放完成了");
            }
        });

        wlPlayer.setOnValumeDBListener(new OnValumeDBListener() {
            @Override
            public void onDbValue(int db) {
//                MyLog.d("db is: " + db);
            }
        });

        wlPlayer.setOnRecordTimeListener(new OnRecordTimeListener() {
            @Override
            public void onRecordTime(int recordTime) {
                PlayerLog.e("record time is " + recordTime);
            }
        });


        seekBarSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(wlPlayer.getDuration() > 0 && isSeekBar)
                {
                    position = wlPlayer.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                wlPlayer.seek(position);
                isSeekBar = false;
            }
        });

        seekBarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                wlPlayer.setVolume(progress);
                tvVolume.setText("音量：" + wlPlayer.getVolumePercent() + "%");
                Log.d("yzg", "progress is " + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

    }

    public void begin(View view) {
        wlPlayer.setSource("http://file.kuyinyun.com/group1/M00/90/B7/rBBGdFPXJNeAM-nhABeMElAM6bY151.mp3");
        wlPlayer.parpared();
    }

    public void pause(View view) {

        wlPlayer.pause();

    }

    public void resume(View view) {
        wlPlayer.resume();
    }

    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if(msg.what == 1)
            {
                if(!isSeekBar)
                {
                    PlayTimeInfoBean wlTimeInfoBean = (PlayTimeInfoBean) msg.obj;
                    tvTime.setText(TimeUtil.secdsToDateFormat(wlTimeInfoBean.getTotalTime(), wlTimeInfoBean.getTotalTime()) + "/" + TimeUtil.secdsToDateFormat(wlTimeInfoBean.getCurrentTime(), wlTimeInfoBean.getTotalTime()));
                    seekBarSeek.setProgress(wlTimeInfoBean.getCurrentTime() * 100 / wlTimeInfoBean.getTotalTime());
                }
            }
        }
    };

    public void stop(View view) {
        wlPlayer.stop();
    }

    public void seek(View view) {
        wlPlayer.seek(215);
    }

    public void next(View view) {
        wlPlayer.playNext("/mnt/shared/Other/林俊杰 - 背对背拥抱.ape");
    }

    public void left(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_LEFT);
    }

    public void right(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void center(View view) {
        wlPlayer.setMute(MuteEnum.MUTE_CENTER);
    }

    public void speed(View view) {
        wlPlayer.setSpeed(1.5f);
        wlPlayer.setPitch(1.0f);
    }

    public void pitch(View view) {
        wlPlayer.setPitch(1.5f);
        wlPlayer.setSpeed(1.0f);
    }

    public void speedpitch(View view) {
        wlPlayer.setSpeed(1.5f);
        wlPlayer.setPitch(1.5f);
    }

    public void normalspeedpitch(View view) {
        wlPlayer.setSpeed(1.0f);
        wlPlayer.setPitch(1.0f);
    }

    public void start_record(View view) {

        wlPlayer.startRecord(new File("/mnt/shared/Other/textplayer-1.aac"));

    }

    public void pause_record(View view) {
        wlPlayer.pauseRecord();
    }

    public void goon_record(View view) {
        wlPlayer.resumeRcord();
    }

    public void stop_record(View view) {
        wlPlayer.stopRecord();
    }
}
