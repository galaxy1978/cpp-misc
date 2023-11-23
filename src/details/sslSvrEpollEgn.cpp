#include <sys/epoll.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <exception>

using namespace wheels;
using namespace dm;

IMP_SINGLETON( sslEpollEgn );

sslEpollEgn :: sslEpollEgn( int maxevts ):m_id__( -1 ), m_max__( maxevts ), m_counts__( 0 ), m_is_running__( false )
{
	m_id__ = epoll_create1( 0 );
	if( m_id__.load() == -1 ){
		std::stringstream ss;
		ss << "创建SSLEPOLLEGN句柄失败:" << strerror( errno );
		throw std::runtime_error( ss.str() );
	}
}

sslEpollEgn :: ~sslEpollEgn()
{
	close( m_id__.load() );
}
	
bool sslEpollEgn :: add( int ch )
{
	if( ch == nullptr ) return false;
	
	struct epoll_event ev;
	
	ev.events = SSLEPOLLEGNIN;
	ev.data.fd = ch;

	int rst = epoll_ctl( m_id__.load() , SSLEPOLLEGN_CTL_ADD , ev.data.fd , &ev );
	if( rst == -1 ){
		return false;
	}

	m_handles__.insert( ch );

	return true;
}

bool sslEpollEgn :: remove( int ch )
{
	if( ch == nullptr ) return false;

	int rst = epoll_ctl( m_id__.load() , SSLEPOLLEGN_CTL_DEL , ch , nullptr );
	if( rst == -1 ){
		return false;
	}

	auto it = m_handles__.find( ch->native_handle() );
	if( it != m_handles__.end() ){
		m_handles__.erase( it );
	}
	return true;
}

void sslEpollEgn :: run( bool sw , long ovt )
{
	if( sw ){
		if( m_is_running__.load() ) return;
		m_is_running__ = true;

		backend__( ovt );
	}else{
		if( !m_is_running__.load() ) return;
		m_is_running__ = false;
	}
}

void sslEpollEgn :: backend__( long ovt )
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	while ( m_is_running__.load() ) {
		int nfds = epoll_wait( m_id__.load() , pt_events__ , m_max__.load(), ovt );
		if (nfds <= 0 ) {
			if( errno == EINTR || nfds == 0 ){
				continue;
			}
			
			std::stringstream ss;
			ss << "sslEpollEgn发生错误:" << strerror( errno );
			ERROR_MSG( ss.str() );
			m_is_running__ = false;
			return;
		}

		// 处理事件，将所有的通讯整理在一起通知给通道管理模块
		for (int i = 0; i < nfds; i++) { // 发出通知
			if( m_cb__ ){
				m_cb__ ( events[ i ].data.fd );
			}else{
				ERROR_MSG( "事件处理回调函数无效" );
				return;
			}			
		}
	}
}
