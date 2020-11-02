# LwIP 开发QEMU仿真环境搭建

## 一、QEMU环境搭建
嵌入式软件开发离不开开发板，在没有物理开发板的情况下，可以使用 QEMU 等类似的虚拟机来模拟开发板。QEMU 是一个支持跨平台虚拟化的虚拟机，它可以虚拟很多开发板。

如果我们手边没有开发板，可以在个人电脑上运行QEMU学习操作系统及互联网协议栈的开发调试。比如，笔者最开始学习操作系统，使用的是《30天自制操作系统》这本书，按照书中所示，从零开始一步步构建起一个简单可运行的操作系统，每实现一个功能，都是运行在QEMU虚拟机中观察效果的，实现过程源码：[https://github.com/StreamAI/30dayMakeOS](https://github.com/StreamAI/30dayMakeOS)。

选择学习TCP/IP互联网协议栈也可以借助QEMU虚拟机，在个人电脑上虚拟出一个开发板，以了解LwIP协议栈的实现原理及开发调试过程。为了方便大家在没有开发板的情况下体验 RT-Thread，RT-Thread 提供了 QEMU 模拟的 ARM vexpress A9 开发板的板级支持包 (BSP)， 我们可以在此基础上学习LwIP协议栈的开发与调试。

需要下载、安装的软件包如下：

 - QEMU开发环境：[https://github.com/StreamAI/LwIP_Projects](https://github.com/StreamAI/LwIP_Projects)
 - ENV开发辅助工具：[https://github.com/RT-Thread/env/tree/1.1.2](https://github.com/RT-Thread/env/tree/1.1.2)
 - Git工具：[https://www.git-scm.com/download/](https://www.git-scm.com/download/)
 - Tab网卡：[https://github.com/StreamAI/LwIP_Projects](https://github.com/StreamAI/LwIP_Projects)
 - Visual Studio Code：[https://code.visualstudio.com/download](https://code.visualstudio.com/download)
 - Wireshark:[https://www.wireshark.org/](https://www.wireshark.org/)

QEMU工具是从[RT-Thread源代码](https://github.com/RT-Thread/rt-thread/tree/v4.0.1)中复制（在rt-thread-4.0.1\bsp\qemu-vexpress-a9目录下启动env环境执行scons --dist命令获得）出来的。针对QEMU环境配置也可以参考RT-Thread文档：[QEMU环境搭建](https://www.rt-thread.org/document/site/tutorial/qemu-network/qemu_setup/qemu_setup/)。

目录.\LwIP_Projects\qemu-vexpress-a9下的文件结构如下图所示：
![qemu-vexpress-a9目录结构](https://img-blog.csdnimg.cn/201911212212192.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
在.\LwIP_Projects\qemu-vexpress-a9目录下启动env环境（不熟悉env环境的可参考：[env视频教程](https://www.rt-thread.org/document/site/tutorial/env-video/)与[env用户手册](https://www.rt-thread.org/document/site/programming-manual/env/env/)），在env环境中执行scons命令，待编译完成后，再次执行qemu命令，会启动QEMU虚拟开发板界面，如下图所示：
![编译qemu工程并启动qemu](https://img-blog.csdnimg.cn/2019112122210342.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
![QEMU虚拟环境运行界面](https://img-blog.csdnimg.cn/2019112122224078.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)

 - **配置TAP网卡**

如果想连通网络，还需要安装并配置TAP网卡，tap-windows-9.21.2安装成功后会在电脑”网络连接“里面出现一个新的网络适配器”TAP-Windows Adapter V9“，将其重命名为tap。

右键选择当前上网的网络连接（本文使用WI-FI），打开属性-->共享，选择家庭网络连接为 tap，点击确定完成设置，如下图所示：
![配置tap网卡并共享网络](https://img-blog.csdnimg.cn/20191121223519332.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
将安装并配置好的TAP网卡添加配置到qemu-vexpress-a9工程qemu.bat脚本文件中，添加配置项”-net nic -net tap,ifname=tap“，添加位置如下图所示（用VSCode打开的qemu-vexpress-a9工程qemu.bat脚本文件）：
![qemu.bat添加tap网卡配置](https://img-blog.csdnimg.cn/2019112122474191.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
配置完tap网络适配器后，重新在env环境中执行qemu命令，待QEMU虚拟开发板界面打开后，执行"ifconfig"命令查看网卡信息，执行"ping www.baidu.com"命令测试网络连通性，命令执行界面如下：
![QEMU查看网卡信息并测试网络连通性](https://img-blog.csdnimg.cn/20191121225215921.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
到这里QEMU虚拟开发板的LwIP环境就搭建好了，从上图可以看出QEMU默认配置的LwIP-2.0.2版本，如果想更改为其他版本，可以在env环境中执行"menuconfig"命令，启动RT-Thread Project Configuration界面，选择RT-Thread Components --> Network --> light weight TCP/IP stack --> lwIP version进入版本选择界面：
![选择lwip版本](https://img-blog.csdnimg.cn/20191122001530527.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
选中要使用的LwIP版本（比如LwIP v2.1.0，旧版LwIP v1.4.1编译后网络连接有问题，我们使用最新版学习）后按space空格键选中，保存配置并推出menuconfig，配置界面如下：
![配置LwIP-V2.1.0](https://img-blog.csdnimg.cn/20191122002050274.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
在env环境中运行scons重新编译qemu-vexpress-a9工程，再次运行qemu命令启动QEMU虚拟开发板，可以看到初始化LwIP-2.1.0完成了，接着执行"ifconfig"命令，如果看到IP地址都显示0，等几秒钟后再次执行"ifconfig"命令即可获得网卡地址，界面如下：
![选择lwip-v2.1网络连通](https://img-blog.csdnimg.cn/20191122002745633.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
如果执行"ifconfig"命令始终无法获得IP地址，或者执行"ping www.baidu.com"超时不可达，则把共享网卡功能（这里是WIFI无线网络连接）关闭后再重新打开，即可解决该问题。

## 二、使用VS Code调试QEMU
VS Code（全称 Visual Studio Code）是一个轻量且强大的代码编辑器，支持 Windows，OS X 和 Linux。内置 JavaScript、TypeScript 和 Node.js 支持，而且拥有丰富的插件生态系统，可通过安装插件来支持 C++、C#、Python、PHP 等其他语言。

若想了解更多VS Code在工程管理中的应用，可参考博客：[VSCode+GCC+Makefile+GitHub项目管理](https://blog.csdn.net/m0_37621078/article/details/88320010)，前面用到的scons编译命令实际上类似于Makefile，scons使用python语法配置比Makefile更简单些。运行scons命令，实际默认使用的编译工具也是GCC。

在 Env 控制台进入 qemu-vexpress-a9 BSP 根目录，然后输入命令 code . (code 是命令，点 '.' 是参数表示当前目录，中间由空格隔开)打开 VS Code，表示使用 VS Code 打开当前目录。开始调试前需要编辑 qemu-vexpress-a9 目录下的 qemu-dbg.bat 文件，在 qemu-system-arm 前加入 start ，配置界面如下图所示（VS Code需要安装C/C++插件）：
![qemu调试配置](https://img-blog.csdnimg.cn/20191122004917160.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
选择调试选项”Debug @windows“ ，在main函数中插入断点，按F5键开始调试：
![选择调试选项](https://img-blog.csdnimg.cn/20191122005213300.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
按F5按键后，QEMU运行起来了，在 VS Code 里可以使用 GDB 命令，需要在最前面加上 -exec。 例如 -exec info registers 命令可以查看寄存器的内容：
![QEMU调试界面](https://img-blog.csdnimg.cn/20191122005807186.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
## 三、WireShark网络抓包分析
在网络编程的过程中，经常需要利用抓包工具对开发板发出或接收到的数据包进行抓包分析。wireshark 是一个非常好用的抓包工具，使用 wireshark 工具抓包分析，是学习网络编程必不可少的一项技能。

安装 wireshark，一路默认安装就行，在进行到Npcap安装时，勾选如下图所示的两个选项：
![Npcap安装配置](https://img-blog.csdnimg.cn/2019112223444673.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
打开 wireshark 之后，会给出你的网卡信息，让你选择一个要抓包的网卡如下图所示，选择自己开发板用来上网的网卡，双击就开始抓包了（不熟悉wireshark使用的可参考博客：[wireshark使用教程](https://www.jianshu.com/p/55ec409c739e)）。
![wireshark选择tap网卡](https://img-blog.csdnimg.cn/20191123000350328.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
wireshark开始抓取tap网络数据包后，启动qemu-vexpress-a9虚拟开发板，并执行“ping www.baidu.com”命令，设置过滤icmp报文，抓取到的其中一个数据包信息如下：
![wireshark抓取icmp数据包](https://img-blog.csdnimg.cn/20191123002904238.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
使用 wireshark 抓包工具，在进行网络开发与调试的时候就会事半功倍了。




# 更多文章：

 - 《[IOT-OS之RT-Thread（一）--- 系统启动与初始化过程](https://blog.csdn.net/m0_37621078/article/details/100584591)》
 - 《[TCP/IP协议栈之LwIP（一）---网络数据包管理](<https://blog.csdn.net/m0_37621078/article/details/93927737>)》
 - 《[TCP/IP协议栈之LwIP（二）---网络接口管理](<https://blog.csdn.net/m0_37621078/article/details/94494848>)》
 - 《[TCP/IP协议栈之LwIP（三）---网际寻址与路由](<https://blog.csdn.net/m0_37621078/article/details/94646591>)》
 - 《[TCP/IP协议栈之LwIP（四）---网络诊断与状态查询](<https://blog.csdn.net/m0_37621078/article/details/96450108>)》
 - 《[TCP/IP协议栈之LwIP（五）---网络传输管理之UDP协议](<https://blog.csdn.net/m0_37621078/article/details/96889896>)》
 - 《[TCP/IP协议栈之LwIP（六）---网络传输管理之TCP协议](<https://blog.csdn.net/m0_37621078/article/details/97439952>)》
 - 《[TCP/IP协议栈之LwIP（七）---内核定时事件管理](<https://blog.csdn.net/m0_37621078/article/details/97978998>)》
 - 《[TCP/IP协议栈之LwIP（八）---Raw/Callbck API编程](<https://blog.csdn.net/m0_37621078/article/details/97841362>)》
 - 《[TCP/IP协议栈之LwIP（九）---Sequetia API编程](<https://blog.csdn.net/m0_37621078/article/details/98465308>)》
 - 《[TCP/IP协议栈之LwIP（十）---Socket API编程](<https://blog.csdn.net/m0_37621078/article/details/99053518>)》
 - 《[TCP/IP协议栈之LwIP（十一）--- LwIP协议栈移植](<https://blog.csdn.net/m0_37621078/article/details/103282134>)》
 - [《Web技术（一）：互联网的设计与演化(URL + HTML + HTTP)》](https://blog.csdn.net/m0_37621078/article/details/105543208)

