# D1 rt-thread

本工程是关于全志D1&&全志D1s(f133)的一份移植，系统运行在M-mode。

## 编译

首先需要下载交叉编译工具链，关于平头哥开源工具链的编译，可以参考下面的文章：

```
https://my.oschina.net/u/4239621/blog/5368281
```

其中下载已经编译好的编译器:

```
链接：https://pan.baidu.com/s/1pu61DYlg5pkZd0xdYALi5Q 
提取码：wrsw 
```

在Linux上编译环境可以采用`newlib.tar.gz`。

关于window工具链，需要下载`riscv64-elf-mingw-20210618.tar.gz`。

下载完成后解压，到指定的目录即可。

## 配置RT-Thread的编译

在Linux环境中只需要修改bsp/d1-nezha下面的rtconfig.py文件即可

```
if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'/xxx/thead/thead_newlib_gcc/bin'
```

将`EXEC_PATH`替换成自己的编译工具链即可。



在windows环境中，可以下载env工具

```
https://www.rt-thread.org/page/download.html
```

下载env工具后，设置gcc路径。

```
 set RTT_EXEC_PATH=C:\work\work\d1\riscv64-elf-mingw\bin
```

后面的路径根据自己的gcc路径进行修改。

进入`d1-nezha-rtthread\bsp\d1-nezha`路径

输入

```
scons -c
scons
```

就能够正常编译了。


## 固件下载

可以采用fel.exe工具

```
https://github.com/bigmagic123/d1-nezha-baremeta/blob/main/docs/3.d1%E4%B8%8B%E8%BD%BDxboot%E7%9A%84%E8%BF%87%E7%A8%8B.md
```

参考上面的文章

```
xfel.exe ddr ddr3
xfel.exe write 0x40000000 rtthread.bin
xfel.exe exec 0x40000000
```

对于D1s(f133)，其DDR2对应的命令如下：

```
xfel.exe ddr ddr2
xfel.exe write 0x40000000 rtthread.bin
xfel.exe exec 0x40000000
```
## 程序运行与测试

正常情况下，可以看到串口打印如下的信息:

```
heap: [0x4004bb1c - 0x4324bb1c]

 \ | /
- RT -     Thread Operating System
 / | \     4.1.0 build Dec 19 2021 18:26:36
 2006 - 2021 Copyright by rt-thread team
file system initialization done!
Hello RISC-V!
msh />
```