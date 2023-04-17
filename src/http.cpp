#include <iostream>
#include <sstream>

#include "ary_str.hpp"
#include "http.hpp"
#include "sys_var.hpp"
using namespace wheels;

size_t http_write_callback(char *ptr, size_t size, size_t nmemb, void *obj )
{
	if( obj == nullptr ) return CURLE_OK;
	if( ptr == nullptr ) return CURLE_OK;

	http * __obj = reinterpret_cast< http*> ( obj );

	size_t len = size * nmemb;

	__obj->__on_recv( ptr , len );

	return len;
}
// ***************************************************************************************************************
http::http( ):m_port( 80 ),m_https( false ),m_keepalive( true ), m_respond_overtime(10) ,ptctx(nullptr),ptHeaders( nullptr )
{
	CURLcode err = curl_global_init(CURL_GLOBAL_ALL);
	if( err == CURLE_OK ){
		ptctx = curl_easy_init();

		if( ptctx == nullptr ){
			ERROR_MSG( "初始化libcurl失败");
			abort();
		}
	}else{
		ERROR_MSG( "初始化libcurl失败");
		abort();
	}
}

http::~http()
{
	if( ptHeaders ){
		curl_slist_free_all( ptHeaders );
	}

	if( ptctx )
		curl_easy_cleanup( ptctx );

	curl_global_cleanup();
}

void http::__on_recv( const char *ptr, size_t size )
{
	if( nullptr == ptr || size == 0 ) return;
	if( __m_cb ){
		__m_cb( ptr , size );
	}
}

http& http::server( const std::string& svr )
{
	m_server = svr;
	return *this;
}

http& http::url( const std::string& url )
{
	m_url = url;
	return *this;
}

http& http::head( const std::string& head )
{
	m_head = head;

	init_header_struct();

	return *this;
}

http& http::cert( const std::string& cert )
{
	m_cert = cert;
	return *this;
}

http& http::ssl( bool ssl )
{
	m_https = ssl;
	return *this;
}

http& http::port( int port )
{
	m_port = port;
	return *this;
}

http& http::keepalive( bool keep )
{
	m_keepalive = keep;
	return *this;
}

int http::do_post( const char * data )
{
	int ret = 0;

	if( data == nullptr ){
		ERROR_MSG( "数据内容不能为空" );
		return -1;
	}
	std::string url;
	std::stringstream ss;
	curl_easy_setopt( ptctx , CURLOPT_WRITEDATA , this );
    if( m_https == true ){
        ss << "https://";
    }else{
        ss << "http://";
    }
    ss << m_server << ":" << m_port << m_url;
	url = ss.str();

	curl_easy_setopt( ptctx , CURLOPT_URL, url.c_str() );                      // 提交数据URL

	if( m_https == true ){   // 使用HTTPS
		curl_easy_setopt( ptctx , CURLOPT_PROTOCOLS, CURLPROTO_HTTPS );        // 使用HTTPS
		if( m_caPath.empty() ){                                                // 不验证服务器
			curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYPEER, false );
			curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYHOST, false );
		}else{//
			curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYPEER, true);              // 开启服务器验证
			curl_easy_setopt(ptctx , CURLOPT_CAINFO , m_caPath.c_str());        // 设置CA路径
		}
	}
	else{                    // 使用HTTP
		curl_easy_setopt( ptctx , CURLOPT_PROTOCOLS, CURLPROTO_HTTP );         // 使用HTTP协议
	}

	init_headers();                                                            // 设置头信息

	curl_easy_setopt( ptctx , CURLOPT_WRITEFUNCTION , http_write_callback );   // 设置接收数据回调处理函数
	curl_easy_setopt( ptctx , CURLOPT_HTTPPOST , 1 );                          // 提及方式为POST
	curl_easy_setopt( ptctx , CURLOPT_POSTFIELDS , data );					   // 要发送的数据
	curl_easy_setopt( ptctx , CURLOPT_TIMEOUT , m_respond_overtime );		   // 设置超时时间
	ret = curl_easy_perform( ptctx );

	return ret;
}

