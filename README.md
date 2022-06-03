# TinyRPC
作者：**ikerli**  **2022-05-13**
## 1. 概述
**TinyRPC** 是一款使用 C++11 开发的小型**异步 RPC** 框架。**TinyRPC** 是本人在学习过程的作品，这些代码没有达到工业强度，最好不要直接用到生产环境，也可能存在一些未知 BUG，甚至 coredump。读者请自行辨别，谨慎使用！

麻雀虽小五脏俱全，从命名上就能看出来，TinyRPC 框架主要用义是为了让读者能**快速地**、**轻量化**地搭建出具有较高性能的 异步RPC 服务。我不敢说有多高的性能，但至少能应付目前大多数场景了。

**TinyRPC** 的特点是：**用同步的代码，实现异步的性能。** 也就是说，**TinyRPC** 在 RPC 调用时候不需要像其他异步操作一样需要写复杂的回调函数，只需要直接调用即可。这看上去是同步的过程，实际上由于内部的协程封装实现了完全的异步。

**TinyRPC** 没有实现跨平台，只支持 Linux 系统，并且必须是 64 位的系统，因为协程切换只实现了 **64** 位系统的，而没有兼容 **32** 位系统。

**TinyRPC** 框架目前支持两类协议：
1. 纯 **HTTP** 协议: TinyRPC 实现了简单的很基本的 HTTP(1.1) 协议的编、解码，完全可以使用 HTTP 协议搭建一个 RPC 服务。
2. TinyPB 协议: 一种基于 **Protobuf** 的自定义协议，属于二进制协议。更多内容参考： [TinyPB协议详解](./tinypb_protocal.md)

综上，**TinyRPC** 有一些优点，当然也有缺点。其中有些不足的地方，要么是我不懂得如何更好地实现，要么是我懒。。

## 2. 安装 TinyRPC
### 2.1 必要的依赖库
要正确编译 **TinyRPC**, 至少要先安装这几个库：

### 2.1.1 protobuf
**protobuf** 是 **google** 开源的有名的序列化库。谷歌出品，必属精品！**TinyRPC** 的 **TinyPB** 协议是基于 protobuf 来 序列化/反序列化 的，因此这个库是必须的。
其地址为：https://github.com/protocolbuffers/protobuf
安装过程不再赘述。

### 2.1.2 tinyxml
由于 **TinyRPC** 读取配置使用了 xml 文件，因此需要安装 **tinyxml** 库来解析配置文件。
其地址为: https://github.com/leethomason/tinyxml2

### 2.1.3 libmysqlclient
需要安装 **MySQL** 的 glibc 库，用于 MySQL 操作, 选择所需的版本安装即可(建议 5.7 以上)

官方下载地址：https://downloads.mysql.com/archives/community/

此外，安装 libmysqlclient 只是保证编译通过，但运行时如果需要连接 MYSQL 数据库，还需要在对方机器上安装 MYSQL 服务并启动才行。

注意，以上几个库的头文件我都放在了 **/usr/include** 下， 库文件放在了 **/usr/lib** 下。因此在 [makefile](./makefile) 中并没有指定其头文件和库文件路径，因为其被安装在了系统默认搜索路径中，无需特殊指定。


## 2.2 安装和卸载

### 2.2.1 安装 TinyRPC
在安装了前置的几个库之后，就可以开始编译和安装 **TinyRPC** 了。安装过程十分简单，只要不出什么意外就好了。

**祈祷**一下一次性成功，然后直接执行以下几个命令即可：
```
git clone https://github.com/Gooddbird/tinyrpc

cd tinyrpc
mkdir bin && mkdir lib

// 先执行编译
make -j4

// 编译成功后直接安装就行了
make install
```

注意, make install 完成后，默认会在 **/usr/lib** 路径下安装 libtinyrpc.a 静态库文件，以及在 **/usr/include/tinyrpc** 下安装所有的头文件。

