#include <unistd.h>
#if defined( __WIN32 ) || defined( __WIN64 )
#include "direct.h"
#endif

#include <thread>
#include <iostream>
#include <algorithm>
#include <regex>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include <curl/curl.h>

#include "url.hpp"
#include "misc.hpp"
#include "details/ftp.hpp"
using namespace wheels;
const size_t curlFtp::DEFAULT_BUF_SIZE    = 2048;    // 默认缓冲区大小

/*
  DAT   ALLO   APPE   AUTH   CDUP   CLNT   CWD
  DELE   EPRT   EPSV   FEAT   HASH   HELP   LIST   MDTM
  MFMT   MKD    MLSD   MLST   MODE   NLST   NOOP   NOP
  OPTS   PASS   PASV   PBSZ   PORT   PROT   PWD    QUIT
  REST   RETR   RMD    RNFR   RNTO   SITE   SIZE   STOR
  STRU   SYST   TYPE   USER   XCUP   XCWD   XMKD   XPWD
  XRMD

*/
/**
 * @brief 检查服务器支持的指令操作数据回调函数。这个函数有CURL调用
 * @param buffer[ IN ] 传入的数据的内容指针
 * @param size[ IN ] 总是为1
 * @param nitems[ IN ] 传入的数据的长度
 * @param userdata[ IN ] FTP对象指针，用来获取调用的
 * @return 操作成功返回nitems，否则返回0以告诉CURL操作错误
 */
size_t onChkInst(char *buffer,   size_t /*size*/,   size_t nitems,   void *userdata)
{
	curlFtp * obj = ( curlFtp *)userdata;
	if( obj == nullptr ) return nitems;

	if( buffer != nullptr ){
		obj->__onChkInst( buffer , nitems );
	}
	return nitems;
}
/**
 * @brief 连接成功操作回调函数
 */
size_t onConnectCb(char *ptr, size_t /*size*/, size_t nmemb, void *userdata)
{
	size_t ret = nmemb;
	curlFtp * obj = ( curlFtp *) userdata ;
	if( obj != nullptr && ret > 0 ){
		obj->__onConnect( ptr , ret );
	}else if( obj != nullptr ){
		obj->__onConnectErr();
	}else{
		ERROR_MSG( "Connect ftp server fail");
	}
	return ret;
}
/**
 * @brief CURL 本地写入回调函数
 * @param ptr[ I ] 数据指针
 * @param size[ I ] 总是是 1
 * @param nmemb[ I ] 数据长度
 * @param userdata[ I ] 用户定义数量
 * @return 接收到数据长度
 */
size_t ftp_write_callback(char *ptr, size_t /*size*/, size_t nmemb, void *userdata)
{
        curlFtp::errCode ret = curlFtp::OK;

        curlFtp::stTransmit * desc = (curlFtp::stTransmit *)userdata;
        if( desc ){
                curlFtp * obj = desc->p_ftp;
                if( obj == nullptr ) return nmemb;

                desc->p_buffer = ( uint8_t *)realloc( desc->p_buffer , nmemb );
                if( desc == nullptr ){ return curlFtp::ERR_DESC_NULL; }
                memcpy( desc->p_buffer , ptr , nmemb );
                desc->m_buf_size = nmemb;
                desc->m_data_size = nmemb;
                ret = (curlFtp::errCode)obj->on_recv( desc );

                if( ret == CURL_WRITEFUNC_PAUSE ){ // 执行暂停操作
                        obj->on_pause( triTrue );
                }else if( ret == curlFtp :: ERR_CANCEL_TRANSFER ){ // 执行取消操作
                        obj->on_pause( triFalse );
                }
        }

        return nmemb;
}

size_t ftp_write_cb_data(char *ptr, size_t /*size*/, size_t nmemb, void *userdata)
{
        size_t ret = 0;

        curlFtp::stTransmit * desc = (curlFtp::stTransmit *)userdata;

        if( desc ){
                curlFtp * obj = desc->p_ftp;
                if( obj == nullptr ) return nmemb;

                desc->p_buffer = ( uint8_t *)realloc( desc->p_buffer , nmemb );
                if( desc == nullptr ){ return 0; }
                memcpy( desc->p_buffer , ptr , nmemb );
                desc->m_buf_size = nmemb;
                desc->m_data_size = nmemb;
                ret = obj->on_recv_data( desc );

                if( ret == CURL_WRITEFUNC_PAUSE ){ // 执行暂停操作
                        obj->on_pause( triTrue );
                }else if( (curlFtp::errCode)ret == curlFtp :: ERR_CANCEL_TRANSFER ){ // 执行取消操作
                        obj->on_pause( triFalse );
                }
        }

        return nmemb;
}
/**
 * @brief 本地读取数据回调函数
 */
size_t ftp_read_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
        size_t ret = size * nitems ;

	curlFtp::stTransmit * obj = reinterpret_cast< curlFtp::stTransmit *>( userdata );
        if( obj && obj->p_ftp ){
                obj->m_is.read( buffer , ret );
		int count = obj->m_is.gcount();
		if( count > 0 ){ // 通知实际读取的内容长度
			ret = count;
		}
		else if( obj->m_is.eof() ){ // 通知文件传输完成
			ret = 0;
		}
        }
        return ret;
}

int on_upld_progress(void *clientp, curl_off_t /*dltotal*/, curl_off_t dlnow, curl_off_t /*ultotal*/, curl_off_t ulnow)
{
	if( clientp == nullptr ){
		return 1;
	}

	curlFtp :: stTransmit * ctl_info = ( curlFtp::stTransmit *)clientp;

	if( ctl_info->m_type == triFalse ){
		if( ctl_info && ctl_info->p_ftp ){  // 这里回调回去，通知传输情况
			ctl_info->p_ftp->__on_progress( (size_t)ulnow );
		}
	}else{
		if( ctl_info && ctl_info->p_ftp ){  // 这里回调回去，通知传输情况
			ctl_info->p_ftp->__on_progress_dwld( (size_t)dlnow );
		}
	}
	return 0;;
}
/**
 * @brief 查询文件大小的回调操作函数
 */
