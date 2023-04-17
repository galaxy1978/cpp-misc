#include <unistd.h>
#include <memory.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/select.h>

#include <thread>
#include <iostream>

#include "mallocSharedPtr.hpp"
#include "misc.hpp"
#include "details/linuxSerial.hpp"
using namespace wheels;

const static int BUFFER_LEN = 1024;

linuxSerial :: linuxSerial( const std::string& name ):m_is_run( 4 )
{
	m_is_open = false;
	m_name    = name;
}

linuxSerial :: linuxSerial( const std::string& name ,  int baud , int char_len , stop::value s , parity::value p , flow::value f ):
	m_is_run( 4 )
{
	
	m_is_open = false;
	m_baud = baud;
	m_char_len = char_len;
	m_stop = s;
	m_parity = p;
	m_flow = f;
	m_name = name;

	// 检查驱动是否正常，串口文件是否存在
	if( access( name.c_str() , F_OK ) == 0 ){
		open( baud , char_len , s , p , f );
		if( m_error != OK ){
			throw m_error.load();
		}
	}else{
		m_error = ERR_DEV_NOT_EXIST;
		throw m_error.load();
	}
}

linuxSerial :: ~linuxSerial()
{
	if( m_is_open )
		close();
}

linuxSerial :: err_code
linuxSerial :: open( int baud , int char_len , stop::value s , parity::value p , flow::value f )
{
	if( m_is_run.load() != 4 ){
		ERROR_MSG( "窗口已经打开" );
		return ERR_ALREADY_OPENED;
	}

	m_baud     = baud;
	m_char_len = char_len;
	m_stop     = s;
	m_parity   = p;
	m_flow     = f;
	
	int e = ::open( m_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY );
        
	if( e >= 0 ){
		m_fd = e;
		m_error = OK;
		m_is_run = 0;
		OnOpen( m_fd );
		MSG_1("打开串口%s成功" , OK_MSG , m_name.c_str());
	}else{
		m_error= ERR_OPEN_SERIAL;
		ERROR_MSG( "打开串口错误" );
		throw m_error.load();
	}

    	return m_error.load();
}

void linuxSerial :: SetBufferSize( size_t l )
{

}


linuxSerial :: err_code
linuxSerial :: reset()
{
	m_is_open = false;

	int e = 0;
		
	if( m_is_run == 1 ){
		m_is_run = 2;
		while( m_is_run.load() != 3 ){
			e ++;
			if( e > 1000 ){
				ERROR_MSG( "Stop read thread fail." );
			}
			usleep( 1000 );
		}
	}
	if( m_is_run != 4 ){
		e = ::close( m_fd );
		if( e < 0 ){
			ERROR_MSG( "Close serial port fail." );
			m_error = ERR_CLOSE_UART;
		}else{
			m_is_run = 4;
			m_fd = -1;
			OnClose( OK );
		}
	}
	
	m_error = open( m_baud , m_char_len , m_stop , m_parity , m_flow );
	return m_error;
}

linuxSerial :: err_code
linuxSerial :: reset( int baud , int char_len , stop::value s , parity::value p , flow::value f )
{
    	int e = 1;
	m_is_open = false;
	if( m_is_run == 1 ){
		m_is_run = 2;
		while( m_is_run.load() != 3 ){
			e ++;
			if( e > 1000 ){
				__show_line( "Stop read thread fail." );
				return ERR_STOP_READ_THD;
			}
			usleep( 1000 );
		}
	}

	if( m_is_run != 4 ){
		::close( m_fd );
		if( e < 0 ){
			__show_line( "Close serial port fail." );
			m_error = ERR_CLOSE_UART;
		}else{
			m_is_run = 4;
			m_fd = -1;
			OnClose( OK );
		}
	}
	m_baud      = baud;
	m_char_len  = char_len;
	m_stop      = s;
	m_parity    = p;
	m_flow      = f;

	m_error = open( baud , char_len , s , p , f );

	return m_error.load();
}

void linuxSerial :: init_device( int fd , int baud , int char_len , stop::value s ,  parity::value p , flow::value f )
{
	updt_param( fd , baud , char_len , s , p , f );
}

void linuxSerial :: OnOpen( int fd )
{
	updt_param( m_fd, m_baud, m_char_len , m_stop, m_parity, m_flow );
	if( m_error == OK ){
		m_is_open = true;
		m_is_run = 0;
	}
	if( on_open ){
		on_open( OK );
	}
	run( true );
}

void linuxSerial::OnRecv( const char * data , size_t len )
{
	if( data == NULL ) return;
	on_recv( len , data , m_error );
}

void linuxSerial::OnClose( err_code e )
{
	if( e == OK ){
		m_is_open = false;
	}
}

linuxSerial::err_code
linuxSerial::send( size_t len , const char* data )
{
	if( data != nullptr && len > 0 && m_is_run == 1){
		write( m_fd , data , len );
		m_error = OK;
	}else{
		m_error = ERR_SEND_DATA_EMPTY;
	}
    	return m_error.load();
}

