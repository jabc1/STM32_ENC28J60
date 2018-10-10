#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "lwip/igmp.h"

//UDP�������ݻ�����
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ����� 
//UDP������������
const u8 *udp_demo_sendbuf="ENC28J60 UDP demo send data\r\n";

//UDP ����ȫ��״̬��Ǳ���
//bit7:û���õ�
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û��������;1,��������.
//bit4~0:����
extern u8 udp_demo_flag;  //UDP ����ȫ��״̬��Ǳ���


struct udp_pcb *udp_server_pcb;  	//����һ��UDP���������ƿ� 
struct ip_addr ipgroup;  	//Զ��ip��ַ


//����Զ��IP��ַ
void udp_demo_set_remoteip(void)
{
	//ǰ����IP���ֺ�DHCP�õ���IPһ��
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	lwipdev.remoteip[3] = 173;
	printf("Զ��IP:%d.%d.%d.%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
} 
void Init_UDP_Server(void)
{
    IP4_ADDR(&ipgroup, 224,0,0,50);//�鲥IP��ַ  
#if LWIP_IGMP  
    igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipgroup));//�鲥���뵱ǰ  
#endif  
    udp_server_pcb = udp_new();  
    if(udp_server_pcb!=NULL){  
        //udp_bind(udp_server_pcb,IP_ADDR_ANY,1177);//����UDP�˿�  ��������ע�͵�  
          
        udp_bind(udp_server_pcb,IP_ADDR_ANY,9999);//�鲥�˿�  
          
        udp_recv(udp_server_pcb,udp_demo_recv,NULL);//���ջص�����  
    }  
}
void multicast_send_data(unsigned char * data,unsigned short len)  
{  
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);  
    memcpy(p->payload, data, len);
    //udp_sendto(udp_server_pcb, p,(struct ip_addr *) (&ipgroup),9999);  
	udp_sendto(udp_server_pcb, p,(struct ip_addr *)("192.168.5.174"),8080);
    pbuf_free(p);  
}
//UDP����
void udp_demo_test(void)
{
 	err_t err;
	struct udp_pcb *udppcb;  	//����һ��UDP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
 	
	u8 *tbuf;
	u8 res=0;		
	u8 t=0; 
 	
	udp_demo_set_remoteip();//��ѡ��IP

	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	printf("UDP������IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
	
	printf("Ŀ��IP:%d.%d.%d.%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
	printf("Ŀ�Ķ˿ں�:%d\r\n",UDP_DEMO_PORT);//Ŀ�Ķ˶˿ں�
	udppcb=udp_new();
	if(udppcb)//�����ɹ�
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
		err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����
		if(err==ERR_OK)
		{
			err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//�󶨱���IP��ַ��˿ں�
			if(err==ERR_OK)	//�����
			{
				udp_recv(udppcb,udp_demo_recv,NULL);//ע����ջص����� 
				printf("UDP Connected\r\n");
				udp_demo_flag |= 1<<5;			//����Ѿ�������
			}else res=1;
		}else res=1;		
	}else res=1;
	while(res==0)
	{
		if(udp_demo_flag&1<<6)//�Ƿ��յ�����?
		{
			printf("Receive Data:%s\r\n",udp_demo_recvbuf);
			udp_demo_flag&=~(1<<6);//��������Ѿ���������.
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

//UDP�ص�����
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
		}
		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;	//��ǽ��յ�������
		pbuf_free(p);//�ͷ��ڴ�
	}else
	{
		udp_disconnect(upcb);
		udp_demo_flag &= ~(1<<5);	//������ӶϿ�
	} 
} 
//UDP��������������
void udp_demo_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)udp_demo_sendbuf),PBUF_POOL); //�����ڴ�
	if(ptr)
	{
		pbuf_take(ptr,(char*)udp_demo_sendbuf,strlen((char*)udp_demo_sendbuf)); 
		udp_send(upcb,ptr);	//udp�������� 
		pbuf_free(ptr);//�ͷ��ڴ�
	} 
} 
//�ر�UDP����
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//�Ͽ�UDP���� 
	udp_demo_flag &= ~(1<<5);	//������ӶϿ�
}

