size_t on_ftp_size( char *ptr, size_t size, size_t nmemb, void *userdata )
{
	if( userdata == nullptr ) return size;
	if( ptr == nullptr ) return size;
	curlFtp * obj = ( curlFtp * ) userdata;

	obj->onSize( ptr , nmemb );

	return nmemb;
}
/**
 * @brief 获取文件列表的回调操作函数
 */
size_t on_nlist( char */*ptr*/, size_t /*size*/, size_t nmemb, void */*userdata*/ )
{
	size_t ret = nmemb;
// TODO (17367#1#): 添加NLST的响应代码

	return ret;
}

curlFtp :: curlFtp():p_ctx( nullptr ),m_port( 0 ),m_file_size( 0 ), m_d_u( triReady ),m_is_running( false )
{
      
	m_local_path = getcwd( nullptr , 0 );
        CURLcode rst = curl_global_init( CURL_GLOBAL_ALL );
        if( rst != 0 ){
                throw ERR_INIT_CURL;
        }
}

curlFtp :: curlFtp( const stParams& params ):p_ctx( nullptr ),m_port( 0 ), m_params( params ),m_file_size( 0 ), m_d_u( triReady ),m_is_running( false )
{

	m_local_path = getcwd( nullptr , 0 );
        CURLcode rst = curl_global_init( CURL_GLOBAL_ALL );

        if( rst != 0 ){
		ERROR_MSG(  curl_easy_strerror( rst ) );
                throw ERR_INIT_CURL;
        }
}

curlFtp :: curlFtp( const std::string& url_str ):p_ctx( nullptr ),m_port( 0 ),m_file_size( 0 ), m_d_u( triReady ),m_is_running( false )
{
	try{
		URL url( url_str );
		m_local_path = getcwd( nullptr , 0 );
		m_remote_add = url.HostName();
		m_port = url.HostPort();
		if( m_port < 0 ){
			m_port = 21;
		}
		m_usr = url.AuthUser();
		m_pswd = url.AuthPswd();

		CURLcode rst = curl_global_init( CURL_GLOBAL_ALL );

		if( rst != 0 ){
			ERROR_MSG(  curl_easy_strerror( rst ) );
			throw ERR_INIT_CURL;
		}
	}catch( URL::emErrCode e ){
		throw ERR_INIT_FTP;
		ERROR_MSG( " URL 格式错误，无法初始化FTP连接 " );
	}
}

curlFtp :: ~curlFtp()
{
        m_d_u = triReady;
        wait_finish();
        curl_easy_cleanup( p_ctx );
        curl_global_cleanup();
}

void curlFtp :: __on_progress( size_t now )
{
	if( upld_cb ){
		upld_cb( curlFtp :: STATUS_TRANSING , now );
	}
}

void curlFtp :: __on_progress_dwld( size_t now )
{
    (void)now;
//	if( dwld_cb ){
//		dwld_cb( curlFtp :: STATUS_TRANSING , now );
//	}
}

int curlFtp :: on_pause( tribool_t status )
{
	int ret = 0;
    (void)status;
	return ret;
}

curlFtp :: errCode
curlFtp :: real_download( const std::string& from ,  const std::string& to )
{
        errCode ret = OK;
        std::ofstream ofs;

	std::string usr , pswd;
	URL url( from );
	usr = url.AuthUser();
	pswd = url.AuthPswd();
	if( usr.empty() ){
		usr = m_usr;
		pswd = m_pswd;
	}

	// 准备传输过程中的描述参数
	stTransmit  transit;
	transit.p_ftp  = this;		  transit.m_type     = triTrue;
	transit.m_from = from;            transit.m_dest     = to;
	transit.p_os   = &ofs;            transit.m_buf_size = m_params.buf_size;
    transit.p_buffer = ( uint8_t *)malloc( m_params.buf_size );
	if( transit.p_buffer == nullptr ){
		return ERR_ALLOC_MEM;
	}
	// 准备连接服务器
	CURLcode rst;
	CURL * p_ctx1 = curl_easy_init( );
	if( p_ctx1 == nullptr ){
		ERROR_MSG( "Initialize curl context fail." );
		return ERR_INIT_CURL;
	}
	std::string path;
	if( from.find( "ftp://" ) == std::string::npos ){
		path = "ftp://" + from;
	}else{
		path = from;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_URL , from.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_USERNAME , usr.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_USER_OR_PSWD;
	}
	if( !pswd.empty() ){
		rst = curl_easy_setopt( p_ctx1 , CURLOPT_PASSWORD , pswd.c_str() );
		if( rst != CURLE_OK ){
			ERROR_MSG( curl_easy_strerror( rst ) );
			return ERR_USER_OR_PSWD;
		}
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_WRITEFUNCTION ,  ftp_write_callback );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_WRITEDATA, &transit );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_XFERINFOFUNCTION, on_upld_progress );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_XFERINFODATA , &transit );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_NOPROGRESS, 0L);
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}
	/*rst = curl_easy_setopt( p_ctx1 , CURLOPT_TIMEOUT , 10 );
	  if( rst != CURLE_OK ){
	  ERROR_MSG( curl_easy_strerror( rst ) );
	  return ERR_CURL_SET_OPT;
	  }*/
	rst = curl_easy_setopt(p_ctx1,CURLOPT_LOW_SPEED_TIME,60L);
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}
	rst = curl_easy_setopt(p_ctx1,CURLOPT_LOW_SPEED_LIMIT,100L);
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt(p_ctx1, CURLOPT_FTPPORT, NULL );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		return ERR_CURL_SET_OPT;
	}
	
	m_is_running = true;
	// 创建本地目标文件
	ofs.open( to.c_str() , std::ofstream::out | std::ofstream::binary );

	if( ofs.is_open() == false ){
		ERROR_MSG( "Can not create local file." );
		return ERR_CREATE_DEST_FILE;
	}
	rst = curl_easy_perform( p_ctx1 );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ) );
		ofs.close();
		if( ::access( to.c_str() , F_OK ) == 0 ){
			remove( to.c_str() );
		}
		ret = ERR_FILE_DOWNLOAD_TRANSFER;
	}else{
		if( dwld_cb ){
            dwld_cb( STATUS_TRANS_FINISH , transit.p_buffer , transit.m_data_size );
		}
		ofs.close();
	}
	// 关闭文件结束传输

	curl_easy_cleanup( p_ctx1 );

        return ret;
}

