/* monitor the /mnt/external_sd/rk_lcd_parameters.   */
/* if the parameters has been changed, and then update the lcdparamers.  */
/* the parameters will work after reboot. */
/* lxt@rock-chips.com */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static int m_dev = -1;

#define false 0
#define UART_9BIT_ON

struct termios opts;
//设置波特率：
static void set_baudrate(struct termios *set_serial, unsigned long int baud_rate)
{
    int baud = B115200;
    switch(baud_rate)
    {
        case 2400:
            baud = B2400;
            break;
        case 4800:
            baud = B4800;
            break;
        case 9600:
            baud = B9600;
            break;
        case 19200:
            baud = B19200;
            break;
        case 38400:
            baud = B38400;
            break;
        case 57600:
            baud = B57600;
            break;
        case 115200:
            baud = B115200;
            break;
        default:
            baud = B115200;
            break;
    }
    cfsetispeed(set_serial, baud);
    cfsetospeed(set_serial, baud);
}

//设置数据位：
static void set_databits(struct termios *set_serial, unsigned int data_bits)
{
    switch(data_bits)
    {
        case 5:
            set_serial->c_cflag |= CS5;
            break;
        case 6:
            set_serial->c_cflag |= CS6;
            break;
        case 7:
            set_serial->c_cflag |= CS7;
            break;
        case 8:
            set_serial->c_cflag |= CS8;
            break;
        default:
            set_serial->c_cflag |= CS8;
            break;
    }
}

//设置校验位：
static void set_parity(struct termios *set_serial, char parity)
{
    switch(parity)
    {
        case 'N':
            set_serial->c_cflag &= ~PARENB;     //no parity check
            break;
        case 'O':
            set_serial->c_cflag |= PARENB;      //odd check
            set_serial->c_cflag &= ~PARODD;
            break;
        case 'E':
            set_serial->c_cflag |= PARENB;      //even check
            set_serial->c_cflag |= PARODD;
            break;
        default:
            set_serial->c_cflag &= ~PARENB;
            break;
    }
}

//停止位：
static void set_stopbits(struct termios *set_serial, unsigned int stop_bits)
{
    if(stop_bits == 2)
    {
        set_serial->c_cflag |= CSTOPB;  //2 stop bits
    }
    else
    {
        set_serial->c_cflag &= ~CSTOPB; //1 stop bits
    }
}
//串口配置函数：
static void set_option(unsigned int baud_rate, unsigned int data_bits, char parity, unsigned int stop_bits)
{
    //struct termios opts;
    tcgetattr(m_dev, &opts);


    set_baudrate(&opts, baud_rate);
    opts.c_cflag |= CLOCAL|CREAD;

    set_parity(&opts, parity);
    
    set_stopbits(&opts, stop_bits);
    
    set_databits(&opts, data_bits);


    opts.c_cflag &= ~CRTSCTS;


    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
    opts.c_oflag &= ~OPOST; // raw output


    opts.c_cc[VTIME]=1;
    opts.c_cc[VMIN]=1023;

    tcsetattr(m_dev, TCSANOW, &opts);
}

//发送数据：
static int send_data(int fd, const char *data, int datalen)
{
    int len = 0;
    len = write(m_dev, data, datalen);
    if(len == datalen)
    {
        return 0;
    }
    else
    {
        tcflush(m_dev, TCOFLUSH);
        return -1;
    }
}

//接收函数：
static int receive(int fd, char *data, int datalen)
{
    int read_len;
    if((read_len = read(m_dev, data, datalen)) > 0)
    {
        return read_len;
    }
    else
    {
        return -1;
    }
}
int main( int argc, char *argv[])
{
    int fd;
    int ret;
    char buff[1024];
    char senddata[] = "uart";
    char byte = 0x69;
    char is_addr = false;
    
	printf(" enter uart test \n");
	
    fd= open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd <= 0)
    {
        printf("uart open fail\n");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);

    m_dev = fd;

    set_option(115200, 8, 'N', 1);

    while (1)
    {
#ifdef UART_9BIT_ON
#if 1
		opts.c_cflag |= PARENB;
	    if (is_addr == false) {
         	opts.c_cflag &= ~PARODD;
    	} else {
        	opts.c_cflag |= PARODD;
    	}
	    tcsetattr(m_dev, TCSANOW, &opts);

	   send_data(fd, senddata, sizeof(senddata));
#endif// endif 0
#if 0
		struct termios tmp_opts;
		tcgetattr(m_dev, &tmp_opts);

		printf(" tmp_opts.c_cflag:%x \n",tmp_opts.c_cflag);

		ret = receive(fd, buff, sizeof(buff));
        buff[ret] = 0;
#endif// endif 0
#else
    
       send_data(fd, senddata, sizeof(senddata));
    
       ret = receive(fd, buff, sizeof(buff));
       buff[ret] = 0;
       printf(" receive data : %s \n", buff);
#endif   
		  usleep(1000000);
    }    
    close(fd);

    
	printf(" exit uart test \n");
	
    return 0;
}


