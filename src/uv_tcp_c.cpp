/**
 * @brief 使用libuv实现的TCP客户端
 * @version 0.1
 * @date 2017-2-21
 * @author 宋炜
 */
#include <unistd.h>
#include <regex>
#include <thread>
#include <atomic>
#include <chrono>

#include "tcpbase.hpp"
#include "uvlper.hpp"
#include "resolver.hpp"
#include "misc.hpp"
#include "timer.hpp"

extern void StartRebootTimer();

void do_tcp_on_alloc_buff(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	CUVClient *obj = (CUVClient *)handle->data;
	if (obj == nullptr)
		return;

	buf->len = obj->m_i_buf.len;
	buf->base = obj->m_i_buf.base;
}

void do_on_recv(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	CUVClient *pt = (CUVClient *)(stream->data);
	if (pt != NULL && nread >= 0){
		if (pt->cb_recv)
			pt->cb_recv(nread, buf->base); //OnRecv( buf->base , nread , CUVClient::OK );

	}
	else if (nread < 0){
                pt->onError( nread );
	}
}

void on_tcp_connect(uv_connect_t *handle, int status)
{
	CUVClient *pt = (CUVClient *)(handle->data);

	if (status == 0){ // 连接成功
		uv_read_start((uv_stream_t *)(pt->p_client), do_tcp_on_alloc_buff, do_on_recv);
		pt->OnConnected( CUVClient :: OK );
	}
	else{ // 连接失败
		pt->OnConnected(( CUVClient :: err_code) status );
	}
}

void on_write_cb(uv_write_t *req, int status)
{
	CUVClient::st_wd *wd = (CUVClient::st_wd *)(req->data);
	CUVClient *obj = nullptr;

	if (req != nullptr){
		if (wd == NULL)	return;
		uv_buf_t *pt = (uv_buf_t *)(wd->__buff);
		obj = (CUVClient *)(wd->__this);
		if (pt != nullptr && pt->base != NULL){
			free(pt->base);
			free(pt);
			size_t s;
			CUVClient::err_code e;
			if (status < 0)	{
				e = CUVClient::ERR_UVCLIENT_OPERATION;
			}else{
				e = CUVClient::OK;
			}
			s = wd->__ds;
			//回调入对象，处理对象内事务
			obj->OnSend(s, e);
		}
		free(wd);
		free(req);
	}
	if (status < 0)	{
		if (obj != nullptr){
			obj->onError( status );
		}
	}
}

void libuv_shutdown_cb(uv_shutdown_t *req, int status)
{
        CUVClient * p_obj = ( CUVClient * )req->data;
        if( p_obj != nullptr ){
                p_obj->onShutdown();
        }
        
	if (req) free(req);
        
}


/**
 * @brief 响应uv_close操作的回调函数。
 */
