# Option
LVP使用字符串来初始化设置
例如
```c
lvp_core_set_options(core,"-reader myreader -video_decoder MY_MediaCodecDecoder");
```


## Options
1. reader 指定reader模块
2. pkt_filter 指定packet fliter模块
3. frame_filter 指定frame filter 模块
4. audio_decoder 指定audio decoder 模块
5. video_decoder 指定video decoder 模块
6. sub_decoder 指定subtext decoder 模块
7. audio_render 指定audio render 模块
8. video_render 指定video render 模块
