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
	delay_init();	    	 	//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 		//串口初始化为115200
	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	 
	TIM3_Int_Init(1000,719);	//定时器3频率为100hz
	usmart_dev.init(72);		//初始化USMART	
	my_mem_init(SRAMIN);		//初始化内部内存池
 	
	while(lwip_comm_init()) //lwip初始化
	{
		printf("LWIP Init Falied!\r\n");
		delay_ms(2000);
	}
	printf("LWIP Init Success!\r\n");
	printf("DHCP IP configing...\r\n");
#if LWIP_DHCP   //使用DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
#endif
	//udp_demo_test();  		//UDP 模式
	Init_UDP_Server();
	printf("system start\r\n");
 	while(1)
	{
		lwip_periodic_handle();
		if(udp_demo_flag&1<<6)//是否收到数据?
		{
			printf("Receive Data:%s\r\n",udp_demo_recvbuf);
			udp_demo_flag&=~(1<<6);//标记数据已经被处理了.
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


