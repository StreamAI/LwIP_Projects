#include "lwip/api.h"
#include "rtthread.h"


//定义最大子任务数量，应与内核支持的最大连接数保持一致
#define MAX_CONN_TASK (MEMP_NUM_NETCONN)
//子任务起始优先级
#define CONN_TASK_PRIO_BASE (TCPIP_THREAD_PRIO + 2)
//子任务堆栈大小
#define CONN_TASK_STACK_SIZE 1024

//定义子任务管理结构
typedef struct conn_task
{
	sys_thread_t tid;						//子任务句柄
	void* data;								//子任务对应的连接数据
}conn_task_t;

//定义所有子任务管理所需要的内存空间
static conn_task_t task[MAX_CONN_TASK];

//子任务函数声明
static void child_task(void *arg);

//初始化子任务管理结构
static void conn_task_init(void)
{
	int i = 0;
	for(i = 0; i < MAX_CONN_TASK; i++)
	{
		task[i].tid = RT_NULL;
		task[i].data = NULL;
	}
}

//创建子任务，创建成功函数返回0，否则返回-1；data为任务创建时用户的传入参数netconn指针
static int conn_task_create(void * data)
{
	int i = 0;
    sys_thread_t tid = RT_NULL;

	for(i = 0; i < MAX_CONN_TASK; i++)
	{
		if(task[i].tid == RT_NULL)
			break;
	}
	if(MAX_CONN_TASK == i)
	{
		return -1;
	}

    tid = sys_thread_new("child task", child_task, (void *)data, \
                        CONN_TASK_STACK_SIZE, CONN_TASK_PRIO_BASE + i);
	if(tid != RT_NULL)
	{
		task[i].tid = tid;
		task[i].data = data;
		return 0;
	}
	return -1;
}

//该函数基于用户数据data，查找与之匹配的子任务，并返回子任务的优先级
static sys_thread_t conn_task_find(void * data)
{
	sys_thread_t tid = RT_NULL;
	int i = 0;
	
	for(i = 0; i < MAX_CONN_TASK; i++)
	{
		if(task[i].data == data && task[i].tid != RT_NULL)
		{
			tid = task[i].tid;
			break;
		}
	}
	return tid;
}

//该函数删除当前任务并回收其管理结构
static void conn_task_del(sys_thread_t tid)
{
	int i = 0;
	
	for(i = 0; i < MAX_CONN_TASK; i++)
	{
		if(task[i].tid == tid)
		{
			task[i].tid = RT_NULL;
			task[i].data = NULL;
			break;
		}
	}
	rt_thread_delete(tid);
}

//服务器主任务函数
static void tcpecho_thread(void * arg)
{
	struct netconn *conn, *newconn;
	err_t err;
	conn_task_init();

	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 7);
	netconn_listen(conn);

	while(1){
		err = netconn_accept(conn, &newconn);
		if(err == ERR_OK){
			if(conn_task_create((void *)newconn) < 0){
				rt_kprintf("Create new child task failed\n");
				netconn_close(newconn);
				netconn_delete(newconn);
			}
		}else{
			rt_kprintf("Server accept error\n");
			netconn_close(conn);
			netconn_delete(conn);
			while(1);
		}
	}
}

//服务器子任务函数
static void child_task(void * arg)
{
	struct netconn *conn = (struct netconn *)arg;
	err_t err;
	struct netbuf *buf;
	void *data;
	u16_t len;
	sys_thread_t tid = conn_task_find(arg);

	while((err = netconn_recv(conn, &buf)) == ERR_OK){
		do{
			netbuf_data(buf, &data, &len);
			err = netconn_write(conn, data, len, NETCONN_COPY);
		}while(netbuf_next(buf) >= 0);
		
		netbuf_delete(buf);
	}
	netconn_close(conn);
	netconn_delete(conn);
	conn_task_del(tid);
}

//服务器初始化函数
static void tcpecho_init(void)
{
	sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, \
					CONN_TASK_STACK_SIZE, TCPIP_THREAD_PRIO + 1);
    rt_kprintf("Startup a tcp echo server.\n");
}
MSH_CMD_EXPORT_ALIAS(tcpecho_init, seqapi_tcpecho, sequential api tcpecho init);
