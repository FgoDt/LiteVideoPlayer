package com.lvp.player;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.lvp.core.LvpNative;
import com.lvp.core.LvpPlayerView;

public class MainActivity extends AppCompatActivity {

    private LvpPlayerView playerView = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        playerView = findViewById(R.id.lvpview);
        Button button = findViewById(R.id.button);
        button.setOnClickListener((View view)->{
            playerView.play("http://vod.xthk.cn/customerTrans/4095636471ceeea04387b2562c872e3e/1996517d-1823f750ce9-0004-f799-d83-5424c.mp4?auth_key=1659085942-9ca0988e00694b499b99b7c44ae64250-0-8ca35b4ef54736a60aa783073ec713b5");
        });
    }

    public void onPlayClick(View view){
    }
}