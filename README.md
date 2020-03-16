## C++  NDK  FFmpeg  OpenSL ES 音频播放器

#### 使用方法
*在project的build.gradle添加如下代码*

    allprojects {
	    repositories {
	        ...
	        maven { url "https://jitpack.io" }
	    }
	}

*在Module的build.gradle添加依赖*
  
    implementation 'com.github.yuzhengang:universalAudioPlayer:1.0.1'
   
*需要权限:*
    
    <uses-permission android:name="android.permission.INTERNET"/>
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/>
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"/>
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
    
#### 接入代码
  *1、播放音频*
      
     UniversalPlayer player=new UniversalPlayer();
     player.setSource("http://file.kuyinyun.com/group1/M00/90/B7/rBBGdFPXJNeAM-nhABeMElAM6bY151.mp3"); //设置音频源
     player.setVolume(65); //设置音量 65%
     player.setSpeed(1.0f); //设置播放速度 (1.0正常) 范围：0.25---4.0f
     player.setPitch(1.0f); //设置播放速度 (1.0正常) 范围：0.25---4.0f
     player.setMute(MuteEnum.MUTE_CENTER); //设置立体声（左声道、右声道和立体声）
     player.parpared();准备开始
     player.setOnPreparedListener(new OnPreparedListener() {
        @Override
        public void onPrepared() {
           player.start(); //准备完成开始播放
        }
     });
     
    //seek时间
    seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
       @Override
       public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
          position = wlMusic.getDuration() * progress / 100;
       }

       @Override
       public void onStartTrackingTouch(SeekBar seekBar) {
         player.seek(position, false, false);// 表示在seeking中，此时不回调时间
       }

       @Override
       public void onStopTrackingTouch(SeekBar seekBar) {
        player.seek(position, true, true);//表示seek已经完成，然后才回调时间，避免自己控制时间逻辑和时间显示不稳定问题。
       }
    });
    
    
    
    
    
