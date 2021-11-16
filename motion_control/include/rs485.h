#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <thread>
#include <pthread.h>
//#include "rs485.hpp"

struct WeightOpt {
	int value1;
	int value2;
	int online1;
	int online2;
	int value;
	int tension;
	int repo;
	int yuliao;
};


enum RS485 {
	RS485_1 = 0,
	RS485_2 = 1,
	RS485_3 = 2,
	RS485_END = 3,
};

struct UartOpt_t {
	int nSpeed;
	int nBits;
	char nEvent;
	int nStop;
};


int RS485_Init(enum RS485 dev, struct UartOpt_t* config);
int RS485_SendData(enum RS485 dev, unsigned char* Data, unsigned int Len);
int RS485_RecvData(enum RS485 dev, unsigned char* Data, unsigned int Len);

#define RS485_NUM 3
enum RS485_IO {
	RS485_IN = 0,
	RS485_OUT = 1,
	RS485_IO_END = 2,
};

int fd[RS485_NUM] = { 0 };
const char* RS485Path[RS485_NUM] = { "/dev/ttyTHS2", "/dev/ttyTHS1", "/dev/ttyTHS3" };
const char* RS485Out[RS485_NUM] = { "echo 1 > /sys/class/gpio/gpio302/value",
									"echo 1 > /sys/class/gpio/gpio299/value",
									"echo 1 > /sys/class/gpio/gpio300/value" };
const char* RS485In[RS485_NUM] = { "echo 0 > /sys/class/gpio/gpio302/value",
									"echo 0 > /sys/class/gpio/gpio299/value",
									"echo 0 > /sys/class/gpio/gpio300/value" };


WeightOpt weight;

int UartOptCfg(int fd, struct UartOpt_t* pUartOpt)
{
	struct termios newtio, oldtio;

	memset(&newtio, 0, sizeof(struct termios));
	memset(&oldtio, 0, sizeof(struct termios));
	/* ��ȡԭ�д������� */
	if (tcgetattr(fd, &oldtio) != 0) {
		printf("com tcgetattr()\n");
		return -1;
	}
	/* CREAD �����������ݽ��գ�CLOCAL���򿪱�������ģʽ */
	newtio.c_cflag |= CLOCAL | CREAD;

	/* ��������λ */
	newtio.c_cflag &= ~CSIZE;
	switch (pUartOpt->nBits) {
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}
	/* ������żУ��λ */
	switch (pUartOpt->nEvent) {
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}

	/* ���ò����� */
	switch (pUartOpt->nSpeed) {
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 38400:
		cfsetispeed(&newtio, B38400);
		cfsetospeed(&newtio, B38400);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}

	/* ����ֹͣλ */
	if (pUartOpt->nStop == 1) {      // ��ֹͣλΪ1�������CSTOPB����ֹͣλΪ2���򼤻�CSTOPB
		newtio.c_cflag &= ~CSTOPB;   // Ĭ��Ϊһλֹͣλ
	}
	else if (pUartOpt->nStop == 2) {
		newtio.c_cflag |= CSTOPB;
	}

	/* ���������ַ��͵ȴ�ʱ�䣬���ڽ����ַ��͵ȴ�ʱ��û���ر��Ҫ��ʱ */
	newtio.c_cc[VTIME] = 0;  // �ǹ淶ģʽ��ȡʱ�ĳ�ʱʱ��
	newtio.c_cc[VMIN] = 0;    // �ǹ淶ģʽ��ȡʱ����С�ַ���

	/* tcflush����ն�δ��ɵ�����/����������ݣ�TCIFLUSH��ʾ������յ������ݣ��Ҳ���ȡ���� */
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
		printf("com tcsetattr()\n");
		return -2;
	}

	return 0;
}

//Ĭ��RS485_IN
int RS485_Direction(enum RS485 dev, enum RS485_IO direct)
{
	if ((dev >= RS485_END) || (direct >= RS485_IO_END)) {
		printf("RS485 direction parameter error\n");
		return -1;
	}
	if (fd[dev] == 0) {
		printf("RS485 device is not open\n");
		return -2;
	}

	if (direct == RS485_OUT) {
		int i = system(RS485Out[dev]);
	}
	else if (direct == RS485_IN) {
		int j = system(RS485In[dev]);
	}
	return 0;
}

