#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "lwip/igmp.h"

//UDP接收数据缓冲区
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区 
//UDP发送数据内容
const u8 *udp_demo_sendbuf="ENC28J60 UDP demo send data\r\n";

//UDP 测试全局状态标记变量
//bit7:没有用到
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上;1,连接上了.
//bit4~0:保留
extern u8 udp_demo_flag;  //UDP 测试全局状态标记变量


struct udp_pcb *udp_server_pcb;  	//定义一个UDP服务器控制块 
struct ip_addr ipgroup;  	//远端ip地址


//设置远端IP地址
void udp_demo_set_remoteip(void)
{
	//前三个IP保持和DHCP得到的IP一致
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	lwipdev.remoteip[3] = 173;
	printf("远端IP:%d.%d.%d.%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
} 
void Init_UDP_Server(void)
{
    IP4_ADDR(&ipgroup, 224,0,0,50);//组播IP地址  
#if LWIP_IGMP  
    igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipgroup));//组播加入当前  
#endif  
    udp_server_pcb = udp_new();  
    if(udp_server_pcb!=NULL){  
        //udp_bind(udp_server_pcb,IP_ADDR_ANY,1177);//本地UDP端口  此行无用注释掉  
          
        udp_bind(udp_server_pcb,IP_ADDR_ANY,9999);//组播端口  
          
        udp_recv(udp_server_pcb,udp_demo_recv,NULL);//接收回调函数  
    }  
}
void multicast_send_data(unsigned char * data,unsigned short len)  
{  
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);  
    memcpy(p->payload, data, len);
	IP4_ADDR(&ipgroup, 192,168,5,175);
    udp_sendto(udp_server_pcb, p,(struct ip_addr *) (&ipgroup),8808);  
    pbuf_free(p);  
}
//UDP测试
void udp_demo_test(void)
{
 	err_t err;
	struct udp_pcb *udppcb;  	//定义一个UDP服务器控制块
	struct ip_addr rmtipaddr;  	//远端ip地址
 	
	u8 *tbuf;
	u8 res=0;		
	u8 t=0; 
 	
	udp_demo_set_remoteip();//先选择IP

	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL)return ;		//内存申请失败了,直接退出
	printf("UDP服务器IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
	
	printf("目的IP:%d.%d.%d.%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
	printf("目的端口号:%d\r\n",UDP_DEMO_PORT);//目的端端口号
	udppcb=udp_new();
	if(udppcb)//创建成功
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
		err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP客户端连接到指定IP地址和端口号的服务器
		if(err==ERR_OK)
		{
			err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//绑定本地IP地址与端口号
			if(err==ERR_OK)	//绑定完成
			{
				udp_recv(udppcb,udp_demo_recv,NULL);//注册接收回调函数 
				printf("UDP Connected\r\n");
				udp_demo_flag |= 1<<5;			//标记已经连接上
			}else res=1;
		}else res=1;		
	}else res=1;
	while(res==0)
	{
		if(udp_demo_flag&1<<6)//是否收到数据?
		{
			printf("Receive Data:%s\r\n",udp_demo_recvbuf);
			udp_demo_flag&=~(1<<6);//标记数据已经被处理了.
		} 
		lwip_periodic_handle();
		delay_ms(10);
		t++;
		if(t==200)
		{
			t=0;
			udp_demo_senddata(udppcb);
			LED0=!LED0;
		}
	}
	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 

//UDP回调函数
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//接收到不为空的数据时
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
		for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
		{
			//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
			//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
		}
		upcb->remote_ip=*addr; 				//记录远程主机的IP地址
		upcb->remote_port=port;  			//记录远程主机的端口号
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;	//标记接收到数据了
		pbuf_free(p);//释放内存
	}else
	{
		udp_disconnect(upcb);
		udp_demo_flag &= ~(1<<5);	//标记连接断开
	} 
} 
//UDP服务器发送数据
void udp_demo_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)udp_demo_sendbuf),PBUF_POOL); //申请内存
	if(ptr)
	{
		pbuf_take(ptr,(char*)udp_demo_sendbuf,strlen((char*)udp_demo_sendbuf)); 
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
	} 
} 
//关闭UDP连接
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//断开UDP连接 
	udp_demo_flag &= ~(1<<5);	//标记连接断开
}

