int http::do_post( const std::string& data )
{
	return do_post( data.c_str() );
}

int http::do_get( const char * data )
{
	int ret = 0;
	if( data == nullptr ){
		ERROR_MSG( "数据内容不能为空" );
		return -1 ;
	}
	
	std::string url;
	std::stringstream ss;
	curl_easy_setopt( ptctx , CURLOPT_WRITEDATA , this );
    if( m_https == true ){
        ss << "https://";
    }else{
        ss << "http://";
    }
    ss << m_server << ":" << m_port << m_url << data;
	url = ss.str();

	curl_easy_setopt( ptctx , CURLOPT_URL, url.c_str() );                      // 提交数据URL

	if( m_https == true ){   // 使用HTTPS
		curl_easy_setopt( ptctx , CURLOPT_PROTOCOLS, CURLPROTO_HTTPS );        // 使用HTTPS
		if( m_caPath.empty() ){                                                // 不验证服务器
			curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYPEER, false );
			curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYHOST, false );
		}else{//
            curl_easy_setopt(ptctx, CURLOPT_SSL_VERIFYPEER, true);             // 开启服务器验证
            curl_easy_setopt(ptctx, CURLOPT_CAINFO , m_caPath.c_str());        // 设置CA路径
		}
	}
	else{                   // 使用HTTP
		curl_easy_setopt( ptctx , CURLOPT_PROTOCOLS, CURLPROTO_HTTP );         // 使用HTTP协议
	}

	init_headers();                                                            // 设置头信息

	curl_easy_setopt( ptctx , CURLOPT_WRITEFUNCTION , http_write_callback );   // 设置接收数据回调处理函数
	curl_easy_setopt( ptctx , CURLOPT_HTTPGET , 1 );                           // 提及方式为POST
					   // 要发送的数据
	curl_easy_setopt( ptctx , CURLOPT_TIMEOUT , m_respond_overtime );		   // 设置超时时间

	ret = curl_easy_perform( ptctx );

	return ret;
}

int http::do_get( const std::string& data )
{
	return do_get( data.c_str() );
}

int http::init_headers()
{
	int ret = 0;
	if( ptHeaders )
		ret = curl_easy_setopt( ptctx , CURLOPT_HTTPHEADER , ptHeaders );

	return ret;

}

int http :: get( const std::string& query , std::function< void ( const char * rst , size_t len ) > fun )
{
	int ret = 0;
	__m_cb = fun;
	ret = do_get( query );
	return ret;
}

int http :: post( const std::string& query , std::function< void ( const char * rst , size_t len ) > fun )
{
	int ret = 0;
	__m_cb = fun;
	ret = do_post( query );
	return ret;
}

size_t http::find_line_pos( const std::string& str , size_t &line_width )
{
	size_t pos_ret = 0 , pos_nl = 0 , ret;
	pos_ret = str.find( "\r");
	pos_nl = str.find( "\n");
	if( pos_ret != std::string::npos && pos_nl != std::string::npos ){
		ret = pos_ret < pos_nl ? pos_ret : pos_ret;

		if( abs( (long)(pos_nl - pos_ret) ) == 1 ){
			line_width = 2;
		}else{
			line_width = 1;
		}
	}else{
		line_width = 1;
		ret = pos_nl;
	}

	return ret;
}

void http::init_header_struct()
{
	ArrayString  a_str;

	// 分析头信息结构，头信息的设置是按行分的。所以首先将头信息按行分成不同的条目

	::split( m_head , '\n' , a_str );
	// 生成头信息内容结构体
	for( auto item : a_str ){
		if( ptHeaders == nullptr ){
			ptHeaders = curl_slist_append( ptHeaders , item.c_str());
		}else{
			curl_slist_append( ptHeaders , item.c_str());
		}
	}
}
