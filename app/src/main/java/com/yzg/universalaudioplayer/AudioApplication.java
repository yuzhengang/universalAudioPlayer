package com.yzg.universalaudioplayer;

import android.app.Application;
import android.content.Context;

/**
 * Created by admin on 2020/2/21.
 */

public class AudioApplication  extends Application {

    public static Context context;

    @Override
    public void onCreate() {
        super.onCreate();
        context = this;
    }




}
