#include <string.h>

#include <iostream>
#include <memory>
#include <sstream>

#include "misc.hpp"
#include "uv.h"
#include "connection.hpp"

void uv_alloc_buffer(uv_handle_t* handle,size_t /*suggested_size */,uv_buf_t* buf)
{
	if( handle == nullptr ) return;
	if( buf == nullptr ) return;

	tcpConnect * obj = (tcpConnect *)handle->data;

	obj->onAllocMem( buf );
}

void uv_on_read( uv_stream_t* client,ssize_t nread,const uv_buf_t* buf)
{
	if( client == nullptr ) return;
	if( buf == nullptr ) return;

	tcpConnect * obj = (tcpConnect *)client->data;

	obj->onData( buf->base , nread );
}

void uv_on_close( uv_handle_t * handle )
{
	tcpConnect * obj = (tcpConnect *)handle->data;
	obj->onClosed();
}

void uv_on_write(uv_write_t* __req, int status)
{
	if( __req == nullptr ) return;
	tcpConnect::stSendReq * req = ( tcpConnect::stSendReq * )__req->data;
	if( req == nullptr ) return;

	if( status < 0 ){ // libuv发送数据失败
		tcpConnect * obj = (tcpConnect *)req->obj;
		obj->onSendFail();
		return;
	}

	if( req->data.base != nullptr ){
		free( req->data.base );
	}

	free( req );	
}
/// 以上是C语言的回调函数，用于和libuv对接
/////////////////////////////////////////////////////////////////////////////////////
tcpConnect :: tcpConnect( uv_tcp_t* sock ):
	__p_tcp( sock ),
	__m_is_stop( false )
{
	if( sock == nullptr ){
		throw ERR_TCP_NULL;
	}
        if( __init() == false ){
                throw ERR_INIT_SOCKET;
        }

	__p_tcp->data = this;
}

tcpConnect :: ~tcpConnect()
{
	if( __m_is_stop == false ){
		uv_read_stop( (uv_stream_t*)__p_tcp );
		uv_close( (uv_handle_t *)__p_tcp , nullptr );
	}
}

void tcpConnect :: onData( const char * data , ssize_t len )
{
	if( __cb_on_data && len > 0 ){
		std::stringstream ss;
		ss << __m_peer_ip << ":" << __m_peer_port;
		__cb_on_data( ss.str() , data , len );
	}else if( len < 0 ){
		uv_read_stop( (uv_stream_t*)__p_tcp );
		uv_close( (uv_handle_t*)__p_tcp , uv_on_close );
		__m_is_stop = true;
	}
}

void tcpConnect :: onSendFail()
{
	std::stringstream ss;
	ss << __m_peer_ip << ":" << __m_peer_port;
	if( __cb_error ){
		__cb_error( ss.str() , ERR_SEND_DATA );
	}
}

void tcpConnect :: onAllocMem( uv_buf_t * buf )
{
	buf->len = 1024 * 1024;
	buf->base = p_buff.get();
}

bool tcpConnect :: __init()
{
        bool ret = true;
        
	char * buff = ( char *)malloc( 1024 * 1024 );   // 分配1M内存进行缓冲
	if( buff == nullptr ) {
		ERROR_MSG("Allocate memory fail.");
		ret = false;
		return ret;
	}

	p_buff = std::shared_ptr< char >( buff ,[]( char * p ){
		if( p ){
		       free( p );
		}
	});

	struct sockaddr_in addr;
	int namelen = sizeof( struct sockaddr );
	int rst = uv_tcp_getpeername( __p_tcp , (struct sockaddr*)&addr , &namelen);
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
	}else{
		std::stringstream ss;
		char __tmp[ 100 ];
		ss << inet_ntop(AF_INET , &addr.sin_addr , __tmp , 100 );
		__m_peer_ip = ss.str();
		__m_peer_port = addr.sin_port;

		rst = uv_read_start( (uv_stream_t*)__p_tcp , uv_alloc_buffer , uv_on_read  );
		if( rst != 0 ){
			ERROR_MSG( uv_strerror( rst ) );
			ret = false;
		}
	}
        return ret;
}

void tcpConnect :: start()
{
	int rst = uv_read_start( (uv_stream_t*)__p_tcp, uv_alloc_buffer, uv_on_read);
	if( rst != 0 ){
		ERROR_MSG( uv_strerror( rst ) );
	}
}


std::string tcpConnect :: peerAddress()
{
	return __m_peer_ip;
}

void tcpConnect :: close()
{
	if( __p_tcp && __m_is_stop == false ){
		uv_read_stop( (uv_stream_t*)__p_tcp );
		uv_close( (uv_handle_t*)__p_tcp , uv_on_close );
		__m_is_stop = true;
	}
}
int tcpConnect :: peerPort()
{
	return __m_peer_port;
}

void tcpConnect :: onClosed()
{
	if( __cb_disconnected ){
		std::stringstream ss;
		ss << __m_peer_ip << ":" << __m_peer_port;
		__cb_disconnected( ss.str() );
	}
}

std::string tcpConnect :: errMsg( emErrCode e )
{
	std::string ret;
	switch( e ){
	case ERR_ALLOC_MEM:
		ret = "Allocate memory fail.";
		break;
	case ERR_INIT_SOCKET:
		ret = "Initialize socket fail.";
	break;
	case ERR_SOCKET:
		ret = "Socket fault.";
	break;
	case ERR_SEND_DATA:
		ret = "Send data fail.";
	break;
	case ERR_TCP_NULL:
		ret = "TCP object can not be null pointer.";
	break;
	default:
		break;
	}
	return ret;
}
void tcpConnect :: send( const char * data , size_t len )
{
	stSendReq * req = ( stSendReq *)malloc( sizeof( stSendReq ));
	if( req != nullptr ){
		req->req.data = req;
		req->obj = this;
		req->data.base = ( char *)malloc( len );
		if( req->data.base != nullptr ){
			req->data.len = len;
			memcpy( req->data.base , data , len );

			int rst = uv_write( &req->req,(uv_stream_t*) __p_tcp , &req->data, 1 , uv_on_write );
			if( rst != 0 ){
				ERROR_MSG( uv_strerror( rst ) );
				if( __cb_error ){
					std::stringstream ss;
					ss << __m_peer_ip << ":" << __m_peer_port;
					__cb_error( ss.str() , ERR_SEND_DATA );
				}
			}
		}else{
			ERROR_MSG( "Allocate memory fail." );
			free( req );
			return;
		}
	}else{
		ERROR_MSG( "Allocate memory fail." );
	}
}
