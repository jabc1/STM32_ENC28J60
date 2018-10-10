#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "usmart.h"
#include "sram.h"
#include "malloc.h"
#include "enc28j60.h" 	 
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "timer.h"
#include "udp_demo.h"


extern u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];
u8 udp_demo_flag;
void multicast_send_data(unsigned char * data,unsigned short len);
u8 udp_senddata[] = "25222222221365464\r\n";
void Init_UDP_Server(void);
 int main(void)
{	 
	u32 i;
	delay_init();	    	 	//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 		//���ڳ�ʼ��Ϊ115200
	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	 
	TIM3_Int_Init(1000,719);	//��ʱ��3Ƶ��Ϊ100hz
	usmart_dev.init(72);		//��ʼ��USMART	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
 	
	while(lwip_comm_init()) //lwip��ʼ��
	{
		printf("LWIP Init Falied!\r\n");
		delay_ms(2000);
	}
	printf("LWIP Init Success!\r\n");
	printf("DHCP IP configing...\r\n");
#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
	}
#endif
	//udp_demo_test();  		//UDP ģʽ
	Init_UDP_Server();
	printf("system start\r\n");
 	while(1)
	{
		lwip_periodic_handle();
		if(udp_demo_flag&1<<6)//�Ƿ��յ�����?
		{
			printf("Receive Data:%s\r\n",udp_demo_recvbuf);
			udp_demo_flag&=~(1<<6);//��������Ѿ���������.
		}
		delay_ms(10);
		i++;
		if(i==50)
		{
			i=0;
			multicast_send_data(udp_senddata,19);
			//printf("%s\r\n",udp_senddata);
			LED0=!LED0;
		}
	}
}


