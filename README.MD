# LVP(Lite video player)

## About
LVP 是一个基于[FFmpeg]("http://ffmpeg.org/")的播放器框架


## Feture
1. 跨平台， LVP目前支持Windows，Linux，MacOS,理论上说LVP支持任何可以编译过FFmpeg的平台 
2. 低开销，LVP在支持[硬件加速](./doc/hwaccel.MD)的平台上开销极低，可以和VLC媲美。
3. 易定制，LVP使用[模块](./doc/module.MD)设计,其中packet filter 和 frame filter可以很方便的拿到解码前后的数据

## Render
Render 可能是您最需要关心的一个模块,播放器的声音播放和画面渲染都在此模块完成

LVP 使用[SDL2](https://www.libsdl.org/)作为默认的Render module,大多数时候你需要自己实现一个Render来渲染解码后的图像和声音。

LVP 还为[Qt](https://qt.io)、[Unity3D](https://unity3d.com)、[Electron](https://electronjs.org)提供了默认的Render模块

## Build
1. 首先你需要FFmpeg的库，默认应该放在./deps/ffmpeg/windows下,当然你也可以通过更改CMakeLists.tx来修改库地址
2. SDL2库也应该被同时添加到./deps/SDL2/windows下
3. build
```shell
#mkdir build
#cd build
#cmake ..
#make
```

## Use
```c 
#include "lvp_core.h"
#include "lvp.h"


LVPCore* core = lvp_core_alloc();
lvp_core_set_option(core, "some options");
lvp_core_set_url(core, "input_url");
lvp_core_play(core);

```

# Data flow
```c
 -------               --------             -------
|       |    reader   |  pkt   |           |  pkt  |
| input |------------>| filter |---------->| cache |
|       |             |        |           |       |
 -------               --------             -------
                                               |
                                               |decoder
                                               |
                                               v
 --------            --------             --------
|        |          | frame  |           | frame  |   
| render |<---------| cache  |<----------| filter |
|        |          |        |           |        |
 --------             -------            --------
```

## License
LGPL license.
