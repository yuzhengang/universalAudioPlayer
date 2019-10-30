package com.yzg.universalaudioplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.yzg.audioplayer.UniversalPlayer;


public class MainActivity extends AppCompatActivity {
   UniversalPlayer universalPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        universalPlayer=new UniversalPlayer();
        universalPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
    }
}
