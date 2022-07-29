package com.lvp.core;

import android.view.Surface;

public class LvpNative {

    // Used to load the 'core' library on application startup.
    static {
        System.loadLibrary("lvpjni");
    }

    private long corePtr = 0;

    private native long createCore();

    private native boolean play(long corePtr,String url);

    private native void surfaceCreated(long corePtr, Surface view);

    private native void surfaceDestroyed(long corePtr, Surface view);

    public LvpNative(){
        corePtr = createCore();
    }

    public boolean play(String url){
        return play(corePtr, url);
    }

    public void surfaceCreated(Surface view){
        surfaceCreated(corePtr, view);
    }

    public void surfaceDestroyed(Surface view){
        surfaceDestroyed(corePtr, view);
    }

 //   public

}