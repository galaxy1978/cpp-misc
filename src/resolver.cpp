#include <iostream>
#include <sstream>
#include <functional>
#include <unistd.h>
#include <stdlib.h>

#include "resolver.hpp"
#include "misc.hpp"

using namespace wheels;

void on_resolver_cb( uv_getaddrinfo_t* req, int status, struct addrinfo* res )
{
      if( req == nullptr ) return;
      resolver * obj = ( resolver*)(req->data);
      if( obj == nullptr ) return;
      if( status == 0 ){
		   obj->on_resolve( res );
      }else{
	   		obj->seterror( status );
	   		obj->on_resolve( nullptr );
      }
      free( req );
}


resolver :: resolver()
{
      m_state = 0;
      m_error = 0;
      pt_result = nullptr;
}

resolver :: resolver( const std::string& url , int port , bool type , std::function< void ( struct addrinfo * )> fun)
{
      m_error = 0;
      m_state = 0;
      pt_result = nullptr;
      resolve( url , port , type , fun );
}
resolver :: resolver( const std::string& url , const std::string& service , std::function< void ( struct addrinfo * )> fun)
{
      m_error = 0;
      m_state = 0;
      pt_result = nullptr;
      resolve( url , service , fun );
}
resolver :: resolver( const std::string& url , int port , bool type , std::function< void ( struct addrinfo * ) > success , std::function< void ( int )> fail )
{
      m_error = 0;
      m_state = 0;
      pt_result = nullptr;
      resolve( url , port , type , success , fail );
}

resolver :: resolver( const std::string& url , const std::string& service , std::function< void(struct addrinfo*)>success , std::function< void ( int )> fail )
{
      m_error = 0;
      m_state = 0;
      resolve( url , service , success , fail );
}
resolver ::~resolver()
{
      if( pt_result ){
	    free_result( pt_result );
	    pt_result = nullptr;
      }
}

void resolver :: free_result( struct addrinfo * rest )
{
      if( rest && rest->ai_next ){
	    free_result( rest->ai_next );
	    rest->ai_next = nullptr;
      }else{
	    free( rest );
      }
}

void resolver::resolve( const std::string& url , int port , bool type , std::function< void( struct addrinfo* )> fun )
{
	std::stringstream ss;
	ss << port;
	std::string port_str = ss.str();

	struct addrinfo hints;
	uv_getaddrinfo_t * req = ( uv_getaddrinfo_t*)malloc( sizeof( uv_getaddrinfo_t ) );
	req->data = this;
	if( req ){
		// 设置回调函数
	        on_success = fun;
		// 配置解析线索
		hints.ai_family   = AF_UNSPEC;
		hints.ai_protocol = 0;
                if( type )
                    hints.ai_socktype = SOCK_STREAM;
                else
                    hints.ai_socktype = SOCK_DGRAM;

		hints.ai_flags    = 0;
		int e = uv_getaddrinfo( looper::get() , req , on_resolver_cb , url.c_str() , port_str.c_str() , &hints );
		if( e ){
			throw ERR_UV_DNS_RESOLVE;
		}else{
		  m_finish = false;
		}
	}else{
		throw ERR_ALLOC_MEM;
	}
}

void resolver :: resolve( const std::string& url , const std::string& service , std::function< void(struct addrinfo* cb )> fun )
{

	struct addrinfo hints;
	uv_getaddrinfo_t * req = ( uv_getaddrinfo_t*)malloc( sizeof( uv_getaddrinfo_t ) );
	req->data = this;
	if( req ){
		// 设置回调函数
	        on_success  = fun;
		// 配置解析线索
		hints.ai_family   = AF_UNSPEC;
		hints.ai_protocol = 0;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags    = 0;
		int e = uv_getaddrinfo( looper::get() , req , on_resolver_cb , url.c_str() , service.c_str() , &hints );
		if( e ){
			throw ERR_UV_DNS_RESOLVE;
		}else{
		  on_success = fun;
		  m_finish = false;
		}
	}else{
		throw ERR_ALLOC_MEM;
	}
}