curlFtp :: errCode
curlFtp :: __real_download( const std::string& from , char * data )
{
    errCode ret = OK;
    URL url( from );
	// 准备传输过程中的描述参数
	stTransmit  transit;
    transit.p_ftp           = this;
    transit.m_type          = triTrue;
    transit.m_from          = from;
    transit.p_os            = nullptr;
    transit.m_buf_size      = m_params.buf_size;
    transit.p_buffer        = ( uint8_t *)malloc( m_params.buf_size );
    transit.p_final_buffer  = data;

	if( transit.p_buffer == nullptr ){
		ERROR_MSG( "Allocate memory fail." );
		return ERR_ALLOC_MEM;
	}
	// 准备连接服务器
	CURLcode rst;
	CURL * p_ctx1 = curl_easy_init( );
	if( p_ctx1 == nullptr ){
		ERROR_MSG( "Initialize curl context fail." );
		return ERR_INIT_CURL;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_URL , from.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
	}
    rst = curl_easy_setopt( p_ctx1 , CURLOPT_USERNAME , url.AuthUser().c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_USER_OR_PSWD;
	}
    rst = curl_easy_setopt( p_ctx1 , CURLOPT_PASSWORD , url.AuthPswd().c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_USER_OR_PSWD;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_WRITEFUNCTION ,  ftp_write_cb_data );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_CURL_SET_OPT;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_WRITEDATA, &transit );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_CURL_SET_OPT;
	}
	rst = curl_easy_setopt( p_ctx1 , CURLOPT_XFERINFOFUNCTION, on_upld_progress );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_XFERINFODATA , &transit );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_CURL_SET_OPT;
	}

	rst = curl_easy_setopt( p_ctx1 , CURLOPT_NOPROGRESS, 0L);
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ERR_CURL_SET_OPT;
	}
	m_is_running = true;
	rst = curl_easy_perform( p_ctx1 );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		ret = ERR_FILE_DOWNLOAD_TRANSFER;
		curl_easy_cleanup( p_ctx1 );
	}else{
		curl_easy_cleanup( p_ctx1 );
		if( dwld_cb ){
            dwld_cb( STATUS_TRANS_FINISH ,transit.p_buffer , transit.m_data_size );
		}else{
			ERROR_MSG( "notify function is defined, you may not get finish information." );
		}
	}

        return ret;
}

void curlFtp :: wait_finish( bool forceFinish )
{
	if( forceFinish == true ){
		m_is_running = false;
	}

	while( true ){
		if( m_is_running == false ){
			break;
		}
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ));
	}
}

bool curlFtp :: download( const std::string& from , const std::string& dest , bool autoRun )
{
	bool ret = false;

	try{
        	m_files.erase( m_files.begin() , m_files.end() );
        	m_dests.erase( m_dests.begin() , m_dests.end() );
                remody_url( from , dest );
                if( autoRun ){
                        errCode e = real_download( m_files[ 0 ] , m_dests[ 0 ] );
                        if( e != OK ){
				do_process_error( e );
                        }else{
				ret = true;
                        }
                }
        }catch( URL::emErrCode e ){
                ERROR_MSG( "check URL fail. " );
                ret = false;
        }
        return ret;
}

void curlFtp :: remody_url( const std::string& from , const std::string& dest )
{
	std::string strF;
	if( from.find( "ftp://") == std::string::npos ){
		if( from.at( 0 ) != '/' ){
			strF = m_remote_add + "/" + from;
		}else{
			strF = m_remote_add + from;
		}
	}else{
		strF = from;
	}

        URL url( strF );

        // 补足协议
        std::string str = url.Protocol( );
        std::transform( str.begin() , str.end() , str.begin() , ::toupper );
        if( str.empty() || str != "FTP" ){
                url.Protocol( "ftp");
	}
        if( url.HostPort() <= 0 ){
                url.HostPort( m_port );
        }
        // 补足用户和密码
        if( is_anonymous() == false ){
                if( url.AuthUser().empty() ){
                        url.AuthUser( m_usr );
                }

                if( url.AuthPswd().empty() ){
                        url.AuthPswd( m_pswd );
                }
        }else{
                url.AuthUser( "anonymous" );
        }
	str = url.toString();

        m_files.push_back( str );

        if( dest.empty() ){
                str = m_local_path + "/" + url.File();
        }else{
                str = dest;
        }
        m_dests.push_back( str );
}
void curlFtp :: download( const std::string& from , const std::string& dest, std::function< bool ( errCode , uint8_t *, size_t  ) > cb ,
			  bool autoRun )
{
        dwld_cb = cb;
        m_files.erase( m_files.begin() , m_files.end() );
        m_dests.erase( m_dests.begin() , m_dests.end() );
        try{
                remody_url( from , dest );

                if( autoRun ){
			std::thread thd( [ & ]{
				m_is_running = true;
				errCode e = real_download( m_files[ 0 ] , m_dests[ 0 ] );
				if( e != OK ){
					do_process_error( e );
				}

				m_is_running = false;
			});

			thd.detach();

                }
	}catch( URL::emErrCode ){
		ERROR_MSG( "check URL fail. " );
        }
}

void curlFtp :: download( const std::string& from , char * buffer , std::function< bool ( errCode , uint8_t * ,size_t ) > cb , bool autoRun )
{
    dwld_cb = cb;
    m_files.erase( m_files.begin() , m_files.end() );
    m_dests.erase( m_dests.begin() , m_dests.end() );
    std::string dests;
    try{
        URL url( from );
        m_usr = url.AuthUser();
        m_pswd = url.AuthPswd();

        remody_url( from , dests );

        if( autoRun ){
			m_is_running = true;

            //std::thread thd([=]{
                errCode e = __real_download( from , buffer );
                if( e != OK ){
                    do_process_error( e );
                }

                m_is_running = false;
            //});

           // thd.detach();
        }
	}catch( URL::emErrCode ){
		ERROR_MSG( " check URL fail. ");
    }
}
bool curlFtp :: download( const ArrayString& from , const ArrayString& dest , bool autoRun )
{
        bool ret = false;
        if( m_files.size() <= 0 ) return false;
        if( from.size() != dest.size() && dest.size() != 0 ) return false;
        // 有任务正在进行
        if(m_is_running == true ) return false;
        if( m_pause == triTrue ) return false;

        m_files.erase( m_files.begin() , m_files.end() );
        m_dests.erase( m_dests.begin() , m_dests.end() );
        // 准备下载相关数据
        try{
                for( size_t i = 0; i < from.size(); i ++ ){
                        remody_url( from[ i ] , dest[ i ] );
                }
                m_d_u = false;
                m_pause = triFalse;
                m_is_running = true;
        }catch( URL::emErrCode e ){
		ERROR_MSG( "check URL fail. ");
                return ret;
        }

        if( autoRun ){

                for( size_t i = 0; i < m_files.size(); i ++ ){
                        errCode e = real_download( m_files[ i ] , m_dests[ i ] );
                        if( e != OK ){
                                do_process_error( e );
                        }
                }
        }

        return true;
}

