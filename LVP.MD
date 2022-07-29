## LVP 概要


对外接口 

创建一个lvp播放器                                                                  
```
lvp_alloc()
```


设置播放地址

```
lvp_set_url() ----> lvp_core_set_url
```


设置参数
```
lvp_set_options() ----> lvp_core_parse_options
```


开始播放
```
lvp_play() ----> lvp_core_play 
```


暂停
```
lvp_pause() ----> lvp_core_pause
```


seek
```
lvp_seek() ----> lvp_core_seek
```


停止
```
lvp_stop() ----> lvp_core_stop
```


销毁播放器
```
lvp_free() ----> lvp_core_free
```


## LVP 生命周期
```code
lvp_alloc()                                
    |                                      
    |-->lvp_core_alloc                     

lvp_set_options()                          
    |                                      
    |-->lvp_core_parse_options             

lvp_set_url()                              
    |                                      
    |-->lvp_core_set_url                   

lvp_play()                                 
    |                                      
    |                              |---> reader init
    |                              |
    |                              |---> pkt/frame filter init
    |                              |
    |--> create/init core module   |---> decoder init
                |                  |
                |----------------->|---> pkt/frame cache init
                                   |
                                   |---> render init
                                   |
                                   |---> audio video sync init
                                   |
                                   |---> other module

lvp_seek()
    |
    |--> lvp_core_seek

lvp_pause()
    |
    |--> lvp_core_pause

lvp_stop()
    |
    |--> lvp_core_stop
            |
            |--> close module
```

## LVP 数据流向

```
reader--->reader_module--->packet_filter--->packet_cache--->decoder--->decoder_module--->frame_filter--->frame_cache--->render
```

## LVP MOUDLE

LVP 的module 有两个公共接口 init 和 close 

init 用于初始化模块，并且会传入 option、event_control 和 log，供模块使用

close用于关闭模块以及释放私有对象内存。

一个典型的lvp module 应该如下,module必须实现init 和 close，module可以自由监听事件和抛出事件

```
              func  |--> init()
            |------>|
            |       |--> close()
module----->|
            |handlers 
            |------>
            |events
            |------>
```
handlers: 一个module 可以监听和处理不同的event，lvp把这些归于handlers

events: module自身可能会抛出的事件


## 默认module的事件和handle

### lvp reader module 

```
                    |--->LVP_EVENT_SET_URL
                    |
                    |--->LVP_EVENT_PLAY
             handle |
            |------>|--->LVP_EVENT_SEEK
            |       |
            |       |--->LVP_EVENT_STOP
            |       |
            |       |--->LVP_EVENT_CHANGE_STREAM
            |
reader----->|
            |       |--->LVP_EVENT_REAER_EOF
            |       |
            |       |--->LVP_EVENT_READER_ERROR
            |------>|
              event |--->LVP_EVENT_SELECT_STREAM
                    |
                    |--->LVP_EVENT_READER_GOT_FRAME
                    |
                    |--->LVP_EVENT_READER_SEND_FRAME
```

### lvp packet filter module
```
             handle 
            |------>|--->LVP_EVENT_READER_SEND_FRAME
            |       
filter----->|       
            |       
            | event      
            |------>|--->LVP_EVENT_PACKET_FILTERD
```

### lvp cache module
```
             handle |
            |------>|--->LVP_EVENT_PACKET_FILTERD
            |       |
            |       |--->LVP_EVENT_SEEK
            |       
cache------>|
            |       |--->LVP_EVENT_CACHE_POP
            | event |
            |------>|--->LVP_EVENT_CACHE_EMPTY
                    |
                    |--->LVP_EVENT_CACHE_FULL
```

### lvp decoder module
```

             handle |--->LVP_EVENT_CACHE_POP
            |------>|
            |       |--->LVP_EVENT_SELECT_STREAM
            |       
decoder---->|
            |       |--->LVP_EVENT_DECODER_GOT_FRAME
            | event |
            |------>|--->LVP_EVENT_DECODER_OPEN
                    |
                    |--->LVP_EVENT_DECODER_ERROR
```

### lvp avsync module
```
                    |--->LVP_EVENT_CACHE_POP
             handle |
            |------>|
            |       |
            |       |--->LVP_EVENT_SEEK
            |       
avsync----->|       |--->LVP_EVENT_UPDATE_AUDIO
            | event |
            |------>|
                    |
                    |--->LVP_EVENT_UPDATE_VIDEO
```
### lvp reander module
```
             handle |--->LVP_EVENT_UPDATE_AUDIO
            |------>|
            |       |--->LVP_EVENT_UPDATE_VIDEO
            |       
a/vrender-->|
            |       
            | event |--->LVP_EVENT_RENDER_OPEN
            |------>|
                    |--->LVP_EVENT_RENDER_CLOSE
```