linuxSerial::err_code
linuxSerial::close()
{
    	int e = 0;
	if( m_is_run == 4 ){
		m_is_run = 2;
		while( m_is_run.load() != 3 ){
			usleep( 1000 );
			e ++;
			if( e > 1000 ){
				__show_line( "Wait for read thread stop fail." );
				m_error =  ERR_STOP_READ_THD;
				return m_error.load();
			}
		}
	}

	e = ::close( m_fd );
	if( e >= 0 ){
		m_fd = -1;
		m_is_run = 4;
		m_error = OK;
	}else{
		__show_line( "Close serial port fail." );
	}
	
	return m_error.load();
}

bool linuxSerial::__is_ready()
{
	bool ret = 0;
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 0;

	fd_set  fds;
	FD_ZERO( &fds );
	FD_SET( m_fd , &fds );

	int rc = select( m_fd + 1 , &fds , NULL , NULL , &t );
	if( rc < 0 ){
		fprintf( stderr , "%s [ %d ] check port error. \n" , __FILE__ , __LINE__ );
		return ret;
	}

	ret = ( FD_ISSET( m_fd , &fds ) ? true : false );
	return ret;
}

void linuxSerial::run( bool run )
{
	if( m_is_run != 0 ){
		ERROR_MSG( "串口没有打开" );
		return;
	}
	if( run == false ) return;

	m_is_run = 1;
       
	std::thread thd([&]{
		char * buf = nullptr;
		mallocSharedPtr< char >  ptBuff( BUFFER_LEN );
		buf = ptBuff.get();
		if( buf == nullptr ){
			ERROR_MSG( "内存分配失败" );
			return;
		}
		while( m_is_run.load() == 1 ){
		       if( __is_ready() == true ){
			       size_t len = read( m_fd ,buf , 1024 );
			       if( len > 0 ){
				       OnRecv( buf , len );
			       }
		       }else{
			       usleep( 10000 );
		       }
	       }

	       free( buf );
	});
	thd.detach();
}

void linuxSerial :: standby()
{
	if( m_is_run == true )
		m_is_run = false;
}

bool linuxSerial::updt_param( int fd , int baud , int char_len , stop::value s , parity::value p , flow::value f )
{
	bool ret = false;
	if( fd > 0 ){
		struct termios newtio,oldtio;
		/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
		if  ( tcgetattr( fd,&oldtio)  !=  0) {
			ret = false;
		}
		bzero( &newtio, sizeof( newtio ) );
		/*步骤一，设置字符大小*/
		newtio.c_cflag  |=  CLOCAL | CREAD;
		newtio.c_cflag &= ~CSIZE;
		switch( char_len ){
		case 5:  newtio.c_cflag |= CS5;  break;
		case 6:  newtio.c_cflag |= CS6;  break;
		case 7:  newtio.c_cflag |= CS7;  break;
		case 8:  default: newtio.c_cflag |= CS8;  break;
		}
		/*设置奇偶校验位*/
		switch( p ) {
		case parity::ODD:
			newtio.c_cflag |= PARENB; newtio.c_iflag |= INPCK; newtio.c_cflag |= PARODD; break;
		case parity::EVEN:
			newtio.c_cflag |= PARENB; newtio.c_iflag |= INPCK; newtio.c_cflag &= ~PARODD; break;
		case parity::NONE:
		default:
			newtio.c_cflag &= ~PARENB;          break;
		}
		/*设置波特率*/
		switch( baud ) {
		case 2400:
			cfsetispeed(&newtio, B2400);           cfsetospeed(&newtio, B2400);            break;
		case 4800:
			cfsetispeed(&newtio, B4800);           cfsetospeed(&newtio, B4800);            break;
		case 9600:
			cfsetispeed(&newtio, B9600);           cfsetospeed(&newtio, B9600);            break;
		case 19200:
			cfsetispeed(&newtio, B19200);          cfsetospeed(&newtio, B19200);           break;
		case 38400:
			cfsetispeed(&newtio, B38400);          cfsetospeed(&newtio, B38400);           break;
		case 57600:
			cfsetispeed(&newtio, B57600);          cfsetospeed(&newtio, B57600);           break;
		case 115200:
			cfsetispeed(&newtio, B115200);         cfsetospeed(&newtio, B115200);          break;
		case 230400:
			cfsetispeed(&newtio, B230400);         cfsetospeed(&newtio, B230400);          break;
		case 460800:
			cfsetispeed(&newtio, B460800);         cfsetospeed(&newtio, B460800);          break;
		case 921600:
			cfsetispeed(&newtio, B921600);         cfsetospeed(&newtio, B921600);          break;
		case 3000000:
			cfsetispeed(&newtio, B3000000);        cfsetospeed(&newtio, B3000000);         break;
		default:
			cfsetispeed(&newtio, B115200);         cfsetospeed(&newtio, B115200);          break;
		}

		if( s == stop::ONE )                    /*设置停止位*/
			newtio.c_cflag &=  ~CSTOPB;
		else if ( s == stop::TWO )
			newtio.c_cflag |=  CSTOPB;

		newtio.c_cc[VTIME]  = 0;                /*设置等待时间和最小接收字符*/
		newtio.c_cc[VMIN] = 0;

		tcflush( fd,TCIFLUSH );                       /*处理未接收字符*/

		if((tcsetattr( fd,TCSANOW,&newtio))!=0){     /*激活新配置*/
			ret = false;
		}
		else ret = true;
	}

	return ret;
}


