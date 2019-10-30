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

    }
}
