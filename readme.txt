20181010使用stm32f103zet6连接enc28j60测试udp成功
连接方式
INT---PG6
S0----PB14
SCK---PB13
REST--PG8

CLK---空闲
WOL---空闲
SI ---PB15
CS----PG7

使用UDP通行ping网络正常
UDP通信正常
目标192.168.5.175：8808
接收224.0.0.50：9999
发送心跳包不能停止，不然会出现断网现象不能重新连接