bool curlFtp :: is_anonymous()
{
        bool ret = false;
        if( m_usr == "anonymous" ){
                ret = true;
        }
        return ret;
}

void curlFtp :: download( const ArrayString& from , const ArrayString& dest , std::function< bool ( errCode , uint8_t * ,size_t ) > cb, bool autoRun )
{
        if( m_files.size() <= 0 ) return;
        if( from.size() != dest.size() && dest.size() != 0 ) return;
        // 有任务正在进行
        if(m_is_running == true ) return;
        if( m_pause == triTrue ) return;

        m_files.erase( m_files.begin() , m_files.end() );
        m_dests.erase( m_dests.begin() , m_dests.end() );
        // 准备下载相关数据
        try{
                for( size_t i = 0; i < from.size(); i ++ ){
                        remody_url( from[ i ] , dest[ i ] );
                }
                m_d_u = false;
                m_pause = triFalse;
                m_is_running = true;
                dwld_cb = cb;
        }catch( URL::emErrCode e ){
		ERROR_MSG( "check URL fail. " );
                return;
        }

        if( autoRun ){
                for( size_t i = 0; i < m_files.size(); i ++ ){
                        errCode e = real_download( m_files[ i ] , m_dests[ i ] );
                        if( e != OK ){
                                __do_process_err( e );
                        }
                }
        }
}

void curlFtp :: __do_process_err( errCode e )
{
    switch( e ){
    case ERR_DOWNLOAD_OSS:
        ERROR_MSG("oss下载失败");
        break;
    case ERR_SYS_VAR:
        break;
    case ERR_CANCEL_TRANSFER:
        break;
    case ERR_UNSUPPORTED_PROTOCOL:
        ERROR_MSG( "CURL不支持指定协议" );
        break;
    case ERR_INIT_CURL:
        ERROR_MSG( "初始化CURL错误" );
        break;
    case ERR_CONNECT_SVR:
        ERROR_MSG( "连接服务器错误" );
        break;
    case ERR_FILE_DOWNLOAD_TRANSFER:
        ERROR_MSG( "下载传输过程错误" );
        break;
    case ERR_FILE_UPLOAD_TRANSFER:
        ERROR_MSG( "上传文件传输过程错误" );
        break;
    case ERR_CTX_NULL:
        ERROR_MSG( "CURL 上下文对象空" );
    break;
    case ERR_ALLOC_MEM:
        ERROR_MSG( "内存分配失败" );
    break;
    case ERR_CREATE_DEST_FILE:
        ERROR_MSG( "创建目的文件失败" );
    break;
    case ERR_CURL_SET_OPT:
        ERROR_MSG( "配置CURL参数失败");
    break;
    case ERR_USER_OR_PSWD:
        ERROR_MSG( "用户名或者密码错误");
    break;
    case ERR_CURL_PERFORM:
        ERROR_MSG( "执行CURL失败" );
    break;
    case ERR_OPEN_LOC_FILE:
        ERROR_MSG( "打开本地文件失败");
    break;
    case ERR_NO_SRC_FILE:
        ERROR_MSG( "缺少源文件" );
    break;
    case ERR_DESC_NULL:
        ERROR_MSG( "描述信息空" );
    break;
    case ERR_EMPTY_URL:
        ERROR_MSG( "url没有相关内容" );
    break;
    case ERR_FILE_NOT_FOUND:
        ERROR_MSG( "远程文件不存在" );
    break;
    case ERR_ACCESS_NOT_SUPPORT:
        ERROR_MSG( "不支持检查文件是否存在的操作" );
    break;             //
    case STATUS_TRANS_FINISH:
        ERROR_MSG( "数据传输完成" );
    break;
    case STATUS_TRANSING:
        ERROR_MSG( "数据正在传输");
    break;
    case ERR_INIT_FTP:
        ERROR_MSG( "初始化FTP模块失败" );
    break;
    case ERR_DELE_OSS_FILE:
        ERROR_MSG( "删除OSS文件失败" );
    break;
    case ERR_LS_OSS_FILE:
        ERROR_MSG( "列出OSS文件失败" );
    break;
    case ERR_UPLOAD_OSS:
        ERROR_MSG( "上传OSS文件失败" );
        break;
    }
}
bool curlFtp :: upload( const std::string& file , const std::string& remote , bool autoRun )
{
	bool ret = false;
    (void)autoRun;
	std::string __remote = remote;
	MSG_1( "准备上传文件：%s" , TNORMAL , remote.c_str() );

	if( file.empty() ) throw ERR_NO_SRC_FILE;
	if( __remote.empty() ){
		if( fileNameFromPath( file , __remote ) == false ){
			ERROR_MSG( "Can not upload directory." );
			return ret;
		}
	}
	URL url( remote );
	if( url.Protocol().empty() ){
		url.Protocol( "ftp");
	}
	if( url.HostPort() == -1 ){
		url.HostPort( m_port );
	}
	if( url.HostName().empty()){
		url.HostName( m_remote_add );
	}
        __remote = url.toString();

	errCode e = __real_upload( file , __remote );
	if( e != OK ){
		MSG_1( "Upload file failed: %d " , TNORMAL , e  );
	}else{
		ret = true;
	}

	return ret;
}

