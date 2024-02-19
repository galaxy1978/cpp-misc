#include <sys/epoll.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <sstream>

#include "misc.hpp"
#include "mallocSharedPtr.hpp"

#include "config.hpp"
#include "details/tcplinux.hpp"

using namespace wheels;

tcpLinux :: tcpLinux( const std::string& url , uint16_t port ):
	m_server__( url ),
	m_port__(0),
	m_epoll_fd__( -1 ),
	m_tcp_fd__( -1 ),
	m_is_running__( false ),
	m_max__( EPOLL_MAX_EVTS )
{
	// 创建epool句柄
	m_epoll_fd__ = epoll_create1( 0 );
	if( m_epoll_fd__.load() == -1 ){
		std::stringstream ss;
		ss << "创建EPOLL句柄失败:" << strerror( errno );
		throw std::runtime_error( ss.str() );
	}
	
    // 创建tcp套接字
	m_tcp_fd__ = socket( AF_INET , SOCK_STREAM , 0 );
	fcntl( m_tcp_fd__.load() , F_SETFL , O_NONBLOCK );
	// 关联套接字
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = m_tcp_fd__.load();
	// 配置epoll，添加tcp socket
	int rst = epoll_ctl( m_epoll_fd__.load() , EPOLL_CTL_ADD , ev.data.fd , &ev );
	if( rst == -1 ){
		::close( m_tcp_fd__.load() );
		::close( m_epoll_fd__.load() );
		
		throw std::runtime_error( "关联TCP套接字失败" );
	}
}

tcpLinux :: ~tcpLinux()
{
	this->close();
}

bool tcpLinux :: connect()
{
	bool ret = true;
    // 执行连接操作
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	std::stringstream ss;
	ss << m_port__.load();
	struct addrinfo *res , * tmp = nullptr ;
	// 域名解析
	int status = getaddrinfo( m_server__.c_str() , ss.str().c_str() , &hints, &res );
	if (status != 0) {
		ERROR_MSG( "解析地址失败" );
		return false;
	}
	tmp = res;
	// 针对每一个的解析结果尝试执行连接操作
	for( ; tmp != nullptr; tmp = tmp->ai_next){
		struct sockaddr *addr;
		if (res->ai_family == AF_INET) {  // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)tmp->ai_addr;
			addr = (struct sockaddr *)ipv4;
			// 调用系统connect函数尝试连接，这个分支控制IPv4，另外一个控制IPv6
			status = ::connect( m_tcp_fd__.load() , addr , sizeof( struct sockaddr_in ) );
			
		} else {  // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)tmp->ai_addr;
			addr = (struct sockaddr *)ipv6;

			status = ::connect( m_tcp_fd__.load() , addr , sizeof( struct sockaddr_in ) );
		}

		if( status < 0 ){
			ERROR_MSG( "连接服务器失败" );
			return false;
		}
	}

	freeaddrinfo(res);
	return ret;
}
// 也是连接接口，调用了上面的
bool tcpLinux :: connect( const std::string& url , uint16_t port )
{
	// 这里将参数保存在类中
	m_server__ = url;
	m_port__ = port;

	return this->connect();
}

size_t tcpLinux :: send( const uint8_t * data , size_t len )
{
	// 在ssl中会使用openssl的函数，这里不用实现
	return 0;
}

void tcpLinux :: close()
{
	m_is_running__ = false;
}

void tcpLinux :: backend__()
{
	// epoll主事件循环处理函数
	// 在linux下std::thread会使用pthread实现。配置pthread参数，方便随时取消
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	while ( m_is_running__.load() ) {
		// 等候epoll事件
		int nfds = epoll_wait( m_epoll_fd__.load() , pt_events__ , m_max__.load(), 100 );
		if (nfds <= 0 ) {
			if( errno == EINTR || nfds == 0 ){
				continue;
			}
			
			std::stringstream ss;
			ss << "epoll发生错误:" << strerror( errno );
			ERROR_MSG( ss.str() );
			m_is_running__ = false;
			return;
		}

		epoll_event * events = pt_events__;
		for (int i = 0; i < nfds; i++) { // 发出通知，针对每一个发生事件的fd进行处理
			int fd = events[ i ].data.fd; 
			// 通过回调函数调用读取数据的操作处理
			m_cb_read_data__( fd );
		}
	}
}

// 这个函数用来启动epoll。
void tcpLinux :: run( bool sw )
{
	if( sw ){ // sw == true， 将backend__启动为线程
		if( m_is_running__.load() ) return;
		m_is_running__ = true;
		
        MSG( "启动EPOLL驱动引擎" , OK_MSG );
		std::thread thd([=]{
			backend__();
		});
		thd.detach();
	}else{  // 结束线程运行。
		if( !m_is_running__.load() ) return;
		m_is_running__ = false;
	}
}

// 下来看sslSocket的实现