# 硬件加速
LVP 可以在支持硬件加速的机器上使用硬件加速解码，其播放开销基本和VLC相同。

LVP 支持的硬件加速，QSV（intel 平台）、CUDA（nvida平台）、DXVA2（windows 平台）、VideoToolbox(MacOS)

使用硬件加速可能导致解码失败，其错误可能由目标用户驱动不是最新导致。如果你想让播放器有更大的兼容性,推荐windows使用DXVA2硬件加速，mac使用VideoToolbox