void on_uv_close(uv_handle_t *h)
{
	CUVClient *pclient = (CUVClient*)(h->data);
        pclient->OnClose();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUVClient ::CUVClient(const std::string &url, int port):
        m_dns_flag( false ),
        m_conn_flag( false ),
        is_connected( false ),
        is_run( false ),
        m_port( port ),
		m_remote( url ),
        m_error( OK ),
        m_is_closed( false ),
		m_closed_on_error( false ),
        p_resolve_rst(nullptr),
        p_conn_current( nullptr )
{
        //m_o_buf = uv_buf_init((char *)malloc(1024), 1024);
	m_i_buf = uv_buf_init((char *)malloc(1024), 1024);
}

CUVClient ::~CUVClient()
{
	free_dns_result();
	stop();
}


void CUVClient :: onShutdown()
{
	 if( m_closed_on_error == true ){ //如果是因为错误断开链接的，则重新接入。
    	free( p_client );
        p_client = nullptr;
        m_closed_on_error = false;
        is_connected = false;
        sleep( 1 );
                
        connect();
    }
}
void CUVClient :: onError( int err )
{
    ERROR_MSG(  uv_strerror( err ) );
    if (cb_error){
    	cb_error(CUVClient::ERR_UVCLIENT_RECV);
    }
        
    m_closed_on_error = true;
    stop();
}

void CUVClient ::stop()
{
	m_is_closed = false;
	if (is_connected == true){
		uv_read_stop((uv_stream_t *)p_client);
                uv_shutdown_t * req = ( uv_shutdown_t *)malloc( sizeof( uv_shutdown_t ) );
                if( req ){
                        req->data = this;
                        uv_shutdown( req , (uv_stream_t *) p_client , libuv_shutdown_cb );
                }

	}
}

void CUVClient ::init()
{
	is_connected = false;
	int err = 0;
	looper *lp = GetLooper();
	if (lp)	{
		if (!err){
			p_client = ( uv_tcp_t *)malloc( sizeof( uv_tcp_t ));
			if( p_client == nullptr ) throw ERR_ALLOC_MEM;
                        
			err = uv_tcp_init(lp->get(), p_client);
                        if( err < 0 ){
				ERROR_MSG(  uv_strerror( err ) );
                                throw ERR_UVCLIENT_INIT_TCP;
                        }
                        
                        p_client->data = this;
                        req.data = this;
		}else{ //执行 uv_loop_configure错误
			m_error = ERR_UVCLIENT_LOOP_CONFIG;
			err_msg = "Configure uv loop fail.";
			return;
		}
	}else{ //初始化事件循环错误
		m_error = ERR_UVCLIENT_LOOP_INIT;
		err_msg = "Initialize uv event loop fail.";
		return;
	}
}
void CUVClient ::Close()
{
        bool connected = is_connected.load();
        
        stop();

        if( connected == true )
                uv_close((uv_handle_t*)p_client, on_uv_close );
}

void CUVClient ::OnClose()
{
	m_is_closed = true;
        is_connected = false;
        
        if( cb_close ){
		cb_close();
	}
        if( m_closed_on_error == true ){
                
                m_closed_on_error = false;
		std::thread thd( [this]{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
			connect();
		} );
                thd.detach();
        }
}

void CUVClient ::OnSend(size_t s, CUVClient::err_code e)
{
	if (cb_send && e == OK)
		cb_send(s);
}

void CUVClient ::connect4( )
{
        if( p_conn_current == nullptr ) return;
	m_conn_flag = true;
        
	int iret;
	iret = uv_tcp_connect( &req,p_client, p_conn_current->ai_addr, on_tcp_connect );

	if( iret < 0 ){
		ERROR_MSG( uv_strerror( iret ) );
        }

}

void CUVClient ::connect6(  )
{
        if( p_conn_current == nullptr ) return;

	m_conn_flag = true;

	int iret = uv_tcp_connect(&req, p_client, p_conn_current->ai_addr, on_tcp_connect);

	if (iret < 0 ){
		ERROR_MSG( uv_strerror( iret ) );
        }
}

void CUVClient :: save_dns_result( struct addrinfo *rest )
{
	if( rest ){

		struct addrinfo * p1 = rest , * p2 = p_resolve_rst;
		do{// 提取数据			
			if( p_resolve_rst == nullptr ){
                                p2 = ( struct addrinfo * )malloc( sizeof( struct addrinfo ));
				p_resolve_rst = p2;
			}
                        memset( p2 , 0 , sizeof( struct addrinfo ));
			p2 -> ai_flags 		= p1 -> ai_flags;
			p2 -> ai_family 	= p1 -> ai_family; 
			p2 -> ai_socktype 	= p1 -> ai_socktype;
			p2 -> ai_protocol 	= p1 -> ai_protocol;
			p2 -> ai_addrlen 	= p1 -> ai_addrlen;
			p2 -> ai_addr 		= ( struct sockaddr *)malloc( sizeof( struct sockaddr ));

			if( p2 -> ai_addr ){
				memcpy( p2 -> ai_addr , p1->ai_addr , p1 -> ai_addrlen );
				if( p1->ai_canonname ){
					size_t len = strlen( p1 -> ai_canonname );
					p2 -> ai_canonname = ( char *)malloc( len );
					if( p2 -> ai_canonname ){
						strcpy( p2 -> ai_canonname , p1-> ai_canonname );
						p2 -> ai_next = p1 -> ai_next;
					}else{
						free( p2 );
						p2 = nullptr;
						p_resolve_rst = nullptr;
						throw ERR_ALLOC_MEM;
					}
				}
			}else{
				free( p2 );
				p2 = nullptr;
				p_resolve_rst = nullptr;
				throw ERR_ALLOC_MEM;
			}
			// 链检查
			p1 = p1->ai_next;
		}while( p1 != nullptr );
	}
}

void CUVClient :: free_dns_result()
{
        p_conn_current = nullptr;
	struct addrinfo * p = p_resolve_rst , *p1 = nullptr;

	if( p != nullptr ){
		do{
			p1 = p->ai_next;

			if( p->ai_addr ) free( p->ai_addr );
			if( p->ai_canonname ) free( p->ai_canonname );

			free( p );
			p = p1;
		}while( p != nullptr );
	}
	p_resolve_rst = nullptr;
}

void CUVClient ::init_dns_result()
{
	p_conn_current = nullptr;
	struct addrinfo * p = p_resolve_rst , *p1 = nullptr;

	if( p != nullptr ){
		do{
			p1 = p->ai_next;

			if( p->ai_addr ) free( p->ai_addr );
			if( p->ai_canonname ) free( p->ai_canonname );

			free( p );
			p = p1;
		}while( p != nullptr );

		p_resolve_rst = nullptr;
	}
}

void CUVClient ::connecturl()
{
	is_connected 	= false;
	m_is_closed 	= false;
	
	init_dns_result();

	try{
		resolver *prs = nullptr;
		prs = new resolver (m_remote,
				    m_port,
				    true,
				    [this , prs ](struct addrinfo *rest) {
					    if( rest != nullptr ){
						    struct sockaddr * p = nullptr;
						    // 保存DNS解析结果
						    try{
							    save_dns_result( rest );
							    p_conn_current = p_resolve_rst;
							    p = p_conn_current->ai_addr;
							    if( p != nullptr )  real_connect( p );
						    }catch( err_code e ){
							    ERROR_MSG( "Allocate memory fail." );
							    return;
						    }
					    }//if( rest != nullptr 
					    delete prs;
				    }, // end of on success function
				    [ this , prs ](int err) {
					    if (err == 0) return;
					    if (cb_error){ cb_error(ERR_UVCLIENT_DNS); }
					    is_connected = false;
					    ERROR_MSG( uv_strerror(err) );

					    delete prs;
				    });
	}catch( std::bad_alloc& e ){
                ERROR_MSG(e.what() );
		return;
	}

}

CUVClient ::err_code CUVClient ::Send(const char *data, size_t s)
{
	if( data == nullptr ) return ERR_DATA_NULL;
	if( s == 0 ) return ERR_SEND_DATA_EMPTY;
	int ret = 0;
	if (is_connected == false){
		return ERR_UVCLIENT_CONNECTION;
	}
	uv_write_t *req_write; //发送请求句柄
	req_write = (uv_write_t *)malloc(sizeof(uv_write_t));
	if (req_write == nullptr){
		if (cb_error){
			cb_error(ERR_ALLOC_MEM);
		}
	}
	uv_buf_t *b = (uv_buf_t *)malloc(sizeof(uv_buf_t));
	if (b == nullptr){
		is_connected = false;
		if (cb_error){
			cb_error(ERR_UVCLIENT_SEND);
		}
	}
	*b = uv_buf_init((char *)malloc(s), s);
	//初始化需要传递给发送完成回调函数的参数.
	//分配的的内存在两个地方释放，1  如果正常完成操作，在回调函数中释放；2 如果没有发送完成
	//在错误处理中释放
	struct st_wd *wd = (struct st_wd *)malloc(sizeof(struct st_wd));
	if (wd == NULL){
		throw ERR_ALLOC_MEM; //内存分配失败
	}
	wd->__buff 	= b;
	wd->__this 	= this;
	wd->__ds 	= s;
	//开始发送
	if (b->base != NULL){
		req_write->data = wd;
		memcpy(b->base, data, s);
		ret = uv_write(req_write, (uv_stream_t *)p_client, b, 1, on_write_cb);
		
		if (ret < 0){
                        ERROR_MSG( uv_strerror( ret ) );
                        Close();
			if (cb_error){
				cb_error(ERR_UVCLIENT_SEND);
			}
			free(wd);
			free(b->base);
		}
	}else{
		is_connected = false;
		free(wd);
		if (cb_error){
			cb_error(ERR_ALLOC_MEM);
		}
	}

	return OK;
}

void CUVClient ::connect()
{
	init();
	
        uv_tcp_keepalive( p_client, true, 5);
        uv_tcp_nodelay( p_client, 1);
        
        connecturl();
}

void CUVClient ::connect(const std::string &url, int port)
{
	m_remote 	= url;
	m_port 		= port;
        try{        
                connect();
        }catch( err_code e ){
               ERROR_MSG( errMsg( e ));
        }
}

CUVClient ::err_code
CUVClient ::real_connect(struct sockaddr *add)
{
        m_conn_flag = false; 
		m_conn_overtime.setTick( 10000 );
        if ( add->sa_family == AF_INET ){
                connect4();
        }
        else if (add->sa_family == AF_INET6 ){
                connect6( );
        }
	
        m_conn_overtime.SetOwner([this ] { // 设置连接超时处理
			is_connected = false;
			m_conn_flag = true;
	                
			uv_cancel( (uv_req_t *) &req );
	               
			OnConnected(CUVClient ::ERR_CONN_OVERTIME); // 通知连接失败，尝试进行下一地址的连接
		});

        m_conn_overtime.Start(-1, true);   // 启动超时计时器

	return OK;
}

void CUVClient ::OnConnected( err_code e )
{
	m_conn_overtime.Stop();

	if( e == OK ){
		is_connected = true;
		m_conn_flag = true;
		if (cb_conn) cb_conn();
		//free_dns_result();
	}else{
		is_connected = false;
		if( p_conn_current && p_conn_current -> ai_next ){
			p_conn_current = p_conn_current->ai_next;
			real_connect( p_conn_current->ai_addr );
		}else{
                        p_conn_current = p_resolve_rst;
                        onError( (int)e );
                        ERROR_MSG( "can not connect to server, retrying...."  );

			if (cb_error){
				cb_error( ERR_UVCLIENT_CONNECTION );
			}
		}
	}
}

std::string CUVClient :: errMsg( int e )
{
	std::string ret;
	switch( e ){
	case ERR_UVCLIENT_INIT_MUTEX:
		ret = "初始化mutex失败";
		break;
	case ERR_UVCLIENT_LOOP_CONFIG:
		ret = "事件循环对象吴肖";
		break;
	case ERR_UVCLIENT_LOOP_INIT:
		ret = "事件循环初始化失败";
		break;
	case ERR_UVCLIENT_INIT_TCP:
		ret = "初始化TCP句柄失败";
		break;
	case ERR_UVCLIENT_OPERATION:
		ret = "客户端操作失败";
	case ERR_UVCLIENT_ALOCMEM:
		ret = "uvclient内存分配失败";
		break;
	case ERR_UVCLIENT_CONNECTION:
		ret = "连接服务器失败";
		break;
	case ERR_UV_DNS_RESOLVE:
		ret = "解析DNS失败";
		break;
	case ERR_ALLOC_MEM:
		ret = "内存分配失败";
		break;
	case ERR_UVCLIENT_RECV:
		ret = "接收数据操作失败";
		break;
	case ERR_UVCLIENT_SEND:
		ret = "发送数据失败";
		break;
	case ERR_UVCLIENT_DNS:
		ret = "DNS失败";
		break;
	case UV_CLIENT_CREATE_THD:
		ret = "创建线程失败";
		break;
		
	case ERR_CONN_OVERTIME:
		ret = "连接超时";
		break;
	case ERR_SEND_DATA_EMPTY:
		ret = "发送内容不能为空";
		break;
	case ERR_DATA_NULL:
		ret = "数据指针为空";
		break;
	}
	return ret;
}
