package com.lvp.core;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class LvpPlayerView extends GLSurfaceView implements SurfaceHolder.Callback {

    private LvpNative lvpNative;

    public LvpPlayerView(Context context) {
        super(context);
        initEGL();
    }

    public LvpPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initEGL();
    }

    private void initEGL(){
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8,8,8,8,8,0);
        setRenderer(new Renderer());
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        getHolder().addCallback(this);
        this.lvpNative = new LvpNative();
    }


    public void play(String url){
        this.lvpNative.play(url);
    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        Log.d("LVP_VIEW","surface changed:" + holder.getSurface().hashCode());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("LVP_VIEW","surface destroyed:" + holder.getSurface().hashCode());
        if(this.lvpNative!=null){
            this.lvpNative.surfaceDestroyed(holder.getSurface());
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if(this.lvpNative != null) {
            this.lvpNative.surfaceCreated(holder.getSurface());
        }
        Log.d("LVP_VIEW","surface created:" + holder.getSurface().hashCode());
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            // GLES3JNILib.step();
            Log.d("LVPJNIE", "DrawFrame");
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //  GLES3JNILib.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // GLES3JNILib.init();
        }
    }
}
