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
            playerView.play("http://video.xthktech.cn/customerTrans/1070d02a7f655820c5359aa22e24c369/2973309f-17f86254c4a-0004-8f57-aa3-1ed3d.mp4?auth_key=1659342984-28463040394e4177bbb5251e84a83f42-0-7dcb35019c55d2147e80663dee1dd770");
        });
    }

    public void onPlayClick(View view){
    }
}