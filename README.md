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
  *播放音频*
      
     UniversalPlayer player= UniversalPlayer.getInstance();
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
    
 #### API介绍（v1.0.1）   
 
     public void setSource(String source) //设置音频源
       
     public void parpared() //准备播放
     
     public void start() //开始播放

     public void pause() //暂停播放
     
     public void resume() //恢复播放（对应于暂停）
     
     public void stop() //停止播放，回收资源
     
     public boolean isPlaying(); //是否正在播放中
     
     public void playNext(String source) //切换播放源

     public void seek(int secds) //secds：时间（秒） 

     public void setVolume(int percent) //设置音量（0~100）
     
     public int  getDuration() //获取时长
     
     public int getVolumePercent()//获取当前音量
     
     public void setSpeed(int speed) //设置播放速度（默认正常速度 1.0 范围：0.25x ~ 4.0x）
    
     public void setPitch(float pitch) //设置音频音调（默认正常音调 1.0 范围：0.25x ~ 4.0x）   
     
     public void setMute(MuteEnum mute) //设置播放声道 （MuteEnum.MUTE_LEFT,MuteEnum.MUTE_RIGHT,MuteEnum.MUTE_CENTER）
     
     public void startRecord(File outfile) //开始录制音频
     
     public void stopRecord() // 停止录制
          
     public void pauseRecord() //暂停录制

     public void resumeRecord() //恢复录制
     
     public void setOnErrorListener(OnErrorListener onErrorListener) //出错回调
     
     public void setOnLoadListener(OnLoadListener onLoadListener) //加载回调
     
     public void setOnTimeInfoListener(OnTimeInfoListener onTimeInfoListener) //播放进度回调
     
     public void setOnParparedListener(OnParparedListener onParparedListener) //准备播放完成回调
     
     public void setOnCompleteListener(OnCompleteListener onCompleteListener) //播放完成回调

     public void setOnPauseResumeListener(OnPauseResumeListener onPauseResumeListener) //暂停、恢复回调
     
     public void setOnVolumeDBListener(OnVolumeDBListener onVolumeDBListener)//声音分贝大小回调
     
     
#### 技术要点和代码说明

 *https://github.com/yuzhengang/universalAudioPlayer/blob/master/%E6%8A%80%E6%9C%AF%E8%AF%B4%E6%98%8E.md*
     
    
     
     
