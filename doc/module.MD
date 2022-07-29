# Module
LVP 由core/reader/filter/decoder/render 五大模块构成，除了core模块外，其他模块基本都支持自定义。

LVP 的module 有两个公共接口 init 和 close 

init 用于初始化模块，并且会传入 option、event_control 和 log，供模块使用
 
 close用于关闭模块以及释放私有对象内存。
 
 一个典型的lvp module 应该如下,module必须实现init 和 close，module可以自由监听[事件](./event.MD)和抛出事件
 
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


## Reader module