#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <regex>

#include "tcpSvr.hpp"
#include "connection.hpp"
#include "url.hpp"

void uv_new_connection(uv_stream_t* server, int status)
{
	if( server == nullptr ) return;
	
	tcpSvr * obj = ( tcpSvr *)server->data;
	obj->incomingConnection( status );
}

tcpSvr :: tcpSvr( ){}

tcpSvr :: tcpSvr( const std::string& ip , int port ): __m_port( port )
{
	int rst = uv_tcp_init( GetLooper()->get() ,&__m_tcp );
	__m_tcp.data = this;
	
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
		throw ERR_INIT;
	}
	if( __init( ip , port ) == false ){
		throw ERR_INIT;
	}
}

tcpSvr :: ~tcpSvr()
{
        std::map< std::string , tcpConnect *> tmp_map = std::move( __m_clients );
        
        __m_clients.erase( __m_clients.begin() , __m_clients.end() );

        for( auto it = tmp_map.begin(); it != tmp_map.end(); it ++ ){
                delete it->second;
        }
}

bool tcpSvr :: __init( const std::string& ip , int port )
{
        bool ret = true;
	struct sockaddr_in addr;

	uv_ip4_addr( ip.c_str() , port , &addr );
	int rst = uv_tcp_bind( &__m_tcp , ( const struct sockaddr *)&addr , 0 );
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
	}

        return ret;
}


void tcpSvr :: __on_acpt_err( int err )
{
	ERROR_MSG( uv_strerror( err ) );
	if( cb_err ){
		cb_err( err );
	}

	onError( err );
}

void tcpSvr :: incomingConnection( int status )
{
	if( status >= 0 ){
		uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
		if( client == nullptr ){
			ERROR_MSG( "内存分配失败" );
			return;
		}
		int rst = uv_tcp_init( GetLooper()->get() , client);
		if( rst != 0 ){
			ERROR_MSG( uv_strerror( rst ) );
			return;
		}
		//判断accept是否成功
		if(uv_accept( (uv_stream_t*)&__m_tcp,(uv_stream_t*)client) == 0){
			
			tcpConnect * c = new tcpConnect( client );
			std::string peer_ip = __get_peer_ip_port( client );

			__m_clients.insert( std::make_pair( peer_ip , c ));
			// 配置事件响应回调函数
			c->onError([ this ](const std::string& id , tcpConnect::emErrCode e){
				onClientError( id , e ,tcpConnect::errMsg(e) );
			});

			c->onDisconnected( [ this ]( const std::string& id ){
				onDisconnected( id );			
			} );

			c->onRecv( [ this ]( const std::string& id , const char * data , size_t len ){
				onRecvData( id , data , len );	
                        });
			onClientConnected();
			if( cb_client_connected ){
				cb_client_connected( peer_ip );
			}
		}else {
			uv_close((uv_handle_t*) client, NULL);
		}
	}else{
		__on_acpt_err( status );
	}
}

std::string tcpSvr :: __get_peer_ip_port( uv_tcp_t * conn )
{
	std::string ret;
	struct sockaddr_in addr ;
	int nlen = sizeof( struct sockaddr_in ), rst;
	char __tmp[ 100 ];
	memset( __tmp , 0 , 100 );
	rst = uv_tcp_getpeername( conn , (struct sockaddr *)&addr , &nlen );
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
		return ret;
	}
	std::stringstream ss;
	inet_ntop(addr.sin_family , &addr.sin_addr , __tmp , 100 );
	ss <<  __tmp  << ":" << addr.sin_port;
	ret = ss.str();
	return ret;
}

void tcpSvr :: onRecvData( const std::string& /* name */, const char * data , size_t len )
{
	onRecv( data , len );
}

void tcpSvr :: onDisconnected( const std::string& name )
{
	if( cb_client_disconnected ){
		cb_client_disconnected( name );
	}

	// 延迟释放资源保证所有通讯都已经完成
	std::thread thd([ this , &name ]{
		std::this_thread::sleep_for( std::chrono::seconds( 1 ));
		std::lock_guard< std::mutex> l( m_mutex );
		auto it = __m_clients.find( name );
		if( it != __m_clients.end() ){
			if( it->second )
				delete it->second;
			__m_clients.erase( it );
		}

	});
	thd.detach();
}

void tcpSvr :: onClientError( const std::string& name , tcpConnect::emErrCode code , const std::string& msg )
{
	ERROR_MSG( msg );
// 延迟释放资源保证所有通讯都已经完成
	std::thread thd([ this , &name ]{
		std::this_thread::sleep_for( std::chrono::seconds( 1 ));
		std::lock_guard< std::mutex> l( m_mutex );
		auto it = __m_clients.find( name );
		if( it != __m_clients.end() ){
			if( it->second )
				delete it->second;
			__m_clients.erase( it );
		}

	});
	thd.detach();
}

void tcpSvr :: connect( const std::string& url , bool )
{
	int rst = uv_listen( (uv_stream_t*)&__m_tcp , 100 , uv_new_connection );
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
		onError( ERR_LISTEN );
	}else{
		onConnected();
		std::stringstream ss;
		ss << "TCP服务器就绪: " << url << ":" << __m_port;
		MSG( ss.str(), OK_MSG );
	}
}

void tcpSvr :: send( const char * data , size_t len )
{
	if( data == nullptr ) return;
	if( len == 0 ) return;
	//std::cout << "Send data Len = " << len << " clients count: " << __m_clients.size() << std::endl;
	for( auto i : __m_clients ){
		i.second->send( data , len );
	}
	
}
void tcpSvr :: close() 
{
	for( auto i : __m_clients ){
		if( i.second ){
			i.second->close();
		}
	}

	uv_close( (uv_handle_t*)&__m_tcp , nullptr );
}
void tcpSvr :: onRecv( const char * data , size_t len )
{
	if( cb_recved ){
		cb_recved( data , len );
	}
}

void tcpSvr :: onError( int err )
{
	if( cb_err ){
		cb_err( err );
	}
}
void tcpSvr :: onConnected()
{
	if( cb_connected ){
		cb_connected();
	}
}
void tcpSvr :: onDisconnected()
{
	if( cb_closed ){
		cb_closed();
	}
}
void tcpSvr :: onClose()
{
	if( cb_closed ){
		cb_closed();
	}
}
void tcpSvr :: onSend( int err )
{
	if( cb_sended ){
		cb_sended( err );
	}
}
void tcpSvr :: onClientConnected(){}

void tcpSvr :: evtClientDisconnected( std::function< void (const std::string& ) > fun )
{
	cb_client_disconnected = fun;
}

void tcpSvr :: evtError( std::function< void ( int err ) > fun )
{
	cb_err = fun;
}
void tcpSvr :: evtConnected( std::function< void () > fun )
{
	cb_connected = fun;
}

void tcpSvr :: evtClientConnected( std::function< void (const std::string& ) > fun )
{
	cb_client_connected = fun;
}

void tcpSvr :: evtClosed( std::function< void ( ) > fun )
{
	cb_closed = fun;
}
void tcpSvr :: evtRecv( std::function< void ( const char * data , size_t len )> fun )
{
	cb_recved = fun;
}
void tcpSvr :: evtSend( std::function< void ( int err ) > fun )
{
	cb_sended = fun;
}

void tcpSvr :: evtStop( std::function< void ()> fun )
{
	cb_stop = fun;
}
