#include "sys/socket.h"
#include "lwip/sys.h"
#include "rtthread.h"

#define SOCK_TARGET_HOST  "192.168.0.4"
#define SOCK_TARGET_PORT  8080

static char rxbuf[1024];
static char sndbuf[64];

static void socket_timeoutrecv(void *arg)
{
  int sock;
  int ret;
  int opt;
  struct sockaddr_in addr;
  size_t len;

  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SOCK_TARGET_PORT);
  addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);

  /* connect */
  do
  {
      sock = socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("sock >= 0", sock >= 0);
      ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	    rt_kprintf("socket connect result [%d]\n", ret);
	    if(ret != 0)
      {
           closesocket(sock);
	    }
  }while(ret != 0);
  /* should succeed */
  if(ret != 0)
  {
     rt_kprintf("socket connect error %d\n", ret);
     ret = closesocket(sock);
     while(1) sys_msleep(1000);
  }

  /* set recv timeout (100 ms) */
  opt = 100;
  ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(int));

  while(1)
  {
      len = 0;
      ret = read(sock, rxbuf, 1024);
      if (ret > 0) {
          len = ret;
      }
      rt_kprintf("read [%d] data\n", ret); 

	    len = rt_sprintf(sndbuf,"Client:I receive [%d] data\n", len);
      ret = write(sock, sndbuf, len);
      if(ret>0)
      {
          rt_kprintf("socket send %d data\n",ret);
	    }
	    else
	    {
          ret = closesocket(sock);
	        rt_kprintf("socket closed %d\n",ret);
		      while(1) sys_msleep(1000);
	    }
  }
}

static void socket_examples_init(void)
{
  sys_thread_new("socket_timeoutrecv", socket_timeoutrecv, NULL, 2048, TCPIP_THREAD_PRIO+1);
  rt_kprintf("Startup a TCP client in order to connect to the 192.168.0.4:8080 server .\n");
}
MSH_CMD_EXPORT_ALIAS(socket_examples_init, socket_demo, socket examples);