void curlFtp :: upload( const std::string& file , const std::string& remote , std::function< bool ( errCode , size_t )> cb , bool autoRun )
{
	std::string __remote = remote;
    (void)autoRun;
	if( file.empty() ) throw ERR_NO_SRC_FILE;
	if( __remote.empty() ){
		if( fileNameFromPath( file , __remote ) == false ){
			ERROR_MSG( "Can not upload directory.");
			return;
		}
	}
	URL url( remote );
	if( url.Protocol().empty() ){
		url.Protocol( "ftp");
	}
	if( url.HostPort() == -1 ){
		url.HostPort( m_port );
	}
	if( url.HostName().empty()){
		url.HostName( m_remote_add );
	}
        __remote = url.toString();
	upld_cb = cb;
	errCode e = __real_upload( file , __remote );
	if( e != OK ){
		ERROR_MSG( " Upload file failed: " );
	}else{
                cb( OK , 0 );
	}
}

bool curlFtp :: del( const std::string& rem )
{
	bool ret = false;
	struct curl_slist * hl = nullptr;
	// 在默认情况下，如果没有在文件路径URL中指定用户名和密码，则以默认对象中配置的参数为准。
	// 否则，需要解析这两个内容以参数指定的内容为准
	std::string user = m_usr , pswd = m_pswd;
	CURLcode rst;
    std::string host;

	if( rem.find( "ftp://") != std::string::npos ){
		// 当文件路径中明确的包含了协议描述，
		URL url( rem );
		std::stringstream ss;
		ss << url.Protocol() << "://" << url.HostName() << ":" << url.HostPort();
		host = ss.str();
		// 解析用户名和密码参数
		if( url.AuthUser().empty() == false ){
			user = url.AuthUser();
		}

		if( url.AuthPswd().empty() == false ){
			pswd = url.AuthPswd();
		}
	}else{// 在URL中没有明确指明协议情况，则按照对象保留的参数为准进行访问服务器
		std::stringstream ss;
		ss << "ftp://" << m_remote_add << ":" <<m_port;
		host = ss.str();
	}
    rst = curl_easy_setopt( p_ctx , CURLOPT_URL , host.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
	}
	std::string cmd("DELE ");
	cmd += rem;

	hl = curl_slist_append(hl, cmd.c_str());
	curl_easy_setopt(p_ctx, CURLOPT_POSTQUOTE, hl);

	rst = curl_easy_setopt( p_ctx, CURLOPT_USERNAME, user.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ret;
	}

	rst = curl_easy_setopt( p_ctx, CURLOPT_PASSWORD, pswd.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ret;
	}

	m_is_running = true;
	rst = curl_easy_perform( p_ctx );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
	}else{
		if( dwld_cb ){
            dwld_cb( STATUS_TRANS_FINISH , nullptr ,0 );
		}

		ret = true;
	}

	return ret;
}

bool curlFtp :: upload( const ArrayString& from , bool autoRun )
{
	bool ret = false;
    (void)from;
    (void)autoRun;
	return ret;
}

void curlFtp :: upload( const ArrayString& file , std::function< bool ( errCode , size_t )> cb , bool autoRun )
{
    (void)file;
    (void)autoRun;
    (void)cb;
}

void curlFtp :: run()
{
	p_ctx = curl_easy_init();
	if( p_ctx == nullptr ){
		throw ERR_CTX_NULL;
	}

	if( __connect() == false ){
		throw ERR_CONNECT_SVR;
	}
}

void curlFtp :: run( std::function< bool ( errCode , size_t , int , const std::string& ) > cb )
{
	p_ctx = curl_easy_init();
	if( p_ctx == nullptr ){
		throw ERR_CTX_NULL;
	}
	if( cb ){
		run_cb = cb;
	}
}

curlFtp :: errCode
curlFtp :: ls( ArrayString& rst )
{
	errCode ret = OK;
	if( p_ctx == nullptr ) return ERR_CTX_NULL;

	rst.erase( rst.begin(), rst.end() );
	std::string cmd;
	int p1 , p2;
	p1 = 65535 / 256;
	p2 = 255;
	p1 = rand() % p1;
	p2 = rand() % p2;

//	std::stringstream ss;
//	std::string l_ip = localIP( false );
//	size_t pos;
//	while( ( pos = l_ip.find( '.' ) != std::string::npos )){
//		l_ip.replace( pos , 1 , 1 , ',');
//	}

//	ss << "PORT " << l_ip << "," << p1 << "," << p2;
//	CURLcode e = curl_easy_setopt( p_ctx , CURLOPT_CUSTOMREQUEST , ss.str().c_str() );
//	if( e != CURLE_OK ){
//		ERROR_MSG( " Set ftp port command fail." );
//		return ERR_CURL_SET_OPT;
//	}
	CURLcode e = curl_easy_setopt( p_ctx , CURLOPT_WRITEFUNCTION , nullptr );
	if( e != CURLE_OK ){
		ERROR_MSG( " clear port callback fail."  );
		return ERR_CURL_SET_OPT;
	}
	e = curl_easy_perform( p_ctx );
	if( e != CURLE_OK ){
		ERROR_MSG( " Execute port command fail." );
		return ERR_CURL_SET_OPT;
	}else{
		e = curl_easy_setopt( p_ctx , CURLOPT_WRITEFUNCTION , on_nlist );
		if( e != CURLE_OK ){
			ERROR_MSG( " Set ftp list callback." );
			return ERR_CURL_SET_OPT;
		}
		e = curl_easy_setopt( p_ctx , CURLOPT_WRITEDATA , this );
		if( e != CURLE_OK ){
			ERROR_MSG( " Set ftp list object fail." );
			return ERR_CURL_SET_OPT;
		}
		e = curl_easy_setopt( p_ctx , CURLOPT_CUSTOMREQUEST , "NLST" );
		if( e != CURLE_OK ){
			ERROR_MSG( " Set ftp nlst query command fail." );
			return ERR_CURL_SET_OPT;
		}
		e = curl_easy_perform( p_ctx );
		if( e != CURLE_OK ){
			ERROR_MSG( " Execute ftp NLST query command fail , try LIST." );

			e = curl_easy_setopt( p_ctx , CURLOPT_CUSTOMREQUEST , "NLST" );
			if( e != CURLE_OK ){
				ERROR_MSG( " Set ftp list query command fail." );
				return ERR_CURL_SET_OPT;
			}
			e = curl_easy_perform( p_ctx );
			if( CURLE_OK != e ){
				ERROR_MSG( " Execute ftp LIST query command fail" );
			}
		}

		rst = m_files;
		ret = OK;
	}
	return ret;
}

