#include "lwip/api.h"
#include "rtthread.h"

static void udpecho_thread(void *arg)
{
  static struct netconn *conn;
  static struct netbuf *buf;
  static ip_addr_t *addr;
  static unsigned short port;

  err_t err;
  LWIP_UNUSED_ARG(arg);

  conn = netconn_new(NETCONN_UDP);
  LWIP_ASSERT("con != NULL", conn != NULL);
  netconn_bind(conn, NULL, 7);

  while (1) {
    err = netconn_recv(conn, &buf);
    if (err == ERR_OK) {
      addr = netbuf_fromaddr(buf);
      port = netbuf_fromport(buf);
      rt_kprintf("addr: %ld, poty: %d.\n", addr->addr, port);

	    err = netconn_send(conn, buf);
      if(err != ERR_OK) {
          LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
      }  
      netbuf_delete(buf);
      rt_thread_mdelay(100);
    }
  }
}

static void udpecho_init(void)
{
  sys_thread_new("udpecho", udpecho_thread, NULL, 1024, 25);
  rt_kprintf("Startup a udp echo server.\n");
}
MSH_CMD_EXPORT_ALIAS(udpecho_init, seqapi_udpecho, sequential api udpecho init);
