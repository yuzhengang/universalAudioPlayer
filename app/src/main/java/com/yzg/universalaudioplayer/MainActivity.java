package com.yzg.universalaudioplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.SeekBar;
import android.widget.TextView;

import com.yzg.audioplayer.MuteEnum;
import com.yzg.audioplayer.UniversalPlayer;
import com.yzg.audioplayer.listener.OnParparedListener;


public class MainActivity extends AppCompatActivity {
    UniversalPlayer universalPlayer;
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
        universalPlayer=new UniversalPlayer();
        tvTime = (TextView) findViewById(R.id.tv_time);
        seekBarSeek = (SeekBar) findViewById(R.id.seekbar_seek);
        seekBarVolume = (SeekBar) findViewById(R.id.seekbar_volume);
        tvVolume = (TextView) findViewById(R.id.tv_volume);
        universalPlayer = new UniversalPlayer();
        universalPlayer.setVolume(80);
        universalPlayer.setPitch(1.0f);
        universalPlayer.setSpeed(1.0f);
        universalPlayer.setMute(MuteEnum.MUTE_LEFT);
        tvVolume.setText("音量：" + universalPlayer.getVolumePercent() + "%");
        seekBarVolume.setProgress(universalPlayer.getVolumePercent());
        universalPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        universalPlayer.parpared();
        universalPlayer.setOnParparedListener(new OnParparedListener() {
            @Override
            public void onParpared() {
                universalPlayer.start();
            }
        });
    }
}