curlFtp :: errCode
curlFtp :: __access( const std::string& url )
{
	errCode ret = OK;

	CURLcode e = curl_easy_setopt( p_ctx , CURLOPT_URL , url.c_str() );
	if( e != CURLE_OK ){
		ERROR_MSG( " Set ftp query command fail." );
		return ERR_CURL_SET_OPT;
	}
	e = curl_easy_setopt(p_ctx, CURLOPT_NOBODY, 1L);
	if( e != CURLE_OK ){
		ERROR_MSG( " Set ftp query command fail." );
		return ERR_CURL_SET_OPT;
	}
	e = curl_easy_setopt(p_ctx, CURLOPT_HEADER, 1L);
	if( e != CURLE_OK ){
		ERROR_MSG( " Set ftp query command fail." );
		return ERR_CURL_SET_OPT;
	}
	e = curl_easy_perform( p_ctx );
	if( e != CURLE_OK ){
		ERROR_MSG( " Set ftp query command fail." );
		return ERR_CURL_SET_OPT;
	}
	if(e == CURLE_REMOTE_FILE_NOT_FOUND){
		ret = ERR_FILE_NOT_FOUND;
	}else if( e == CURLE_OK){
		ret = OK;
	}
	return ret;
}

curlFtp :: errCode
curlFtp :: access( const std::string& url , int type )
{
	errCode ret = OK;
	if( url.empty() ) return ERR_EMPTY_URL;
	switch( type ){
	case F_OK:
		ret = __access( url );
		break;
	default:
		return ERR_ACCESS_NOT_SUPPORT;
		break;
	}

	return ret;
}

void curlFtp :: ls( std::function< void ( errCode , ArrayString& ) > cb )
{
	errCode e = ls( m_files );
	if( cb ){
		cb( e , m_files );
	}
}

curlFtp :: errCode
curlFtp :: cd( const std::string& path )
{
	errCode ret = OK;

	struct curl_slist * hl = nullptr;
	// 在默认情况下，如果没有在文件路径URL中指定用户名和密码，则以默认对象中配置的参数为准。
	// 否则，需要解析这两个内容以参数指定的内容为准
    std::string user = m_usr , pswd = m_pswd , host;
	CURLcode rst;

    if( path.find( "ftp://") != std::string::npos ){
		// 当文件路径中明确的包含了协议描述，
        URL url( path );
		std::stringstream ss;
		ss << url.Protocol() << "://" << url.HostName() << ":" << url.HostPort();
		host = ss.str();
		// 解析用户名和密码参数
		if( url.AuthUser().empty() == false ){
			user = url.AuthUser();
		}

		if( url.AuthPswd().empty() == false ){
			pswd = url.AuthPswd();
		}
	}else{// 在URL中没有明确指明协议情况，则按照对象保留的参数为准进行访问服务器
		std::stringstream ss;
		ss << "ftp://" << m_remote_add << ":" <<m_port;
		host = ss.str();
	}
	rst = curl_easy_setopt( p_ctx , CURLOPT_URL , host.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
	}

	std::string cmd("CWD ");
    cmd += path;

	hl = curl_slist_append(hl, cmd.c_str());
	curl_easy_setopt(p_ctx, CURLOPT_POSTQUOTE, hl);

	rst = curl_easy_setopt( p_ctx, CURLOPT_USERNAME, user.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ret;
	}

	rst = curl_easy_setopt( p_ctx, CURLOPT_PASSWORD, pswd.c_str() );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
		return ret;
	}

	m_is_running = true;
	rst = curl_easy_perform( p_ctx );
	if( rst != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( rst ));
	}else{
		if( dwld_cb ){
            dwld_cb( STATUS_TRANS_FINISH , nullptr , 0 );
		}

		ret = OK;
	}

	return ret;
}

int curlFtp :: do_process_save_file( stTransmit * desc )
{
        int ret = 0;

        if( desc != nullptr && desc->p_os != nullptr ){
                if( desc->p_os->write( ( char *)desc->p_buffer , desc->m_data_size ).bad() == false ){
                        ret = desc->m_data_size;
                }
        }

        return ret;
}

void curlFtp :: do_process_cmd( stTransmit * desc )
{
	(void)desc;
}

int curlFtp :: on_recv( stTransmit * desc )
{
        int ret = 0;
        if( desc == nullptr ) return 0;

        if( m_is_running == true ){ // 正在传输文件
                if( m_pause == triTrue ){               // 如果在传输的过程中启动暂停的操作
                        ret = CURL_WRITEFUNC_PAUSE;
                }else if( m_pause == triFalse ){        // 取消正在执行的传输操作
                        ret = ERR_CANCEL_TRANSFER;
                }else{ // 执行传输操作
                        ret = do_process_save_file( desc );
                }
        }else{ // 指令操作
                do_process_cmd( desc );
        }

        return ret;
}

/// @note 这个函数中采用重复分配的方式，可能会引起性能下降
int curlFtp :: on_recv_data( stTransmit * desc )
{
    int ret = 0;
    if( desc == nullptr ) return 0;
	// 执行传输操作
	if( desc->p_final_buffer != nullptr ){
		memcpy( desc->p_final_buffer + desc->m_final_off , desc->p_buffer , desc->m_data_size );
		desc->m_final_off += desc->m_data_size;
		ret = desc->m_final_off;
    }else{ // 如果这个指针是空，则可以调用回调函数通知进行处理
        if( dwld_cb ){
            dwld_cb(STATUS_TRANSING , desc->p_buffer , desc->m_data_size );
        }
	}

        return ret;
}

