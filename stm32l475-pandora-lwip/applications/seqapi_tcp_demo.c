#include "lwip/api.h"
#include "rtthread.h"
#include "board.h"
#include <stdbool.h>

/* defined the LED1 pin: PE9 */
#define LED1_PIN    GET_PIN(E, 9)

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_html[] = "<html><head><title>Congrats!</title></head><body><h1>Welcome to LwIP 2.1.0 HTTP server!</h1> \
                                                        <center><p>This is a test page based on netconn API.</center></body></html>";
const unsigned char LedOn_Data[] ="\
	<HTML>	\
	<head><title>LED Monitor</title></head> \
	<center>	\
	<p>   \
	<center>LED is on!!</center>\
	<form method=post action=\"off\" name=\"ledform\"> \
	<font size=\"2\">Change LED status:</font>  \
	<input type=\"submit\" value=\"off\">	\
	</form> \
	</p>  \
	</center>  \
	</HTML>";

const unsigned char LedOff_Data[] ="\
	<HTML>	\
	<head><title>LED Monitor</title></head> \
	<center>	\
	<p>   \
	<center>LED is off!!</center>\
	<form method=post action=\"on\" name=\"ledform\"> \
	<font size=\"2\">Change LED status:</font>  \
	<input type=\"submit\" value=\"on\">	\
	</form> \
	</p>  \
	</center>  \
	</HTML>";
	
static bool led_on = false;

/*send page*/
static void httpserver_send_html(struct netconn *conn, bool led_status)
{
    netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);  
    /* Send our HTML page */
    netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
      
     /* Send our HTML page */
	  if(led_status == true)
        netconn_write(conn, LedOn_Data, sizeof(LedOn_Data)-1, NETCONN_NOCOPY);
	  else
	  	  netconn_write(conn, LedOff_Data, sizeof(LedOff_Data)-1, NETCONN_NOCOPY);

}
/** Serve one HTTP connection accepted in the http thread */

static void httpserver_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  
  /* Read the data from the port, blocking if nothing yet there. 
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 && buf[0]=='G' && buf[1]=='E' && buf[2]=='T' &&
        buf[3]==' ' && buf[4]=='/' ) {
      
      /* Send the HTML header 
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
    httpserver_send_html(conn, led_on);
    }
	else if(buflen>=8 && buf[0]=='P' && buf[1]=='O' && buf[2]=='S' && buf[3]=='T')
	{
		if(buf[6]=='o' && buf[7]=='n'){		//请求打开LED
		    led_on = true;
            rt_pin_write(LED1_PIN, PIN_LOW);
		}else if(buf[6]=='o' && buf[7]=='f' && buf[8]=='f'){	//请求关闭LED
		    led_on = false;
            rt_pin_write(LED1_PIN, PIN_HIGH);
	    }

		httpserver_send_html(conn, led_on);
	}

	netbuf_delete(inbuf);
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
}

/** The main function, never returns! */
static void httpserver_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);

  led_on = true;
  rt_pin_write(LED1_PIN, PIN_LOW);
  
  /* Bind to port 80 (HTTP) with default IP address */
  netconn_bind(conn, NULL, 80);
  
  /* Put the connection into LISTEN state */
  netconn_listen(conn);
  
  do {
    err = netconn_accept(conn, &newconn);
    if (err == ERR_OK) {
      httpserver_serve(newconn);
      netconn_delete(newconn);
    }
  } while(err == ERR_OK);
  LWIP_DEBUGF(HTTPD_DEBUG, ("http_server_netconn_thread: netconn_accept received error %d, shutting down", err));
  netconn_close(conn);
  netconn_delete(conn);
}

/** Initialize the HTTP server (start its thread) */
void httpserver_init()
{
  /* set LED0 pin mode to output */
  rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

  sys_thread_new("http_server_netconn", httpserver_thread, NULL, 1024, TCPIP_THREAD_PRIO + 1);
  rt_kprintf("Startup a tcp web server.\n");
}
MSH_CMD_EXPORT_ALIAS(httpserver_init, seqapi_httpserver, sequential api httpserver init);
