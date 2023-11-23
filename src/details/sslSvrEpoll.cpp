#include "mallocSharedPtr.hpp"
#include "designM/command.hpp"

#include "sslSvrEpoll.hpp"
#include "sslEpollEgn.hpp"

using namespace wheels;
using namespace dm;

sslSvrEpoll::sslSvrEpoll(const std::string& ip, uint16_t port, const std::string& ca, const std::string& cert, const std::string& key ):
	sslSvrItfc( ca , cert , key ),
	m_thd_pool__( THD_COUNT )
{
	m_sock__ = socket(AF_INET, SOCK_STREAM, 0);

	// 绑定Socket
	sockaddr_in server_addr;
	
	if( ip.find( ":" ) == std::string::npos ){  // 监听IP4
		server_addr.sin_family = AF_INET;
		inet_pton( AF_INET , ip.c_str() , server_addr.sin_addr.s_addr );
	}else{ // 监听IP6
		server_addr.sin_family = AF_INET6;
		inet_pton( AF_INET6 , ip.c_str() , server_addr.sin_addr.s_addr );
	}
	server_addr.sin_port = htons( port );
	if( bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ){
		throw std::runtime_error( "端口绑定失败" );
	}
	// 准备EPOLL引擎
	sslEpollEgn * egn = sslEpollEgn :: get();
	egn->add( m_sock__.load() );
	// 配置处理函数，实际使用线程池分别处理连接请求和数据通讯的操作
	egn->onSocket( [=]( int sock ){
		if( sock == m_sock__ ){
			m_thd_pool__.enqueue( std::bind( &sslSvrEpoll :: process_accept__ , this ) );
		}else{
			m_thd_pool__.enqueue( std::bind( &sslSvrEpoll :: process_read__ , this , sock ) );
		}
	});
}

bool sslSvrEpoll :: start( int maxConn )
{
	bool ret = false;
	int rc = listen(server_sock, maxConn );
	if( rc != 0 ){
		ERROR_MSG( "监听端口失败" );
	}else{
		MSG( "启动通讯事件引擎" , OK_MSG );
		auto * p_evt_loop = mainLoop< event >::create();
		if( p_evt_loop == nullptr ){
			throw std::runtime_error( "启动事件循环失败" );
		}
		p_evt_loop->exec();
		// 启动线程池
		MSG_1( "启动线程池, 同步处理线程数量: %d" , OK_MSG , THD_COUNT );
		m_thd_pool__.start( true );
		// 启动EPOLL
		MSG( "启动EPOLL" , OK_MSG );
		sslEpollEgn * egn = sslEpollEgn :: get();
		if( egn ){
			egn->run();
		}
	}
	
	return ret;
}

void sslSvrEpoll :: process_accept__()
{
	sockaddr_in client_addr;
        unsigned int addr_len;
	
	// 接受客户端连接
        int client_sock = ::accept( m_sock__.load() , (struct sockaddr*)&client_addr, &addr_len);
	const char * peer_c_str = inet_ntoa(client_addr.sin_addr);
	std::string peer;
	
	if( peer_c_str ){
		peer = peer_c_str;
	}
	
        MSG_1( "远程 %s 正在接入" , TNORMAL , peer_c_str );
	// 通知正在接入
	disppatcher * dispt = disppatcher::get();
	dispt->emit( event::EVT_REQ_CONNECT , peer );
	
	if( this->accept( client_sock ) ){
		// 操作添加到epoll中
		sslEpollEgn * egn = sslEpollEgn :: get();
		egn->add( m_sock__.load() );
		dispt->emit( event::EVT_CONNECTED , peer );
	}else{
		ERROR_MSG( "握手失败" );
		dispt->emit( event::EVT_ERROR , peer );
	}
}

void sslSvrEpoll :: process_read__( int socket )
{
	auto p_conn = get( socket );
	if( p_conn ){
		auto buff = p_conn->read();
		if( buff.dataLen() > 0 ){
			disppatcher<event> * dispt = disppatcher<event>::get();
			evtData data = { p_conn , buff };
			
			dispt->emit( event::EVT_DATA , data );
		}
	}
}
