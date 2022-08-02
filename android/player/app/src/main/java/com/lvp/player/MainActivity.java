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
            playerView.play("http://video-static-cdn.xthkwx.cn/vod/live.mp4");
        });
    }

    public void onPlayClick(View view){
    }
}