size_t curlFtp ::  __size( const std::string& file )
{
	size_t ret = 0;
	if( p_ctx ){
		if( file.find( '/' ) != std::string::npos || file.find( '\\' ) != std::string::npos ){

		}else{
			std::string cmd = "SIZE " + file;
			CURLcode e = curl_easy_setopt( p_ctx , CURLOPT_HEADER , 1 );
			if( CURLE_OK != e ){
				ERROR_MSG( "Enable command port fail" );
				return ret;
			}
			e = curl_easy_setopt( p_ctx , CURLOPT_HEADERFUNCTION, on_ftp_size );
			if( CURLE_OK != e ){
				ERROR_MSG( "Set header callback fun fail" );
				return ret;
			}
			e = curl_easy_setopt( p_ctx ,  CURLOPT_HEADERDATA , this );
			if( CURLE_OK != e ){
				ERROR_MSG( "Set header callback object fail" );
				return ret;
			}

			struct curl_slist *cmdlist = nullptr;

			cmdlist = curl_slist_append(cmdlist, cmd.c_str() );
			e = curl_easy_setopt( p_ctx , CURLOPT_QUOTE , cmdlist );
			if( e != CURLE_OK ){
				ERROR_MSG( "Set ftp size query command fail." );
				return 0;
			}
			e = curl_easy_perform( p_ctx );
			if( e != CURLE_OK ){
				ERROR_MSG( "perform ftp size query fail." );
				return 0;
			}

			ret = m_file_size;
			if( cmdlist ){
				curl_slist_free_all( cmdlist );
			}
		}
	}
	return ret;
}

void curlFtp :: onSize( const char * data , size_t size )
{
	if( data == nullptr ){
		m_file_size = 0;
		return;
	}
	if( size == 0 ) {
		m_file_size = 0;
		return;
	}
	// 准备数据
	std::stringstream ss;
	ss << data;
	MSG_1( "Get size return data: %s" , TNORMAL , data );
	std::string str;
	std::regex reg( R"(\d+\s+\d+)");
	std::smatch m;

	do{
		getline( ss , str );
		if( str.empty() == false ){
			if( std::regex_search( str , m , reg ) == true ){
				std::stringstream ss1;
				str = m[ 0 ];
				str = str.substr( str.find( ' ') , str.length() );
				ss1 << str;
				ss1 >> m_file_size;
				curl_easy_setopt( p_ctx , CURLOPT_HEADERFUNCTION , nullptr );
				break;
			}else{
				m_file_size = 0;
			}
		}else{
			m_file_size = 0;
		}
	}while( ss.eof() == false );

	if( m_file_size == 0 && ss.eof() ){
		ERROR_MSG( "Query ftp file size fail, pls check if the file exist." );
	}
}

void curlFtp :: do_process_error( errCode e )
{
        switch( e ){
        case OK:
	default:break;
        case ERR_CANCEL_TRANSFER:
		ERROR_MSG( " User canceled tranfer." );
                break;
        case ERR_UNSUPPORTED_PROTOCOL:
		ERROR_MSG( "Unsupported protocol." );
                break;
        case ERR_INIT_CURL:
		ERROR_MSG( " Initialize curl context fail." );
                break;
        case ERR_CONNECT_SVR:
		ERROR_MSG( " Can not connect to the server." );
                break;
        case ERR_FILE_DOWNLOAD_TRANSFER:
		ERROR_MSG( " Download transfer fail." );
                break;
        case ERR_FILE_UPLOAD_TRANSFER:
		ERROR_MSG( " Upload transfer fail." );
                break;
        case ERR_CTX_NULL:
		ERROR_MSG( " CURL context can not be null." );
                break;
	case ERR_ALLOC_MEM:
		ERROR_MSG( " Allocate memory fail." );
		break;
	case ERR_CREATE_DEST_FILE:
		ERROR_MSG( " Can not create destination file." );
		break;
	case ERR_CURL_SET_OPT:
		break;
        }
}

void curlFtp::setUser( const std::string& acc , const std::string& pswd )
{
	m_usr = acc;
	m_pswd = pswd;
}

bool curlFtp::__connect()
{
	bool ret = false;
	if( p_ctx ){
		std::stringstream str_url;
		str_url << "ftp://"<< m_usr<< ":"<< m_pswd<< "@"<< m_remote_add	<< ":"	<< m_port;
		std::string url = str_url.str();
		CURLcode  e = curl_easy_setopt( p_ctx ,	CURLOPT_URL ,url.c_str() );
		if( CURLE_OK != e ){
			ERROR_MSG( " Set CURLOPT_URL fail" );
			return ret;
		}

		struct curl_slist *cmdlist = nullptr;

		cmdlist = curl_slist_append(cmdlist, "TYPE I");
		e = curl_easy_setopt( p_ctx , CURLOPT_QUOTE , cmdlist );
		if( CURLE_OK != e ){
			ERROR_MSG( " Send HELP command fail" );
			return ret;
		}
		e = curl_easy_setopt( p_ctx ,
				      CURLOPT_WRITEFUNCTION ,
				      onConnectCb );
		if( CURLE_OK != e ){
			ERROR_MSG( " Set callback function fail" );
			return ret;
		}
		e = curl_easy_setopt( p_ctx , CURLOPT_PROTOCOLS , CURLPROTO_FTP );
		if( e != CURLE_OK ){
			ERROR_MSG( " Set protocols fail");
			return ret;
		}
		e = curl_easy_setopt( p_ctx , CURLOPT_WRITEDATA, this );
		if( CURLE_OK != e ){
			ERROR_MSG( " Set callback object fail");
			return ret;
		}

		e = curl_easy_perform( p_ctx );
		if( CURLE_OK != e ){
			ERROR_MSG( " Connect remote ftp server fail " );
			return ret;
		}
		if( cmdlist != nullptr )
			curl_slist_free_all( cmdlist );
		ret = __chk_svr();
	}
	return ret;
}

bool curlFtp::__chk_svr()
{
	bool ret = false;
	CURLcode e;
	e = curl_easy_setopt( p_ctx , CURLOPT_HEADER , 1 );
	if( CURLE_OK != e ){
		ERROR_MSG( " Enable command port fail" );
		return ret;
	}
	e = curl_easy_setopt( p_ctx , CURLOPT_HEADERFUNCTION, onChkInst );
	if( CURLE_OK != e ){
		ERROR_MSG( " Set header callback fun fail" );
		return ret;
	}
	e = curl_easy_setopt( p_ctx ,  CURLOPT_HEADERDATA , this );
	if( CURLE_OK != e ){
		ERROR_MSG( " Set header callback object fail" );
		return ret;
	}
	struct curl_slist *cmdlist = nullptr;

	cmdlist = curl_slist_append(cmdlist, "HELP");
	e = curl_easy_setopt( p_ctx , CURLOPT_QUOTE , cmdlist );
	if( CURLE_OK != e ){
		ERROR_MSG( " Set CHECK fail" );
		return ret;
	}
	e = curl_easy_perform( p_ctx );
	if( CURLE_OK != e ){
		ERROR_MSG( " Check server fail " );
		return ret;
	}
	if( cmdlist != nullptr )
		curl_slist_free_all( cmdlist );
	e = curl_easy_setopt( p_ctx , CURLOPT_HEADER , 0 );  // 关闭服务器探测
	if( CURLE_OK != e ){
		ERROR_MSG( " Enable command port fail" );
		return ret;
	}
	ret = true;
	return ret;
}

