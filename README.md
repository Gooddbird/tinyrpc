# TinyRPC
作者：**ikerli**  **2022-05-13**
## 1. 概述
**TinyRPC** 是一款使用 C++11 开发的小型 **RPC** 框架。**TinyRPC** 是本人在学习过程的作品，这些代码没有达到工业强度，也可能存在一些未知 BUG，甚至 coredump。读者请自行辨别，谨慎使用！

麻雀虽小五脏俱全，从命名上就能看出来，TinyRPC 框架主要用义是为了让读者能**快速地**、**轻量化**地搭建出具有一定性能的 RPC 服务。我不敢说有多高的性能，但至少能应付目前大多数场景了。

**TinyRPC** 没有实现跨平台，只支持 LINUX 系统，并且必须是 64 位的系统，因为协程切换只实现了 **64** 位系统的，而没有兼容 **32** 位系统。

**TinyRPC** 的自定义的 RPC 协议报文暂时只支持基于 **Protobuf** 的序列化。

综上，**TinyRPC** 有一些优点，当然也有缺点。其中有些不足的地方，要么是我不懂得如何更好地实现，要么是我懒。。

## 2. 前置知识
要正确编译 **TinyRPC**, 至少要先安装这两个库：

### 2.1 protobuf
**protobuf** 是 **google** 开源的有名的序列化库。谷歌出品，必属精品！**TinyRPC** 的 RPC 是基于 protobuf 来 序列化/反序列化 的，因此这个库是必须的。
其地址为：https://github.com/protocolbuffers/protobuf

### 2.2 tinyxml
由于 **TinyRPC** 读取配置使用了 xml 文件，因此需要安装 **tinyxml** 库来解析配置文件。
其地址为: https://github.com/leethomason/tinyxml2

注意，以上两个库的头文件我都放在了 **/usr/include** 下， 库文件放在了 **/usr/lib** 下。因此在 [makefile](./makefile) 中并没有指定其头文件和库文件路径，因为其被安装在了系统默认搜索路径中，无需特殊指定。


## 3. 如何编译
在安装了前置的几个库之后，就可以开始编译 **TinyRPC** 了。编译过程：
```
1. git clone https://github.com/Gooddbird/tinyrpc
2. cd tinyrpc
3. mkdir bin & mkdir lib;
4. make
```
make 完成后，会在 lib 目录下生成静态库文件 **libtinyrpc.a**。在真正开发 RPC 服务时，只需要**静态链接**这个库，就能使用其中的函数了。(当然也能**动态链接**, 读者可自行修改 makefile 满足自己的需求)。
更多详细信息请参考这个仓库：

此外，还会在 bin 目录下生成一些单元测试文件。运行 bin/test_xxxx 文件，可以简单测试下 TinyRPC 的一些单元模块功能是否正常。

如果编译出现问题，欢迎提 [issue](https://github.com/Gooddbird/tinyrpc/issues/), 我会尽快回应。

testcase 提供了两个简单的 RPC 测试服务, **test_rpc_server1** 和 **test_rpc_server2**. 读者可自行运行测试。更多详细信息参考: [quick_stark](./quick_rpc_test.md).

## 4. RPC 服务搭建实例

请移步项目：[TinyRPCExamples](https://github.com/Gooddbird/TinyRPCExamples)

## 5. 模块说明
TinyRPC 框架的主要模块包括：异步日志、协程封装、Reactor封装、Tcp 封装、TinyPb、以及RPC封装模块等。

### 5.1 异步日志模块
设计初期，TinyRPC 的日志主要参考了 (sylar)[https://github.com/protocolbuffers/protobuf]，并精简后实现了最基础的打印日志。

在开发到一定程度后，发现同步日志或多或少有些影响性能，遂改为异步日志。TinyRPC 的异步日志实现非常简单，只是额外建立了一个线程来负责打印日志罢了。

当然，TinyRPC 的日志做到了了以下几点：
- **异步**：日志异步打印，不阻塞当前线程。
- **日志级别**：日志分级别打印，当设定级别高于待打印日志的级别时，日志打印是个空操作，无性能消耗。
- **文件输出**：日志支持可以输出到文件中，特别是在生存环境上，把日志打印到控制台可不是一个好方法。
- **滚动日志**：日志文件会自行滚动，当**跨天**或者**单个文件超过一定大小**后，会建立新的文件写入日志信息。

### 5.2 协程模块
TinyRPC 的协程底层使用了腾讯的开源协程库 [libco](https://github.com/Tencent/libco)，即协程上下文切换那一块。而协程切换的原理不过是寄存器切换罢了。
除了协程切换之外，TinyRPC 提供了一些基本函数的 hook，如 read、write、connect 等函数。

更多协程的介绍请移步我的知乎文章：
[C++实现的协程网络库tinyrpc（一）-- 协程封装](https://zhuanlan.zhihu.com/p/466349082)
[协程篇（一）-- 函数调用栈](https://zhuanlan.zhihu.com/p/462968883)

### 5.3 Reactor 模块
--建设中--
可移步知乎文章：
[Reactor模式介绍](https://zhuanlan.zhihu.com/p/428693405)

### 5.4 Tcp 模块
--建设中，敬请期待--

### 5.5 TinyPb 协议
--建设中，敬请期待--
协议更多细节见 [TinyPb协议详解](./tinypb_protocal.md).

### 5.6 RPC 封装
--建设中，敬请期待--




## 关于作者
**ikerli**
后台开发新人，闲来没事写点技术文章分享。懂的不多，但我分享的东西一定是我懂的，因为不懂的我一句也不会提。请放心食用！

欢迎关注我的**知乎**账号：知乎搜索 **ikerli**

欢迎关注我的**个人微信**公众号(无广告，放心食用):
![](./code.jpg)



## 参考资料
libco: https://github.com/Tencent/libco
sylar: https://github.com/sylar-yin/sylar
muduo: https://github.com/chenshuo/muduo
tinyxml: https://github.com/leethomason/tinyxml2
protobuf: https://github.com/protocolbuffers/protobuf