如果编译出现问题，欢迎提 [issue](https://github.com/Gooddbird/tinyrpc/issues/), 我会尽快回应。

### 2.2.2 卸载 TinyRPC
卸载也很简单，如下即可：
```
make uninstall
```
**注：如果此前已经安装过 TinyRPC, 建议先执行卸载命令后再重新 make install 安装.**


## 3. 使用 TinyRPC 快速搭建服务

### 3.1 快速上手
在 make 成功之后，出了生成静态库文件。此外，还会在 bin 目录下生成一些单元测试文件。TinyRPC 提供了一个简单地 RPC 服务调用示例，更多内容请参考文档：[quick_stark](./quick_rpc_test.md).


### 3.2 标准示例

有一说一，使用 TinyRPC 框架搭建一个 RPC 服务还是比较简单地，核心代码几十行就能搞定。在这提供了几个 TinyRPC 搭建 RPC 服务的简单工程实例，这个工程的架构还是相对比较规范的。

更多内容请移步项目(建设中)：[TinyRPCExamples](https://github.com/Gooddbird/TinyRPCExamples)


## 4. 模块设计
TinyRPC 框架的主要模块包括：异步日志、协程封装、Reactor封装、Tcp 封装、TinyPb协议封装、HTTP 协议封装、以及RPC封装模块等。

### 4.1 异步日志模块
设计初期，**TinyRPC** 的日志主要参考了 (**sylar**),并精简后实现了最基础的打印日志。

在开发到一定程度后，发现同步日志或多或少有些影响性能，遂改为异步日志。TinyRPC 的异步日志实现非常简单，只是额外建立了一个线程来负责打印日志罢了。

当然，**TinyRPC** 的日志做到了了以下几点：
- **异步**：日志异步打印，不阻塞当前线程。生产者只需要将日志信息放入buffer即可，消费者线程会按照一定时间频率自动将日志同步到磁盘文件中。
- **日志级别**：日志分级别打印，**当设定级别高于待打印日志的级别时，日志打印是个空操作**，无性能消耗。
- **文件输出**：日志支持可以输出到文件中，特别是在生存环境上，把日志打印到控制台可不是一个好方法。
- **滚动日志**：日志文件会自行滚动，当**跨天**或者**单个文件超过一定大小**后，会建立新的文件写入日志信息。
- **崩溃处理**：TinyRPC 的日志库处理了**程序突然崩溃**的情况，简单来说就是当程序崩溃退出前先将日志信息同步到磁盘文件上。这是非常重要的，如果缺失了崩溃那一瞬间的日志内容，那就很难排查具体原因。

### 4.2 协程模块
TinyRPC 的协程底层使用了腾讯的开源协程库 [libco](https://github.com/Tencent/libco)，即协程上下文切换那一块。而协程切换的原理不过是寄存器切换罢了。
除了协程切换之外，TinyRPC 提供了一些基本函数的 hook，如 read、write、connect 等函数。

更多协程的介绍请移步我的知乎文章：

[C++实现的协程网络库tinyrpc（一）-- 协程封装](https://zhuanlan.zhihu.com/p/466349082)

[C++实现的协程网络库tinyrpc（二）-- 协程Hook](https://zhuanlan.zhihu.com/p/474353906)

[协程篇（一）-- 函数调用栈](https://zhuanlan.zhihu.com/p/462968883)


### 4.3 Reactor 模块
--建设中--
可移步知乎文章：

[C++实现的协程网络库tinyrpc（四）-- Reactor 实现](https://zhuanlan.zhihu.com/p/503323714)

[Reactor模式介绍](https://zhuanlan.zhihu.com/p/428693405)

### 4.4 Tcp 模块
--建设中，敬请期待--

### 4.5 TinyPb 协议
--建设中，敬请期待--
协议更多细节见 [TinyPb协议详解](./tinypb_protocal.md).

### 4.6 Http 模块
--建设中，敬请期待--

### 4.7 RPC 调用封装
--建设中，敬请期待--




## 关于作者
**ikerli**
后台开发新人，闲来没事写点技术文章分享。懂的不多，但我分享的东西一定是我懂的，因为不懂的我一句也不会提。请放心食用！

联系我：**1753009868@qq.com**

欢迎关注我的**知乎**账号：知乎搜索 **ikerli**

欢迎关注我的**个人微信**公众号, **微信公众号** 搜索 **ikerli**(无广告，放心食用):
![](./code.jpg)



## 参考资料
libco: https://github.com/Tencent/libco

sylar: https://github.com/sylar-yin/sylar

muduo: https://github.com/chenshuo/muduo

tinyxml: https://github.com/leethomason/tinyxml2

protobuf: https://github.com/protocolbuffers/protobuf


