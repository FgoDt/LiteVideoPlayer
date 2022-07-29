package com.lvp.core;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.provider.MediaStore;
import android.util.Log;

public class AudioTrackRender {

    private AudioTrack player = null;

    private int channels = 0;
    private int format = -1;
    private int sampleRate = 0;
    private short[] buf = null;
    private int wpos = 0;

    public AudioTrackRender(){
        maybeReInitPlayer(2,1,22050);
    }

    private void maybeReInitPlayer(int channels, int format, int sampleRate) {
        if (player != null) {
            return;
        }
        player = new AudioTrack.Builder()
                .setAudioAttributes(
                        new AudioAttributes.Builder()
                                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                .setUsage(AudioAttributes.USAGE_MEDIA).build())
                .setAudioFormat(new AudioFormat.Builder()
                        .setChannelMask(AudioFormat.CHANNEL_IN_STEREO)
                        .setSampleRate(sampleRate)
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT).build())
                .build();
        player.play();
    }

    public int onAudioDataUpdate(short[] data, int channels, int format, int sampleRate){
        maybeReInitPlayer(channels,format,sampleRate);
        int ret = 0;
        if(this.buf != null){
            ret = -11;
            int len = player.write(this.buf, wpos, buf.length - wpos, AudioTrack.WRITE_NON_BLOCKING);
            wpos += len;
            if(wpos == buf.length){
                this.buf = null;
            }
            return  ret;
        }else{
            int len = player.write(data,0, data.length, AudioTrack.WRITE_NON_BLOCKING);
            if(len != data.length){
                wpos = len;
                this.buf = data;
            }
            return 0;
        }
    }
}