int RS485_Init(enum RS485 dev, struct UartOpt_t* config)
{
	int bRet;

	if (dev >= RS485_END) {
		printf("RS485 init parameter error\n");
		return -1;
	}

	if (fd[dev] != 0) {
		printf("RS485 device occupancy\n");
		return -2;
	}

	fd[dev] = open((const char*)RS485Path[dev], O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd[dev] < 0)
	{
		fd[dev] = 0;
		perror((const char*)RS485Path[dev]);
		return -3;
	}

	bRet = UartOptCfg(fd[dev], config);
	RS485_Direction(dev, RS485_IN);
	return bRet;
}

int RS485_SendData(enum RS485 dev, unsigned char* Data, unsigned int Len)
{
	int ret = 0;
	int count = 0;

	if ((dev >= RS485_END) || (Data == NULL) || (Len == 0)) {
		printf("RS485 send parameter error\n");
		return -1;
	}
	if (fd[dev] == 0) {
		printf("RS485 is not open,so send message failed\n");
		return -2;
	}

	tcflush(fd[dev], TCIFLUSH);
	int i = system("echo 1 > /sys/class/gpio/gpio299/value");
	while (Len > 0)
	{
		ret = (int)(write(fd[dev], (char*)(Data + count), Len));
		if (ret < 1) {
			break;
		}
		count += ret;
		Len = Len - ret;
	}
	int j = system("echo 0 > /sys/class/gpio/gpio299/value");
	return count;
}

int RS485_RecvData(enum RS485 dev, unsigned char* Data, unsigned int Len)
{
	int ret = 0;
	fd_set rd;
	struct timeval tv;

	if ((dev >= RS485_END) || (Data == NULL) || (Len == 0)) {
		printf("RS485 recv parameter error\n");
		return -1;
	}
	if (fd[dev] == 0) {
		printf("RS485 is not open, so recv message failed\n");
		return -2;
	}

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	FD_ZERO(&rd);
	FD_SET(fd[dev], &rd);
	ret = select(fd[dev] + 1, &rd, NULL, NULL, &tv);
	if (ret == 0) //��ʱ
	{
		printf("RS485 read time out\n");
	}
	else if (ret < 0) //ʧ��
	{
		printf("rs485 select error\n");
	}
	else //�ɹ�
	{
		ret = (int)(read(fd[dev], (char*)Data, Len));
		if (ret < 0) {
			printf("RS485 read error ocuured\n");
		}
	}

	return ret;
}



int getWeight(char* buffer, int len)
{
	int weight = 0;
	char bbb[6];
	for (int i = 0; i < 6; i++)
	{
		bbb[i] = buffer[6 - i];

	}
	int b = atoi(bbb);
	if (buffer[0] == 0x3D && len == 8)
	{
		weight = buffer[7] == 0x30 ? b : -b;

	}
	return weight;
}


void TaskUart3()
{
	while (true)
	{
		unsigned char buf[256];
		memset(buf, 0, 256);
		int len = RS485_RecvData(RS485_3, buf, 256);
		if (len > 0)
		{
			weight.value1 = getWeight((char*)buf, len);
			weight.value1 = weight.value1 > 0 ? weight.value1 : 0;
			weight.online1 = 1;
			//printf("UART3 %d\r\n", weight.value1);
		}
		else
		{
			weight.online1 = 0;
			//printf("online1 \r\n");
			//break;
		}
		usleep(10000);
	}
}
void TaskUart2()
{
	while (true)
	{
		unsigned char buf[256];
		memset(buf, 0, 256);
		int len = RS485_RecvData(RS485_2, buf, 256);
		if (len > 0)
		{
			weight.value2 = getWeight((char*)buf, len);
			weight.value2 = weight.value2 > 0 ? weight.value2 : 0;
			weight.online2 = 1;
			//printf("UART2 %d\r\n", weight.value2);
		}
		else
		{
			weight.online2 = 0;
			//printf("online2 \r\n");
			//break;
		}
		usleep(10000);
	}
}

void WeightInit()
{
	int ret = 0;
	struct UartOpt_t rs485Config;
	memset(&rs485Config, 0, sizeof(struct UartOpt_t));
	rs485Config.nSpeed = 9600;
	rs485Config.nBits = 8;
	rs485Config.nEvent = 'N';
	rs485Config.nStop = 1;

	ret = RS485_Init(RS485_2, &rs485Config);
	if (ret != 0)
	{
		printf("RS495_2 Init failed:%d\n", ret);
	}
	else
	{
		std::thread taskUart2(TaskUart2);
		taskUart2.detach();
	}

	ret = RS485_Init(RS485_3, &rs485Config);
	if (ret != 0)
	{
		printf("RS495_3 Init failed:%d\n", ret);
	}
	else
	{
		std::thread taskUart3(TaskUart3);
		taskUart3.detach();
	}


}



