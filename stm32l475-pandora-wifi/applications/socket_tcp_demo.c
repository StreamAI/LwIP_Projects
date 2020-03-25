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
