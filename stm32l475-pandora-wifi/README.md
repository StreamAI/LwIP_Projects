# ESP8266 WiFi模块移植

## 一、AT命令集简介
AT 命令（Attention Commands）最早是由发明拨号调制解调器（MODEM）的贺氏公司（Hayes）为了控制 MODEM 而发明的控制协议。后来随着网络带宽的升级，速度很低的拨号 MODEM（比如ADSL） 基本退出一般使用市场，但是 AT 命令保留下来。当时主要的移动电话生产厂家共同为 GSM 研制了一整套 AT 命令，用于控制手机的 GSM 模块（包括对SMS短消息服务的控制），AT 命令在此基础上演化并加入 GSM 07.05 标准以及后来的 GSM 07.07 标准，实现比较健全的标准化。

在随后的 GPRS 控制、3G 模块等方面，均采用的 AT 命令来控制，AT 命令逐渐在产品开发中成为实际的标准。如今，AT 命令也广泛的应用于嵌入式开发领域，AT 命令作为主芯片和通讯模块的协议接口，硬件接口一般为串口，这样主控设备可以通过简单的命令和硬件设计完成多种操作。

### 1.1 AT命令集简介
AT 命令集是一种应用于 AT 服务器（AT Server）与 AT 客户端（AT Client）间的设备连接与数据通信的方式。 其基本结构如下图所示：
![AT服务器与客户端通信方式](https://img-blog.csdnimg.cn/20200322165833525.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)

 一般 AT 命令由前缀、主体、结束符三个部分组成：**前缀**由字符 AT构成；**主体**由命令、参数和可能用到的数据组成；**结束符**一般为<CR><LF> (也即"\r\n")，其中<LF>是串口终端的要求（使用串口工具发送命令时记得勾选新行模式）。每个命令一般可分为下表所示的四种命令类型（不是每条指令都具备四种类型）：
![指令结构描述](https://img-blog.csdnimg.cn/20200323175846651.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
AT命令发出一般都有响应，或响应请求的数据，或响应命令执行的状态，响应数据前后都有一个<CR>字符，下面给出几个基础AT命令（每条命令支持上表的几种类型）的格式、响应、参数说明与示例（其中<CR>、<LF>这类不可见字符省略了）：
![AT基础命令示例](https://img-blog.csdnimg.cn/20200323184656867.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
 AT 功能的实现需要 AT Server 和 AT Client 两个部分共同完成：**AT Server** 主要用于接收 AT Client 发送的命令请求，判断接收的命令及参数格式，并下发对应的响应数据，或者主动下发URC数据(Unsolicited Result Code，常在AT Server出现比如 WIFI 连接断开、TCP 接收数据等特殊情况时，通知AT Client做出相应的操作)；**AT Client** 主要用于发送命令、等待 AT Server 响应，并对 AT Server响应数据或主动发送的URC数据进行解析处理，获取相关信息。

AT Server 和 AT Client 之间支持多种数据通讯的方式（UART、SPI 等），目前最常用的是串口 UART 通讯方式（包括RS232、RS485等）。随着 AT 命令的逐渐普及，越来越多的嵌入式产品上使用了 AT 命令，AT 命令作为主芯片和通讯模块的协议接口，硬件接口一般为串口，这样主控设备可以通过简单的命令和硬件设计完成多种操作。

### 1.1 AT组件简介
虽然 AT 命令已经形成了一定的标准化，但是不同的芯片支持的 AT 命令并没有完全统一，这直接提高了用户使用的复杂性。对于 AT 命令的发送和接收以及数据的解析没有统一的处理方式，并且在使用 AT 设备连接网络时，只能通过命令完成简单的设备连接和数据收发功能，很难做到对上层网络应用接口的适配，不利于产品设备的开发。

为了方便用户使用 AT 命令，简单的适配不同的 AT 模块， RT-Thread 提供了 AT 组件用于 AT 设备的连接和数据通讯。AT 组件的实现也包括客户端的和服务器两部分，完成 AT 命令的发送、命令格式及参数判断、命令的响应、响应数据的接收、响应数据的解析、URC 数据处理等整个 AT 命令数据交互流程。

通过 AT 组件，设备可以作为 AT Client 使用串口连接其他设备发送并接收解析数据，可以作为 AT Server 让其他设备甚至电脑端连接完成发送数据的响应，也可以在本地 shell 启动 CLI 模式使设备同时支持 AT Server 和 AT Client 功能，该模式多用于设备开发调试。对于嵌入式设备而言，更多的情况下设备使用AT组件作为客户端连接服务器设备。

支持AT命令集的设备一般内部都集成了TCP/IP网络协议栈，我们只需要通过串口连接AT模块与主控芯片，就可以使用AT命令集控制AT模块实现我们需要的网络服务功能。类比前一篇博客介绍的[网络分层结构](https://blog.csdn.net/m0_37621078/article/details/104836942)，AT组件的协议栈架构并没有网络协议层，AT通讯模块的驱动协议框架如下图所示：
![AT组件网络框架](https://img-blog.csdnimg.cn/20200323100140286.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
 - **APP层**：开发者只需要使用标准的系统调用接口，比如BSD Socket API 开发应用即可，无需关心底层的实现，同时应用代码还拥有比较好的可移植性；
 - **SAL组件层**：对上层提供了BSD Socket API，对下层提供了协议簇注册接口；
 - **AT组件层**：对上层提供了基于AT的Socket接口，对下层提供了移植接口。AT device在初始化完成后会被作为一个AT Socket设备注册到SAL组件中，当上层应用调用BSD Socket API时，会通过注册的接口调用底层的AT device驱动，完成数据的传输；
 - **AT device层**：利用AT组件对AT device做的移植，主要是利用AT组件提供的接口完成对AT设备（比如ESP8266模块）的初始化工作。由于AT设备内部有MCU芯片运行射频芯片的驱动与协议栈代码（要想正常使用AT模块，需要确保里面烧录了正确的固件代码），我们在主控设备上只需要完成串口驱动的配置和AT命令集的发送解析既可以了。

## 二、 ESP8266 WiFi模块简介
本文使用的WiFI模块型号为ATK-ESP-01，实际上是封装的乐鑫的ESP8266芯片，该SOC 集成了Tensilica L106 超低功耗 32 位微型 MCU，带有 16 位精简模式，主频支持 80MHz 和 160MHz，支持 RTOS，集成 Wi-Fi MAC/BB/RF/PA/LNA，ESP8266芯片的体系框架图如下：
![ESP8266芯片模块框架图](https://img-blog.csdnimg.cn/20200323163147280.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
ESP8266 是一个完整且自成体系的 Wi-Fi 网络解决方案，能够独立运行，也可以作为从机搭载于其它主机 MCU 运行。ATK-ESP-01模块支持标准的IEEE802.11 b/g/n协议，完整的TCP/IP协议栈，该模块一般作为从机通过UART接口（默认波特率115200）连接其它主机MCU，通过AT命令集与主机MCU交互，为主机MCU提供WiFi扩展服务。ATK-ESP-01模块的引脚定义如下：
![ATK-ESP-01模块引脚定义](https://img-blog.csdnimg.cn/20200323170019546.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
WiFi模块要想正常工作，内部需要运行WiFi固件，负责 IEEE802.11数据帧与IEEE 802.3数据帧之间的转换，并驱动RF基带管控无线数据链路层。ATK-ESP-01模块还内置了完整的LwIP协议栈，协议栈代码也需要随WiFi固件驱动一起烧录到模块内，该模块才能正常响应AT命令集。我们买来的ATK-ESP-01模块事先已经烧录好了WiFi固件（包含TCP/IP协议栈），如果用户意外擦除了，可以使用其提供的工具ESPFlashDownloadTool按照说明文档将模块固件（两个bin文件，分别保存主程序和射频参数）烧录到ATK-ESP-01模块内即可。

## 三、AT device层与AT组件层
ATK-ESP-01模块使用UART总线与主机MCU连接，直接插在Pandora开发板的ATK MODULE接口即可，如下图所示：
![ESP8266接口](https://img-blog.csdnimg.cn/20200323190334905.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
### 3.1 配置UART2外设驱动
ATK-ESP-01模块连接在Pandora开发板的UART2接口上，[UART标准库](https://blog.csdn.net/m0_37621078/article/details/100164277)与[驱动框架](https://blog.csdn.net/m0_37621078/article/details/104790217)在前面的博客中已经详细介绍过了。RT-Thread已经在Pandora BSP文件中，使用CubeMX帮我们配置好了UART2引脚，我们只需要在menuconfig中启用UART2相关的宏定义即可。

在AT命令集的交互关系中，ATK-ESP-01模块是提供WiFi网络服务的一方，也即AT Server；Pandora开发板是请求WiFi网络服务的一方，当需要网络服务时，向ATK-ESP-01模块发送AT命令，接收并处理ATK-ESP-01模块的响应数据，因此Pandora开发板作为AT Client，需要运行AT组件中的AT Client模块。我们在Kconfig中添加ESP8266扩展模块的宏定义配置项如下：

```c
// .\board\Kconfig

menu "Board extended module Drivers"
	......
    config BSP_USING_ESP8266
            bool "Enable ESP8266"
            select BSP_USING_UART2
            select RT_USING_AT
            select AT_USING_CLIENT
            default n
endmenu
```
保存添加的配置项，并在env工具中运行menuconfig命令，启用刚才的配置项，如下图所示：
![启用ESP8266模块](https://img-blog.csdnimg.cn/20200323205703961.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
我们已经在工程中启用了UART2外设与AT Client模块，接下来看AT Client的工作原理及其初始化过程。

### 3.2 AT Client初始化
AT Client主要是将比较直观的API转换为AT命令，并通过UART2接口发送给ESP8266模块。ESP8266执行完相应的AT命令后，将响应结果通过UART2返回给STM32L475，AT Client负责解析返回的AT响应结果及URC数据，并将解析后的数据递交给上面的应用层。

首先看AT Client 设备的数据结构描述：

```c
// rt-thread\components\net\at\include\at.h

struct at_client
{
    rt_device_t device;

    at_status_t status;
    char end_sign;

    char *recv_buffer;
    rt_size_t recv_bufsz;
    rt_size_t cur_recv_len;
    rt_sem_t rx_notice;
    rt_mutex_t lock;

    at_response_t resp;
    rt_sem_t resp_notice;
    at_resp_status_t resp_status;

    const struct at_urc *urc_table;
    rt_size_t urc_table_size;

    rt_thread_t parser;
};
typedef struct at_client *at_client_t;

struct at_response
{
    /* response buffer */
    char *buf;
    /* the maximum response buffer size */
    rt_size_t buf_size;
    /* the number of setting response lines
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    rt_size_t line_num;
    /* the count of received response lines */
    rt_size_t line_counts;
    /* the maximum response time */
    rt_int32_t timeout;
};
typedef struct at_response *at_response_t;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY' request by AT server */
struct at_urc
{
    const char *cmd_prefix;
    const char *cmd_suffix;
    void (*func)(const char *data, rt_size_t size);
};
typedef struct at_urc *at_urc_t;
```
at_client结构体包含了一个设备对象指针device，指向模块通信使用的外设接口，这里指向UART2外设对象。AT Client除了向AT Server发送AT命令请求外，还要接收并解析响应数据与URC数据，因此at_client结构体还包含了指向响应结构的指针resp与执行URC数据列表的指针urc_table。

AT Client为了提高对响应数据at_response与URC数据at_urc的解析效率，还创建了一个数据解析线程，at_client结构体中的成员parser便指向该解析线程。

 - **AT Client 初始化过程**

在使用AT Client前需要先对其进行初始化，完成AT Client运行所需的资源配置，AT Client的初始化过程如下：

```c
// rt-thread\components\net\at\src\at_client.c

static struct at_client at_client_table[AT_CLIENT_NUM_MAX] = { 0 };
/**
 * AT client initialize.
 * @param dev_name AT client device name
 * @param recv_bufsz the maximum number of receive buffer length
 * @return 0 : initialize success
 *        -1 : initialize failed
 *        -5 : no memory
 */
int at_client_init(const char *dev_name,  rt_size_t recv_bufsz)
{
    int idx = 0;
    int result = RT_EOK;
    rt_err_t open_result = RT_EOK;
    at_client_t client = RT_NULL;

    for (idx = 0; idx < AT_CLIENT_NUM_MAX && at_client_table[idx].device; idx++);
	......
    client = &at_client_table[idx];
    client->recv_bufsz = recv_bufsz;

    result = at_client_para_init(client);
    ......
    /* find and open command device */
    client->device = rt_device_find(dev_name);
    if (client->device)
    {
        RT_ASSERT(client->device->type == RT_Device_Class_Char);
        /* using DMA mode first */
        open_result = rt_device_open(client->device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
        /* using interrupt mode when DMA mode not supported */
        ......
        rt_device_set_rx_indicate(client->device, at_client_rx_ind);
    }
    ......
    if (result == RT_EOK)
    {
        client->status = AT_STATUS_INITIALIZED;
        
        rt_thread_startup(client->parser);
    }
    ......
    return result;
}
```
AT Client是支持多客户端模式的，当有多个AT Server扩展模块都需要为主机提供网络服务时，主机端就需要有多个at_client对象，因此AT Client使用一个数组at_client_table来管理多个at_client对象，数组成员数AT_CLIENT_NUM_MAX默认值为1（可通过menuconfig配置），我们只有一个AT Server扩展模块，因此不需要更改该值。接下来是AT Client解析线程的初始化过程：

```c
// rt-thread\components\net\at\src\at_client.c

/* initialize the client object parameters */
static int at_client_para_init(at_client_t client)
{
	......
    client->status = AT_STATUS_UNINITIALIZED;

    client->cur_recv_len = 0;
    client->recv_buffer = (char *) rt_calloc(1, client->recv_bufsz);
    ......
    client->lock = rt_mutex_create(name, RT_IPC_FLAG_FIFO);
    ......
    client->rx_notice = rt_sem_create(name, 0, RT_IPC_FLAG_FIFO);
    ......
    client->resp_notice = rt_sem_create(name, 0, RT_IPC_FLAG_FIFO);
    ......
    client->urc_table = RT_NULL;
    client->urc_table_size = 0;

    rt_snprintf(name, RT_NAME_MAX, "%s%d", AT_CLIENT_THREAD_NAME, at_client_num);
    client->parser = rt_thread_create(name,
                                     (void (*)(void *parameter))client_parser,
                                     client,
                                     1024 + 512,
                                     RT_THREAD_PRIORITY_MAX / 3 - 1,
                                     5);
    ......
    return result;
}

static void client_parser(at_client_t client)
{
    int resp_buf_len = 0;
    const struct at_urc *urc;
    rt_size_t line_counts = 0;

    while(1)
    {
        if (at_recv_readline(client) > 0)
        {
            if ((urc = get_urc_obj(client)) != RT_NULL)
            {
                /* current receive is urc, try to execute related operations */
                if (urc->func != RT_NULL)
                {
                    urc->func(client->recv_buffer, client->cur_recv_len);
                }
            }
            else if (client->resp != RT_NULL)
            {
                /* current receive is response */
                client->recv_buffer[client->cur_recv_len - 1] = '\0';
                if (resp_buf_len + client->cur_recv_len < client->resp->buf_size)
                {
                    /* copy response lines, separated by '\0' */
                    memcpy(client->resp->buf + resp_buf_len, client->recv_buffer, client->cur_recv_len);
                    resp_buf_len += client->cur_recv_len;
                    line_counts++;
                }
                else
                {
                    client->resp_status = AT_RESP_BUFF_FULL;
                }
                /* check response result */
                ......
                client->resp->line_counts = line_counts;

                client->resp = RT_NULL;
                rt_sem_release(client->resp_notice);
                resp_buf_len = 0, line_counts = 0;
            }
        }
    }
}
```
从上面AT Client解析线程的初始化过程可以看出，当client_parser接收到来自AT Server的数据后，会判断是at_urc数据还是at_response数据，并根据情况进行处理。如果是at_urc数据，则调用相应的执行函数；如果是at_response数据，则将接收到的数据放到at_response结构体中，检查并设置响应状态at_resp_status。待解析完接收到的数据后，client_parser线程会释放信号量client->resp_notice，通知AT命令请求函数（当发送AT命令请求后，函数会阻塞等待AT Server返回的响应数据），已接收并解析响应数据，可以继续往下执行了。

继续介绍AT Client 初始化过程，在完成解析线程client->parser的初始化后，开始配置AT命令通讯的设备client->device（这里指的是UART2设备）。我们在前面已经启用了UART2设备（RT-Thread已经帮我们配置好了UART2引脚），所以可以直接通过rt_device_find 查找到目标设备，通过rt_device_open函数完成UART2的初始化配置。WiFi 模块有主动接收网络数据的需求，因此少不了配置接收回调函数，这里接收回调函数的作用跟前面介绍的在Finsh组件中的作用一样，都是释放一个信号量，让等待获取该信号量的函数开始从UART读取并处理这些数据。接收回调过程如下：

```c
// rt-thread\components\net\at\src\at_client.c

static rt_err_t at_client_rx_ind(rt_device_t dev, rt_size_t size)
{
    int idx = 0;

    for (idx = 0; idx < AT_CLIENT_NUM_MAX; idx++)
    {
        if (at_client_table[idx].device == dev && size > 0)
        {
            rt_sem_release(at_client_table[idx].rx_notice);
        }
    }
    return RT_EOK;
}

static rt_err_t at_client_getchar(at_client_t client, char *ch, rt_int32_t timeout)
{
    rt_err_t result = RT_EOK;

    while (rt_device_read(client->device, 0, ch, 1) == 0)
    {
        rt_sem_control(client->rx_notice, RT_IPC_CMD_RESET, RT_NULL);

        result = rt_sem_take(client->rx_notice, rt_tick_from_millisecond(timeout));
        ......
    }
    return RT_EOK;
}

static int at_recv_readline(at_client_t client)
{
    ......
    while (1)
    {
        at_client_getchar(client, &ch, RT_WAITING_FOREVER);

        if (read_len < client->recv_bufsz)
        {
            client->recv_buffer[read_len++] = ch;
            client->cur_recv_len = read_len;
        }
        ......
        /* is newline or URC data */
        if ((ch == '\n' && last_ch == '\r') || (client->end_sign != 0 && ch == client->end_sign)
                || get_urc_obj(client))
        {
            ......
            break;
        }
        last_ch = ch;
    }
    return read_len;
}
```
当UART2触发中断时，会调用at_client_rx_ind函数，释放信号量client->rx_notice。AT Client 解析线程client_parser阻塞等待接收字符串at_recv_readline，该函数又阻塞等待接收字符at_client_getchar。从UART2设备读取字符的函数rt_device_read则阻塞等待信号量client->rx_notice，该信号量在UART2中断触发时通过回调函数at_client_rx_ind释放，AT Client 解析线程client_parser就可以继续处理接收到的数据了。

 - **at_response响应数据解析过程**

响应数据at_response的解析过程实际上就是字符串处理过程，从AT命令返回的数据中通过格式匹配，提取出有效数据的过程，必要的话再对有效数据按照需要的或者更直观的方式进行格式化处理。AT Client 为方便调试，还提供了 CLI 命令行交互模式，省去了数据解析过程，通过控制台console（比如Finsh）直接向AT Server发送AT命令，并将AT Server返回的响应数据照原样输出到控制台console。

从前面at_response和at_client的数据结构可以看出，在解析响应数据时，需要创建一个at_response对象。AT Client 解析线程client_parser接收到数据并判断是响应数据时，就是把接收缓冲区内的数据复制到at_response结构体，并完成at_response对象成员变量的配置，供其它函数处理。为了方便管理at_response对象，AT Client 实现了几个接口函数如下：

```c
// rt-thread\components\net\at\include\at.h

/* AT response object create and delete */
at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);
void at_delete_resp(at_response_t resp);
at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);

/* AT response line buffer get and parse response buffer arguments */
const char *at_resp_get_line(at_response_t resp, rt_size_t resp_line);
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);
int at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...);
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);
```
处理at_response对象的函数名称比较直观，实现原理也比较简单，可以按行解析和按关键字解析，支持可变长参数（类似于scanf与printf）。

 - **AT Client 发送命令并等待响应的过程**

AT Client 向AT Server 发送命令最终肯定要通过二者连接的通讯接口UART2来完成，但在命令请求发送前，也需要先对命令表达式进行处理，确保 AT Server 能够识别并给出响应。在命令发送完成后，需要等待接收到AT Server 返回的响应数据后，才能继续处理命令响应结果。该过程的实现过程如下：

```c
// rt-thread\components\net\at\src\at_client.c

int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...)
{
    ......
    /* check AT CLI mode */
    if (client->status == AT_STATUS_CLI && resp)
        return -RT_EBUSY;

    rt_mutex_take(client->lock, RT_WAITING_FOREVER);

    client->resp_status = AT_RESP_OK;
    client->resp = resp;

    va_start(args, cmd_expr);
    at_vprintfln(client->device, cmd_expr, args);
    va_end(args);

    if (resp != RT_NULL)
    {
        resp->line_counts = 0;
        if (rt_sem_take(client->resp_notice, resp->timeout) != RT_EOK)
        {
            ......
            client->resp_status = AT_RESP_TIMEOUT;
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        if (client->resp_status != AT_RESP_OK)
        {
            ......
            result = -RT_ERROR;
            goto __exit;
        }
    }
__exit:
    client->resp = RT_NULL;
    rt_mutex_release(client->lock);

    return result;
}

// rt-thread\components\net\at\src\at_utils.c

rt_size_t at_vprintfln(rt_device_t device, const char *format, va_list args)
{
    rt_size_t len;

    len = at_vprintf(device, format, args);
    rt_device_write(device, 0, "\r\n", 2);

    return len + 2;
}

rt_size_t at_vprintf(rt_device_t device, const char *format, va_list args)
{
    last_cmd_len = vsnprintf(send_buf, sizeof(send_buf), format, args);
	......
    return rt_device_write(device, 0, send_buf, last_cmd_len);
}
```
AT Client 发送的命令表达式支持变长参数，命令表达式cmd_expr最终是通过vsnprintf 解析的（响应结果表达式resp_expr最终是通过vsscanf解析的），解析后的字符串最终通过rt_device_write 发送给AT Server。

AT Client 命令发送后，会阻塞等待获取信号量client->resp_notice，该信号量在响应数据解析线程 client_parser中被释放，at_obj_exec_cmd获取到信号量client->resp_notice后，说明响应数据已经接收并复制到at_response结构体对象中，可以继续处理at_response对象中的响应数据了。

AT Client 除了向AT Server 发送命令请求，还会向其直接发送数据，这时就不需要解析命令表达式，直接通过rt_device_write 将数据发送给AT Server即可。AT Client 对数据发送、接收进行了再次封装，封装后的接口函数如下：

```c
// rt-thread\components\net\at\include\at.h
/* ========================== multiple AT client function ============================ */
/* AT client send or receive data */
rt_size_t at_client_obj_send(at_client_t client, const char *buf, rt_size_t size);
rt_size_t at_client_obj_recv(at_client_t client, char *buf, rt_size_t size, rt_int32_t timeout);
/* AT client send commands to AT server and waiter response */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...);
/* ========================== single AT client function ============================ */
#define at_client_send(buf, size)                at_client_obj_send(at_client_get_first(), buf, size)
#define at_client_recv(buf, size, timeout)       at_client_obj_recv(at_client_get_first(), buf, size, timeout)
#define at_exec_cmd(resp, ...)                   at_obj_exec_cmd(at_client_get_first(), resp, __VA_ARGS__)
```

### 3.3 urc_table配置过程
URC（Unsolicited Result Code）数据直译是不请自来的数据，也即并非是对命令请求的响应，而是AT Server主动向AT Client 发送的数据，一般是遇到一些特殊情况，比如 WiFi 连接断开、TCP 接收数据等特殊情况时，通知AT Client做出相应的操作。AT Client 对这些特殊情况会做出怎样的处理，就需要自己定义了，并将定义的at_urc 设置到at_client 对象的成员urc_table中，也即让client->urc_table指向这些at_urc 组成的列表urc_table。AT Client 提供的设置client->urc_table的函数接口如下：

```c
// rt-thread\components\net\at\include\at.h
/* ========================== multiple AT client function ============================ */
/* Set URC(Unsolicited Result Code) table */
void at_obj_set_urc_table(at_client_t client, const struct at_urc * table, rt_size_t size);
/* ========================== single AT client function ============================ */
#define at_set_urc_table(urc_table, table_sz)    at_obj_set_urc_table(at_client_get_first(), urc_table, table_sz)
```
在前面介绍的AT Client 初始化过程中，将client->urc_table设置为RT-NULL（见函数at_client_para_init代码）。如果我们需要AT Server（比如ESP8266）遇到一些比如 WiFi连接断开、TCP接收到数据等特殊情况时，能通知 AT Client 做出相应的操作，就需要我们自己定义需要的urc_table执行函数集合，并将其设置到client->urc_table中。

RT-Thread为我们提供了一套 AT device 移植软件包，其中就包括对 esp8266 的移植文件，可通过menuconfig启用，配置界面如下：
![启用AT SOCKET DEVICE MODULES ESP8266](https://img-blog.csdnimg.cn/20200324150559536.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
选择Espressif ESP8266，下面默认的设备名uart2 不需要更改，接下来是ESP8266 WiFi模块要连接的附近WiFi的SSID与密码，可以先变更为自己附近的WiFi SSID与密码，保存配置后，会自动下载esp8266移植文件at_socket_esp8266.c到packages目录下，我们可以在该文件中找到urc_table执行函数的实现与设置如下：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c

static struct at_urc urc_table[] = {
        {"SEND OK",          "\r\n",           urc_send_func},
        {"SEND FAIL",        "\r\n",           urc_send_func},
        {"Recv",             "bytes\r\n",      urc_send_bfsz_func},
        {"",                 ",CLOSED\r\n",    urc_close_func},
        {"+IPD",             ":",              urc_recv_func},
        {"busy p",           "\r\n",           urc_busy_p_func},
        {"busy s",           "\r\n",           urc_busy_s_func},
        {"WIFI CONNECTED",   "\r\n",           urc_func},
        {"WIFI DISCONNECT",  "\r\n",           urc_func},
};

static int at_socket_device_init(void)
{
    /* create current AT socket event */
    at_socket_event = rt_event_create("at_se", RT_IPC_FLAG_FIFO);
    ......
    /* create current AT socket event lock */
    at_event_lock = rt_mutex_create("at_se", RT_IPC_FLAG_FIFO);
    ......
    /* initialize AT client */
    at_client_init(AT_DEVICE_NAME, AT_DEVICE_RECV_BUFF_LEN);

    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
    ......
    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);
```
从urc_table的内容可以看出，实现的at_urc 执行函数大概有五类，分别是数据发送状态通知、socket断开连接通知、socket 接收数据通知、AT Server 忙碌不能响应命令的通知、WiFi已连接/已断开状态变更通知等。当AT Server 遇到上面这些定义的情况，将会调用我们定义的urc_table中的执行函数（见函数client_parser代码）。

实现了urc_table 执行函数集合，当然需要将其设置到client->urc_table中，设置urc_table的函数at_set_urc_table在ESP8266模块的初始化函数at_socket_device_init中被调用，同时被调用的还有前面介绍的AT Client组件初始化函数at_client_init。 

函数at_socket_device_init 会被自动初始化组件执行，我们启用相应的宏定义后，就可以在系统启动时自动完成 AT Client 组件的初始化和urc_table的设置。

到这里，我们就可以使用AT CLI 命令行交互模式，通过AT命令集向ESP8266 WiFi 扩展模块请求网络服务了，可使用的AT命令集可以参考文件《ATK-ESP8266 WIFI用户手册》或网页[ESP_AT_Commands_Set.md](https://github.com/espressif/esp-at/blob/master/docs/ESP_AT_Commands_Set.md)，下面给出部分ATK-ESP8266模块支持的AT命令集列表：
![ESP8266支持的AT命令集](https://img-blog.csdnimg.cn/20200325133811827.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
使用上面提供的AT命令集，编写一个示例程序，通过AT CLI 命令行交互模式，实现连接周围WiFi 热点、查询IP/MAC地址与网络状态、解析域名、ping通外网的功能，AT 命令集的交互命令及参数解释如下图所示：
![AT命令PING示例程序](https://img-blog.csdnimg.cn/20200324225421791.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)

## 四、AT组件层与SAL组件层
网络应用程序一般都使用BSD Socket API 编写，要想让AT Client 组件能支持 BSD Socket API，需要增加一层协议无关接口层 SAL。前篇博客[netdev/SAL原理](https://blog.csdn.net/m0_37621078/article/details/104836942)中对SAL组件实现原理有过介绍，SAL组件是依赖于netdev组件的，要想使用SAL组件，需要同时启用netdev组件与SAL组件，在menuconfig中启用这两个组件的配置方法在前篇博客已经介绍过了。需要注意的一点是，启用SAL组件时，需要选择协议栈支持类型 AT Commands Stack，配置界面如下：
![启用SAL支持AT命令](https://img-blog.csdnimg.cn/20200324161614856.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
### 4.1 AT socket API
AT组件要想提供socket API，也需要维护一个socket 结构体或类，作为socket API 管理网络连接资源的载体。AT组件提供的socket数据结构是at_socket，代码描述如下：

```c
// rt-thread\components\net\at\at_socket\at_socket.h

struct at_socket
{
    /* AT socket magic word */
    uint32_t magic;

    int socket;
    /* type of the AT socket (TCP, UDP or RAW) */
    enum at_socket_type type;
    /* current state of the AT socket (CONNECT, CLOSED) */
    enum at_socket_state state;
    /* receive semaphore, received data release semaphore */
    rt_sem_t recv_notice;
    rt_mutex_t recv_lock;
    rt_slist_t recvpkt_list;

    /* timeout to wait for send or received data in milliseconds */
    int32_t recv_timeout;
    int32_t send_timeout;
    /* A callback function that is informed about events for this AT socket */
    at_socket_callback callback;

    /* number of times data was received, set by event_callback() */
    uint16_t rcvevent;
    /* number of times data was ACKed (free send buffer), set by event_callback() */
    uint16_t sendevent;
    /* error happened for this socket, set by event_callback() */
    uint16_t errevent;

#ifdef SAL_USING_POSIX
    rt_wqueue_t wait_head;
#endif
};

/* A callback prototype to inform about events for AT socket */
typedef void (* at_socket_callback)(struct at_socket *conn, int event, uint16_t len);

/* AT receive package list structure */
struct at_recv_pkt
{
    rt_slist_t list;
    size_t bfsz_totle;
    size_t bfsz_index;
    char *buff;
};
typedef struct at_recv_pkt *at_recv_pkt_t;

// rt-thread\components\net\at\at_socket\at_socket.c
/* the global array of available sockets */
static struct at_socket sockets[AT_SOCKETS_NUM] = { 0 };
```
at_socket 结构包含了socket 编号/类型/状态、接收数据包及信号量、发送/接收数据的等待超时、接收/发送/错误事件的次数等信息。

AT组件对外提供的socket API 如下：

```c
// rt-thread\components\net\at\at_socket\at_socket.h

int at_socket(int domain, int type, int protocol);
int at_closesocket(int socket);
int at_shutdown(int socket, int how);
int at_bind(int socket, const struct sockaddr *name, socklen_t namelen);
int at_connect(int socket, const struct sockaddr *name, socklen_t namelen);
int at_sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int at_send(int socket, const void *data, size_t size, int flags);
int at_recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int at_recv(int socket, void *mem, size_t len, int flags);
int at_getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen);
int at_setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen);
struct hostent *at_gethostbyname(const char *name);
int at_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void at_freeaddrinfo(struct addrinfo *ai);

struct at_socket *at_get_socket(int socket);
void at_socket_device_register(const struct at_device_ops *ops);

#ifndef RT_USING_SAL

#define socket(domain, type, protocol)                      at_socket(domain, type, protocol)
#define closesocket(socket)                                 at_closesocket(socket)
#define shutdown(socket, how)                               at_shutdown(socket, how)
#define bind(socket, name, namelen)                         at_bind(socket, name, namelen)
#define connect(socket, name, namelen)                      at_connect(socket, name, namelen)
#define sendto(socket, data, size, flags, to, tolen)        at_sendto(socket, data, size, flags, to, tolen)
#define send(socket, data, size, flags)                     at_send(socket, data, size, flags)
#define recvfrom(socket, mem, len, flags, from, fromlen)    at_recvfrom(socket, mem, len, flags, from, fromlen)
#define getsockopt(socket, level, optname, optval, optlen)  at_getsockopt(socket, level, optname, optval, optlen)
#define setsockopt(socket, level, optname, optval, optlen)  at_setsockopt(socket, level, optname, optval, optlen)

#define gethostbyname(name)                                 at_gethostbyname(name)
#define getaddrinfo(nodename, servname, hints, res)         at_getaddrinfo(nodename, servname, hints, res)
#define freeaddrinfo(ai)                                    at_freeaddrinfo(ai)

#endif /* RT_USING_SAL */
```
前面已经启用了SAL组件，因此我们可以使用下半部分BSD Socket API 编写网络应用。对比下前篇博客介绍的BSD Socket API，发现AT组件提供的socket API 并不完整，比如没有accept与listen接口，因此就不支持通过at_socket API 完成 TCP server 编程。

要使用AT组件提供的socket API 编写网络程序，自然需要实现一套接口函数，AT socket API 最终是通过调用这套接口函数at_device_ops来实现相应功能的，需要我们实现并向AT组件注册的接口函数集合at_device_ops包含以下成员函数：

```c
// rt-thread\components\net\at\at_socket\at_socket.h

/* AT device socket options function */
struct at_device_ops
{
    int (*at_connect)(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client);
    int (*at_closesocket)(int socket);
    int (*at_send)(int socket, const char *buff, size_t bfsz, enum at_socket_type type);
    int (*at_domain_resolve)(const char *name, char ip[16]);
    void (*at_set_event_cb)(at_socket_evt_t event, at_evt_cb_t cb);
};

typedef void (*at_evt_cb_t)(int socket, at_socket_evt_t event, const char *buff, size_t bfsz);
```
将实现的操作函数集合at_device_ops注册到AT组件的函数代码如下：

```c
// rt-thread\components\net\at\at_socket\at_socket.c

void at_socket_device_register(const struct at_device_ops *ops)
{
    RT_ASSERT(ops);
    RT_ASSERT(ops->at_connect);
    RT_ASSERT(ops->at_closesocket);
    RT_ASSERT(ops->at_send);
    RT_ASSERT(ops->at_domain_resolve);
    RT_ASSERT(ops->at_set_event_cb);
    at_dev_ops = (struct at_device_ops *) ops;
}

/* AT device socket options */
static struct at_device_ops *at_dev_ops = RT_NULL;
```
操作函数集合at_dev_ops是一个全局变量（被static修饰，只限本文件内有效），在文件at_socket.c 中实现的 at_socket API 都可以调用该函数集合以实现自身功能。

我们要想使用AT组件提供的socket，就需要自己实现at_device_ops函数集合，并通过at_socket_device_register函数将其注册到AT组件中，这部分任务依然由前面提到过的 AT device移植文件at_socket_esp8266.c中的代码来实现。

### 4.2 at_device_ops实现与注册
我们可以猜到，操作函数集合at_device_ops的实现最终要靠AT命令集，毕竟单靠AT命令集就可以向AT Server请求网络服务，socket API 实际上就是对AT 命令集的再封装。

ESP8266 移植文件实现的操作函数集合at_device_ops及其注册过程如下：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c

static const struct at_device_ops esp8266_socket_ops =
{
    esp8266_socket_connect,
    esp8266_socket_close,
    esp8266_socket_send,
    esp8266_domain_resolve,
    esp8266_socket_set_event_cb,
};

static int at_socket_device_init(void)
{
    ......
    /* set esp8266 AT Socket options */
    at_socket_device_register(&esp8266_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);
```
ESP8266 为AT组件实现的操作函数集合esp8266_socket_ops，其注册过程也是在at_socket_device_init函数中完成的，自动初始化时被调用。下面以esp8266_socket_connect 函数的实现为例，说明AT组件执行AT命令的过程：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c

static int esp8266_socket_connect(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client)
{
    at_response_t resp = RT_NULL;
    int result = RT_EOK;
    rt_bool_t retryed = RT_FALSE;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    ......
    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

__retry:
    if (is_client)
    {
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands to connect TCP server */
            if (at_exec_cmd(resp, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d,60", socket, ip, port) < 0)
                result = -RT_ERROR;
            break;

        case AT_SOCKET_UDP:
            if (at_exec_cmd(resp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d", socket, ip, port) < 0)
                result = -RT_ERROR;
            break;

        default:
            result = -RT_ERROR;
            goto __exit;
        }
    }
    if (result != RT_EOK && !retryed)
    {
        if (esp8266_socket_close(socket) < 0)
            goto __exit;
        retryed = RT_TRUE;
        result = RT_EOK;
        goto __retry;
    }
    
__exit:
    rt_mutex_release(at_event_lock);
    if (resp)
        at_delete_resp(resp);

    return result;
}
```
从上面的代码可以看出，esp8266_socket_connect函数主要是通过执行AT命令：AT+CIPSTART=<>实现的。在每次执行AT命令前，需要通过调用函数at_create_resp为响应数据准备资源，并在最后释放此资源。操作函数集合esp8266_socket_ops中其余函数的实现也是采用类似的方法，只不过执行的AT命令不一样。

### 4.3 sal_proto_family配置
前篇博客[netdev/SAL原理](https://blog.csdn.net/m0_37621078/article/details/104836942)中介绍给SAL的实现原理，要想使用SAL层提供的socket API，需要先实现并向其注册操作函数集合sal_socket_ops与sal_netdb_ops。前篇博客已经介绍了LwIP协议向SAL层注册的操作函数集合lwip_socket_ops与lwip_netdb_ops，AT组件向SAL层注册操作函数集合at_socket_ops与at_netdb_ops的过程类似，代码逻辑如下：

```c
// rt-thread\components\net\sal_socket\impl\af_inet_at.c

static const struct sal_socket_ops at_socket_ops =
{
    at_socket,
    at_closesocket,
    at_bind,
    NULL,
    at_connect,
    NULL,
    at_sendto,
    at_recvfrom,
    at_getsockopt,
    at_setsockopt,
    at_shutdown,
    NULL,
    NULL,
    NULL,
#ifdef SAL_USING_POSIX
    at_poll,
#endif /* SAL_USING_POSIX */
};

static const struct sal_netdb_ops at_netdb_ops = 
{
    at_gethostbyname,
    NULL,
    at_getaddrinfo,
    at_freeaddrinfo,
};

static const struct sal_proto_family at_inet_family =
{
    AF_AT,
    AF_INET,
    &at_socket_ops,
    &at_netdb_ops,
};


/* Set AT network interface device protocol family information */
int sal_at_netdev_set_pf_info(struct netdev *netdev)
{
    RT_ASSERT(netdev);

    netdev->sal_user_data = (void *) &at_inet_family;
    return 0;
}
```
其中at_socket_ops与at_netdb_ops均由AT组件提供，也就是前面介绍过的AT socket API，从这儿也可以明显看出AT组件实现的socket API 并不完整。操作函数集合at_socket_ops与at_netdb_ops要想注册到SAL组件中，需要调用sal_at_netdev_set_pf_info函数，前篇博客也介绍过，该函数是被netdev_add调用的。

既然SAL组件依赖于netdev组件，要使用netdev组件则需要添加网卡，并向其注册操作函数集合netdev_ops。netdev层靠近AT device esp8266，netdev_ops的实现与注册也是由移植文件at_socket_esp8266.c中的代码实现的。

### 4.4 esp8266_netdev注册
操作函数集合netdev_ops的实现最终自然也要靠AT命令集，ESP8266 移植文件实现的操作函数集合esp8266_netdev_ops及其注册过程如下：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c\

const struct netdev_ops esp8266_netdev_ops =
{
    esp8266_netdev_set_up,
    esp8266_netdev_set_down,

    esp8266_netdev_set_addr_info,
    esp8266_netdev_set_dns_server,
    esp8266_netdev_set_dhcp,

    esp8266_netdev_ping,
    esp8266_netdev_netstat,
};

static struct netdev *esp8266_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1500
#define HWADDR_LEN          6
    struct netdev *netdev = RT_NULL;

    netdev = (struct netdev *) rt_calloc(1, sizeof(struct netdev));
    ......
    netdev->mtu = ETHERNET_MTU;
    netdev->ops = &esp8266_netdev_ops;
    netdev->hwaddr_len = HWADDR_LEN;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    netdev_register(netdev, netdev_name, RT_NULL);

    return netdev;
}

static int at_socket_device_init(void)
{
    ......
    /* Add esp8266 to the netdev list */
    esp8266_netdev_add(ESP8266_NETDEV_NAME);
    ......
    /* set esp8266 AT Socket options */
    at_socket_device_register(&esp8266_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);
```
操作函数集合esp8266_netdev_ops中函数的实现过程跟前面介绍的esp8266_socket_ops的实现过程类似，前面已经通过esp8266_socket_connect的实现代码展示过了，这里就不赘述了。esp8266_netdev_ops在函数esp8266_netdev_add中被注册到netdev组件，同时配置at_inet_family的函数sal_at_netdev_set_pf_info也在函数esp8266_netdev_add中被调用。函数esp8266_netdev_add也在at_socket_device_init中被调用，而后者被自动初始化组件执行。

我们看函数esp8266_netdev_add的实现代码，发现netdev结构体中的很多成员信息并没有被设置或初始化。在LwIP协议栈中添加netdev网卡时，将lwip_netif 指针赋值给了 netdev->user_data，也即可以通过netdev 访问LwIP网卡接口 lwip_netif 的所有信息，可以不必对netdev中的所有成员进行配置或初始化。

但AT组件并没有为我们提供类似netif 的网卡接口结构，netdev->user_data 只能赋值为空指针RT_NULL，我们需要通过AT命令获取netdev 需要的信息，并使用注册的esp8266_netdev_ops 接口函数完成netdev 成员信息的设置。通过AT命令获取netdev网卡信息并设置到netdev设备对象中的函数是exp8266_get_netdev_info，该函数的部分实现代码如下：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c

static void exp8266_get_netdev_info(struct rt_work *work, void *work_data)
{
#define AT_ADDR_LEN     32
    at_response_t resp = RT_NULL;
    char ip[AT_ADDR_LEN], mac[AT_ADDR_LEN];
    char gateway[AT_ADDR_LEN], netmask[AT_ADDR_LEN];
    char dns_server1[AT_ADDR_LEN] = {0}, dns_server2[AT_ADDR_LEN] = {0};
    const char *resp_expr = "%*[^\"]\"%[^\"]\"";
    const char *resp_dns = "+CIPDNS_CUR:%s";
    ip_addr_t sal_ip_addr;
    rt_uint32_t mac_addr[6] = {0};
    rt_uint32_t num = 0; 
    rt_uint8_t dhcp_stat = 0;
    struct netdev *netdev = RT_NULL;

    netdev = (struct netdev *)work_data;

    rt_memset(ip, 0x00, sizeof(ip));
    rt_memset(mac, 0x00, sizeof(mac));
    rt_memset(gateway, 0x00, sizeof(gateway));
    rt_memset(netmask, 0x00, sizeof(netmask));

    resp = at_create_resp(512, 0, rt_tick_from_millisecond(300));
    if (!resp) return;

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    /* send mac addr query commond "AT+CIFSR" and wait response */
    if (at_exec_cmd(resp, "AT+CIFSR") < 0)
        goto __exit;

    if (at_resp_parse_line_args(resp, 2, resp_expr, mac) <= 0)
        goto __exit;

    /* send addr info query commond "AT+CIPSTA?" and wait response */
    if (at_exec_cmd(resp, "AT+CIPSTA?") < 0)
        goto __exit;

    if (at_resp_parse_line_args(resp, 1, resp_expr, ip) <= 0 ||
            at_resp_parse_line_args(resp, 2, resp_expr, gateway) <= 0 ||
            at_resp_parse_line_args(resp, 3, resp_expr, netmask) <= 0)
        goto __exit;

    /* set netdev info */
    inet_aton(ip, &sal_ip_addr);
    netdev_low_level_set_ipaddr(netdev, &sal_ip_addr);
    inet_aton(gateway, &sal_ip_addr);
    netdev_low_level_set_gw(netdev, &sal_ip_addr);
    inet_aton(netmask, &sal_ip_addr);
    netdev_low_level_set_netmask(netdev, &sal_ip_addr);
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]);
    for (num = 0; num < netdev->hwaddr_len; num++)
        netdev->hwaddr[num] = mac_addr[num];

    /* send dns server query commond "AT+CIPDNS_CUR?" and wait response */
    if (at_exec_cmd(resp, "AT+CIPDNS_CUR?") < 0)
        goto __exit;

    if (at_resp_parse_line_args(resp, 1, resp_dns, dns_server1) <= 0 &&
            at_resp_parse_line_args(resp, 2, resp_dns, dns_server2) <= 0)
        goto __exit;

    if (strlen(dns_server1) > 0)
    {
        inet_aton(dns_server1, &sal_ip_addr);
        netdev_low_level_set_dns_server(netdev, 0, &sal_ip_addr);
    }

    if (strlen(dns_server2) > 0)
    {
        inet_aton(dns_server2, &sal_ip_addr);
        netdev_low_level_set_dns_server(netdev, 1, &sal_ip_addr);
    }

    /* send DHCP query commond " AT+CWDHCP_CUR?" and wait response */
    if (at_exec_cmd(resp, "AT+CWDHCP_CUR?") < 0)
        goto __exit;

    /* parse response data, get the DHCP status */
    if (at_resp_parse_line_args_by_kw(resp, "+CWDHCP_CUR:", "+CWDHCP_CUR:%d", &dhcp_stat) < 0)
        goto __exit;

    /* Bit0 - SoftAP DHCP status, Bit1 - Station DHCP status */
    if (dhcp_stat & 0x02)
        netdev_low_level_set_dhcp_status(netdev, RT_TRUE);
    else
        netdev_low_level_set_dhcp_status(netdev, RT_FALSE);

__exit:
    rt_mutex_release(at_event_lock);

    if (resp)
        at_delete_resp(resp);
}

static int at_socket_device_init(void)
{
    ......
    /* initialize esp8266 net workqueue */
    rt_delayed_work_init(&esp8266_net_work, exp8266_get_netdev_info, (void *)netdev_get_by_name(ESP8266_NETDEV_NAME));
	......
}
INIT_APP_EXPORT(at_socket_device_init);
```
到这里netdev组件与SAL组件就都完成初始化和配置了，我们已经可以正常使用netdev和SAL提供的接口函数（详见前篇博客）了。在函数at_socket_device_init中调用的rt_delayed_work_init实际上是初始化一个工作队列，也即将函数exp8266_get_netdev_info连同参数放入工作队列esp8266_net_work，该函数允许被延迟执行。

### 4.5 esp8266网络初始化
还记得前面我们启用AT device 移植软件包时，有配置WiFi SSID与密码的选项吗？esp8266 移植文件为我们提供了提前配置WiFi 网络的功能，当esp8266的主控系统启动时，会按照预先的WiFi 网络配置搜索周围热点，如果WiFi 网络配置正确，将会自动连接网络，然后运行我们的网络应用程序。esp8266的网络初始化配置过程如下：

```c
// packages\at_device-v1.6.0\at_socket_esp8266.c

int esp8266_net_init(void)
{
#ifdef PKG_AT_INIT_BY_THREAD
    rt_thread_t tid;
    tid = rt_thread_create("esp8266_net_init", esp8266_init_thread_entry, RT_NULL, ESP8266_THREAD_STACK_SIZE, ESP8266_THREAD_PRIORITY, 20);
    if (tid)
        rt_thread_startup(tid);
#else
    esp8266_init_thread_entry(RT_NULL);
#endif

    return RT_EOK;
}

static void esp8266_init_thread_entry(void *parameter)
{
    at_response_t resp = RT_NULL;
    rt_err_t result = RT_EOK;
    rt_size_t i;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    ......
    rt_thread_delay(rt_tick_from_millisecond(5000));
    /* reset module */
    AT_SEND_CMD(resp, "AT+RST");
    /* reset waiting delay */
    rt_thread_delay(rt_tick_from_millisecond(1000));
    /* disable echo */
    AT_SEND_CMD(resp, "ATE0");
    /* set current mode to Wi-Fi station */
    AT_SEND_CMD(resp, "AT+CWMODE=1");
    /* get module version */
    AT_SEND_CMD(resp, "AT+GMR");
    /* show module version */
    ......
    /* connect to WiFi AP */
    if (at_exec_cmd(at_resp_set_info(resp, 128, 0, 20 * RT_TICK_PER_SECOND), "AT+CWJAP=\"%s\",\"%s\"",
            AT_DEVICE_WIFI_SSID, AT_DEVICE_WIFI_PASSWORD) != RT_EOK)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    AT_SEND_CMD(resp, "AT+CIPMUX=1");

__exit:
    if (resp)
        at_delete_resp(resp);

    if (!result)
        netdev_low_level_set_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_TRUE);
    else
        netdev_low_level_set_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_FALSE);
}

static int at_socket_device_init(void)
{
    ......
    /* initialize AT client */
    at_client_init(AT_DEVICE_NAME, AT_DEVICE_RECV_BUFF_LEN);

    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    /* Add esp8266 to the netdev list */
    esp8266_netdev_add(ESP8266_NETDEV_NAME);

    /* initialize esp8266 net workqueue */
    rt_delayed_work_init(&esp8266_net_work, exp8266_get_netdev_info, (void *)netdev_get_by_name(ESP8266_NETDEV_NAME));

    /* initialize esp8266 network */
    esp8266_net_init();

    /* set esp8266 AT Socket options */
    at_socket_device_register(&esp8266_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);
```
函数esp8266_init_thread_entry中用到的两个宏AT_DEVICE_WIFI_SSID与AT_DEVICE_WIFI_PASSWORD就是我们在menuconfig中配置的宏定义。esp8266的网络初始化函数esp8266_net_init也是被at_socket_device_init调用的，也即在系统启动时都会被自动初始化组件调用执行。到这里AT device esp8266的初始化过程就完成了，包括netdev组件与SAL组件的初始化配置也完成了，我们可以使用BSD socket API 编写网络应用程序了（需要注意部分socket API 并没有实现）。

我们配置完前面介绍的模块后，在env中执行scons --target=mdk5生成MDK工程，使用MDK打开工程文件编译无报错，将程序烧录到Pandora开发板中，执行ifconfig命令与ping命令，看看ESP8266模块是否可以正常访问外网：
![ping测试](https://img-blog.csdnimg.cn/2020032423093712.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)
从上面ifconfig与ping命令的执行结果看，ESP8266已经成功连接上WiFi，并能访问外部网络（比如www.baidu.com），说明我们的移植暂未出现明显问题，可以在此基础上使用BSD Socket API 编写网络应用程序了。

## 五、AT Socket 应用示例
本想使用前篇博客的示例程序，由Pandora开发板作为HTTP Server，当有Client连接请求时，向客户端发送一个网页，并打印实时温湿度信息，但AT Socket 支持的socket API 并不完整，没有提供 accept / listen 等的实现。因此，本示例将Pandora开发板作为 TCP Client ，通过msh命令，让Pandora开发板连接到某个 TCP Server。当TCP Server向Pandora 发送 “get” 请求时，Pandora向 TCP Server 返回实时采集的温湿度信息，当Pandora接收到 “exit” 命令时，将断开该连接并释放资源。

Pandora开发板采集温湿度数据的方法，在前篇博客示例程序中已经介绍过了，这里直接给出示例代码：

```c
// applications\socket_tcp_demo.c

#include <rtthread.h>
#include <sys\socket.h>		/* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <string.h>
#include <sensor.h>

#define DBG_TAG               "Socket"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

/* defined received buffer size */
#define BUFSZ       512

/* defined aht10 sensor name */
#define SENSOR_TEMP_NAME    "temp_aht10"
#define SENSOR_HUMI_NAME    "humi_aht10"

static char Sensor_Data[] ="The current temperature is: %3d.%d C, humidity is: %3d.%d %.";

static void tcpclient(int argc, char **argv)
{
    char *buffer;
    const char *url;
    struct hostent *host;
    int sock, port, bytes_recv, ret;
    struct sockaddr_in server_addr;
    /* sensor设备对象与sensor数据类型 */
    rt_device_t sensor_temp, sensor_humi;
    struct rt_sensor_data temp_data, humi_data;

    if (argc < 3)
    {
        LOG_E("Usage: tcpclient URL PORT\n");
        LOG_E("Like: tcpclient 192.168.12.44 5000\n");
        return ;
    }

    url = argv[1];
    port = strtoul(argv[2], 0, 10);
    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = gethostbyname(url);

    /* 分配接收用的数据缓冲 */
    buffer = rt_malloc(BUFSZ);
    if(buffer == RT_NULL)
    {
      LOG_E("No memory\n");
      return;
    }

    /* 创建一个socket，类型是SOCK_STREAM，TCP类型 */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(sock == -1)
    {
      LOG_E("Socket error\n");
      rt_free(buffer); 
      return;
    }

    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 绑定socket到服务端地址 */
    ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if(ret == -1)
    {
      LOG_E("Connect fail\n");
      closesocket(sock);
      rt_free(buffer);    
      return;
    }

    LOG_I("Successfully connected to (%s , %d)\n",
            inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    
    while(1) {
        /* 从sock中接收最大 BUFSZ-1 字节数据 */
        bytes_recv = recv(sock, buffer, BUFSZ-1, 0);
        if (bytes_recv <=  0)
        {
            LOG_E("Received error,close the socket.\r\n");
            closesocket(sock);
            rt_free(buffer);    
            return;
        }
        /* 添加字符串结束符 */
        buffer[bytes_recv] = '\0';

        /* 如果接收数据是get，向客户端发送温湿度数据 */
        if(strncmp(buffer, "get", 3) == 0)
        {
            /* 发现并打开温湿度传感器设备 */
            sensor_temp = rt_device_find(SENSOR_TEMP_NAME);
            rt_device_open(sensor_temp, RT_DEVICE_FLAG_RDONLY);
            sensor_humi = rt_device_find(SENSOR_HUMI_NAME);
            rt_device_open(sensor_humi, RT_DEVICE_FLAG_RDONLY);

            rt_thread_mdelay(1000);

            /* 读取温湿度数据，并将其填入Sensor_Data字符串 */
            rt_device_read(sensor_temp, 0, &temp_data, 1);
            rt_device_read(sensor_humi, 0, &humi_data, 1);
            rt_sprintf(buffer, Sensor_Data, 
                    temp_data.data.temp / 10, temp_data.data.temp % 10,
                    humi_data.data.humi / 10, humi_data.data.humi % 10);
            /* 发送数据到sock连接 */
            send(sock, buffer, strlen(buffer), 0);
            LOG_D("%s\n", buffer);
        }

        /* 如果接收数据是exit，退出 */
        if (strncmp(buffer, "exit", 4) == 0)
        {
            LOG_I("Got a 'q' or 'Q',close the socket.\r\n");
            closesocket(sock);
            rt_free(buffer);

            rt_device_close(sensor_temp);
            rt_device_close(sensor_humi);
            
            break;
        }
    }
    return;
}
MSH_CMD_EXPORT(tcpclient, Command: tcpclient URL PORT);
```
使用scons命令创建MDK工程，并使用MDK打开工程文件，编译无报错，将其烧录到开发板中，先通过ifconfig与ping命令确认esp8266已分配IP，并可以访问外部网络。

打开网络助手（比如NetAssist），启动一个TCP Server，然后在finsh界面输入命令：tcpclient IP port（其中的IP与port是网络助手启动的TCP Server的IP与port）。待Pandora成功连接我们启动的TCP Server，在网络助手消息发送框输入“get”，发送后可以看到Pandora返回的温湿度传感器数据，过程如下：
![AT Socket 示例结果](https://img-blog.csdnimg.cn/20200325222356252.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NjIxMDc4,size_16,color_FFFFFF,t_70)

# 更多文章：

 - 《[LwIP_STM32 Porting Source Code based on enc28j60](https://github.com/StreamAI/LwIP_Projects/tree/master/stm32l475-pandora-lwip)》
 - [《IOT-OS之RT-Thread（十五）--- SDIO设备对象管理 + AP6181(BCM43362) WiFi模块》](https://blog.csdn.net/m0_37621078/article/details/105097567)
 - 《[IOT-OS之RT-Thread（十六）--- WLAN管理框架 + AP6181(BCM43362) WiFi模块](https://blog.csdn.net/m0_37621078/article/details/105264345)》

