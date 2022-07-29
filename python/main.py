from ctypes import *

dll = CDLL("/home/fgodt/worke/LiteVideoPlayer/build/liblvp.so")

if dll == None :
    print("can not open dll")

core = dll.lvp_core_alloc()
if core == None:
    print("alloc core error")
url = c_char_p(bytes("/home/fgodt/gd.mkv","utf-8"))
ret = dll.lvp_core_set_url(core,url)
if ret != 0 :
    print("set url error")

ret = dll.lvp_core_play(core)
if ret != 0 :
    print("play error")

v = input()