void curlFtp :: __onConnect( const char * data , size_t len )
{
	if( data == nullptr ) return;
	if( len == 0 ) return;
	// TODO: 处理接收到的内容数据
}

void curlFtp :: __onConnectErr()
{
	ERROR_MSG( " set ftp obj fail" );

}

void curlFtp ::  __onChkInst( const char * data , size_t len )
{
	(void)len;
	if( data != nullptr )
		std::cout << data << std::endl;
}

curlFtp :: errCode
curlFtp :: __real_upload( const std::string& local ,const std::string& remote )
{
	errCode ret = OK;
	CURL * ctx = curl_easy_init();

	if( ctx != nullptr ){
		stTransmit trans;
		try{
			__init_upload_trans_info( local , remote , trans );
		}catch( errCode e ){
			do_process_error( e );
			if( trans.p_buffer )
				free( trans.p_buffer );
			curl_easy_cleanup( ctx );
			return e;
		}
		CURLcode e = __init_upload_curl( ctx , trans );
		if( CURLE_OK != e ){
			ERROR_MSG( curl_easy_strerror( e ) );
			if( trans.p_buffer )
				free( trans.p_buffer );
			curl_easy_cleanup( ctx );
			return ERR_CURL_SET_OPT;
		}
		e = curl_easy_perform( ctx );
		if( CURLE_OK != e ){
			ERROR_MSG( curl_easy_strerror( e ) );
			if( trans.p_buffer )
				free( trans.p_buffer );
			curl_easy_cleanup( ctx );
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup( ctx );
	}else{
		ERROR_MSG(" Initialize curl context handle fail." );

		ret = ERR_INIT_CURL;
	}
	return ret;
}

CURLcode curlFtp :: __init_upload_curl( CURL * ctx , stTransmit& trans )
{
	CURLcode ret = CURLE_OK;
	MSG_1( "上传文件：%s" , TNORMAL , trans.m_dest.c_str() );

	ret = curl_easy_setopt( ctx , CURLOPT_URL , trans.m_dest.c_str() );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}
	ret = curl_easy_setopt( ctx , CURLOPT_UPLOAD , 1L );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_USERNAME, m_usr.c_str() );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_PASSWORD , m_pswd.c_str() );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_READDATA , &trans );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_READFUNCTION , ftp_read_callback );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_XFERINFOFUNCTION, on_upld_progress );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_XFERINFODATA , &trans );
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}

	ret = curl_easy_setopt( ctx , CURLOPT_NOPROGRESS, 0L);
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}
	ret = curl_easy_setopt(ctx, CURLOPT_FTPPORT, "-");
	if( ret != CURLE_OK ){
		ERROR_MSG( curl_easy_strerror( ret ) );
		return ret;
	}
	return ret;
}
std::ifstream curlFtp :: __init_local_file( const std::string& file )
{
	std::ifstream is;
	if( ::access( file.c_str() , F_OK ) == 0 ){
		is.open( file.c_str() , std::ios_base :: in | std::ios_base :: binary );

		if( is.is_open() == false ){
			throw ERR_OPEN_LOC_FILE;
		}
	}

	return is;
}

void curlFtp :: __init_upload_trans_info( const std::string& local , const std::string& remote , stTransmit& trans )
{
	trans.p_ftp = this;
	trans.m_from = local;
	if( remote.find( "ftp://") == std::string::npos ){
		std::stringstream ss;
		ss << "ftp://" << m_remote_add << ":" << m_port  << remote;
		trans.m_dest = ss.str();
	}else{
		trans.m_dest = remote;
	}
	trans.m_type = triFalse;
	trans.m_is = __init_local_file( local );

	trans.p_buffer = nullptr;
	trans.m_buf_size = 0;
	trans.m_data_size = 0;
}

std::string
curlFtp :: errMsg( errCode e )
{
	std::string ret;
	switch( e ){
	case ERR_CANCEL_TRANSFER:
		ret = "用户终端传输";
		break;
	case ERR_UNSUPPORTED_PROTOCOL:
		ret = "CURL不支持指定协议";
		break;//
	case ERR_INIT_CURL:
		ret = "初始化CURL错误";
		break;//
	case ERR_CONNECT_SVR:
		ret = "连接服务器错误";
		break;//
	case ERR_FILE_DOWNLOAD_TRANSFER:
		ret = "下载传输过程错误";
		break;//
	case ERR_FILE_UPLOAD_TRANSFER:
		ret = "上传文件传输过程错误";
		break;//
	case ERR_CTX_NULL:
		ret = "CURL 上下文对象空";
		break;//
	case ERR_ALLOC_MEM:
		ret = "内存分配失败";
		break;			    //
	case ERR_CREATE_DEST_FILE:
		ret = "创建目的文件失败";
		break;               //
	case ERR_CURL_SET_OPT:
		ret = "配置CURL参数失败";
		break;                   //
	case ERR_USER_OR_PSWD:
		ret = "用户名或者密码错误";
		break;                   //
	case ERR_CURL_PERFORM:
		ret = "执行CURL失败";
		break;                   //
	case ERR_OPEN_LOC_FILE:
		ret = "打开本地文件失败";
		break;                  //
	case ERR_NO_SRC_FILE:
		ret = "指定文件在服务器上不存在";
		break;
	case ERR_DESC_NULL:
		ret = "目标为空";
		break;
	case ERR_EMPTY_URL:
		ret = "空URL";
		break;
	case ERR_FILE_NOT_FOUND:
		ret = "找不到文件";
		break;                // 远程文件不存在
	case ERR_ACCESS_NOT_SUPPORT:
		ret = "ACCESS不支持";
		break;             //              // 数据传输完成
	case STATUS_TRANSING:
		ret = "数据正在传输";
		break;                    //
	case ERR_INIT_FTP:
		ret = "初始化FTP失败";
		break;
	default:break;
	}
	return ret;
}