void resolver :: on_resolve( struct addrinfo * info )
{
    if( info ){
        struct addrinfo * p = nullptr , * pdest = nullptr;
	if( pt_result ){
	      free_result( pt_result );
	      pt_result = nullptr ;
	}
	if( info != nullptr ){
	    p = info->ai_next;
	    pt_result = ( struct addrinfo *)malloc( sizeof( struct addrinfo));
	    if( pt_result == nullptr ){
		    if(  on_fail ){
			    on_fail( -4 );
		    }
		    return;
	    }
	    pdest = pt_result;
	    memcpy( pt_result , info , sizeof( struct addrinfo ));
	    pdest->ai_next = nullptr;
	    while( p != nullptr && info != nullptr ){
		  pdest->ai_next = ( struct addrinfo *)malloc( sizeof( struct addrinfo ));
		  if( pdest != nullptr ){
		       memcpy( pdest->ai_next , p , sizeof( struct addrinfo ));
		       pdest->ai_next->ai_next = nullptr;
		       pdest = pdest->ai_next ;
		  }

		  p = p->ai_next;
	    }
	    // invoke call back function
	    if( on_success ){
	          on_success( pt_result );
	    }
	}else{
	    	std::cerr << "server is down" << std::endl;
		if( on_fail)
			on_fail( -1 );
	}
    }else if( info == nullptr && on_fail ){
          on_fail( -2 );
    }else{
      // TODO 在没有回调函数的情况下添加需要的错误处理代码
    }
	m_finish = true;
}
void resolver :: resolve( const std::string& url , int port , ArrayString& ips , protocol ptcol )
{
	std::stringstream ss;
	ss << port;
	std::string port_str = ss.str();
	m_finish = false;
	struct addrinfo hints;
	uv_getaddrinfo_t * req = ( uv_getaddrinfo_t*)malloc( sizeof( uv_getaddrinfo_t ) );
	req->data = this;
	if( req ){
		if( ptcol == IP4 )
			hints.ai_family   = AF_INET;
		else if( ptcol == IP6 )
			hints.ai_family   = AF_INET6;
		hints.ai_protocol = 0;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags    = 0;
		int e = uv_getaddrinfo( looper::get()  , req , on_resolver_cb , url.c_str() , port_str.c_str() , &hints );
		if( e ){
		     std::cerr << uv_strerror( e) << std::endl;
		     abort();
		}else{
		  m_finish = false;
		}
	}else{

	}
}

void resolver :: resolve( const std::string& url , const std::string& service , ArrayString& ips , protocol ptcol )
{
}

void resolver :: resolve( const std::string& url , int port , struct addrinfo * res , protocol ptcol )
{
}

void resolver :: resolve( const std::string& url , const std::string& service , struct addrinfo * res , protocol ptcol )
{
}

void resolver :: resolve( const std::string& url ,
                          int port ,
                          bool type ,
                          std::function<void ( struct addrinfo * )> funok ,
			  std::function<void ( int )> funfail)
{
  	std::stringstream ss;
	ss << port;
	std::string port_str = ss.str();

	struct addrinfo hints;
	uv_getaddrinfo_t * req = ( uv_getaddrinfo_t*)malloc( sizeof( uv_getaddrinfo_t ) );
	req->data = this;
	if( req ){
		// 设置回调函数
		on_success = funok;
		on_fail = funfail;
		// 配置解析线索
		hints.ai_family   = AF_UNSPEC;
		hints.ai_protocol = 0;
		if( type ){
			hints.ai_socktype = SOCK_STREAM;
		}else{
			hints.ai_socktype = SOCK_DGRAM;
		}
		hints.ai_flags    = 0;
		int e = uv_getaddrinfo( looper::get() , req , on_resolver_cb , url.c_str() , port_str.c_str() , &hints );
		if( e ){
			throw ERR_UV_DNS_RESOLVE;
		}else{
		  	m_finish = false;
		}
	}else{
		throw ERR_ALLOC_MEM;
	}
}

void resolver :: resolve( const std::string& url ,  const std::string& service ,
			  std::function<void ( struct addrinfo * )> funok ,
			  std::function<void ( int )> funfail
		)
{
	struct addrinfo hints;
	pt_result = nullptr;
	uv_getaddrinfo_t * req = ( uv_getaddrinfo_t*)malloc( sizeof( uv_getaddrinfo_t ) );
	req->data = this;
	if( req ){
		// 设置回调函数
	    on_success = funok;
		on_fail = funfail;
		// 配置解析线索
		hints.ai_family   = AF_UNSPEC;
		hints.ai_protocol = 0;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags    = 0;
		int e = uv_getaddrinfo( looper::get() , req , on_resolver_cb , url.c_str() , service.c_str() , &hints );
		if( e ){
			throw ERR_UV_DNS_RESOLVE;
		}else{
		  	m_finish = false;
		}
	}else{
		throw ERR_ALLOC_MEM;
	}
}

std::string resolver :: errMsg( int err )
{
    std::string ret;
    switch( (emErrCode)err ){
    case ERR_ALLOC_MEM:
        ret = "内存分配失败";
        break;
    case ERR_UV_DNS_RESOLVE:
        ret = "域名解析失败";
        break;
    }
    return ret;
}
