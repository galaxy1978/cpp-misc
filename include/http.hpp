/**
 * @brief HTTP模块,支持GET POST两种操作。如果需要考虑HTTPS 那么这个模块同样可以实现
 * @version 1.0
 * @date 2018-3-20
 * @author 宋炜
 */
#ifndef HTTP_HPP
#define HTTP_HPP
#include <string>
#include <atomic>
#include <functional>

#include <curl/curl.h>
namespace wheels
{
class http
{
	friend size_t http_write_callback(char *ptr, size_t size, size_t nmemb, void *obj );
public:
	enum emErrCode{
		ERR_ALLOC_MEM = -1000,
		OK = 0
	};
   	 enum mtd{ GET , POST };
private:
	std::string         m_server;           // 服务器
	std::string         m_url;              // URL
	std::string         m_head;             // 头信息
	std::string         m_cert;             // 客户端数字证书路径
	std::string         m_caPath;           // 用来验证服务器的CA
	mtd                 m_mtd;              // 数据提交方法

	int                 m_port;             // 连接端口
	bool                m_https;	    	// https
	bool                m_keepalive;        // http keepalive

	int                 m_respond_overtime; // 操作超时时间

    CURL                * ptctx;            // 使用CURL 作为HTTP后台操作的协议层引擎。
	struct curl_slist   * ptHeaders;        // HTTP头信息结构

	std::function< void ( const char * rst , size_t len ) >  __m_cb;

public:
	http();
	virtual ~http();
	/**
	 * @brief 设置服务器地址，这个地址可以是域名也可以是IP地址
	 * @param svr , 服务器地址
	 * @param port , 服务器监听端口
	 */
	http& server( const std::string& svr  );
	const std::string server( int &port )const;
	http& server( const std::string& svr , int port );
	/**
	 * @brief 设置URL，注意这个URL必须是经过URL格式化的数据。
	 * @note 千万注意不要传递没有经过URL格式化的数据
	 */
	http& url( const std::string& url  );
	const std::string url( ) const;
	/**
	 * @brief 设置头信息。
	 * @param head[ I ]， 头信息内容。可以一次提交多行的头信息，行之间以\n区分；也可
	 *   分多次配置头信息，每次可以配置一行或者多行都可以。
	 * @note 当分多次配置如果要读取行信息只能拿到最后一次的配置内容
	 */
	http& head  ( const std::string& head );
	const std::string head( ) const;
	/**
	 * @brief 配置证书路径
	 */
	http& cert  ( const std::string& cert );
	const std::string cert()const;
	/**
	 * @brief 开启则使用Https协议
	 * @param ssl
	 * @return
	 */
	http& ssl( bool ssl );
	bool ssl()const;
	/**
	 */
	http& port( int port );
	int port()const;
	/**
	 */
	http& keepalive( bool keep );
	bool keepalive()const;
	/**
	 * @brief get
	 * @param query
	 * @param fun
	 * @return
	 */
	int get( const std::string& query , std::function< void ( const char * rst , size_t len ) > fun );
	/**
	 * @brief post
	 * @param query
	 * @param fun
	 * @return
	 */
	int post( const std::string& query , std::function< void ( const char * rst , size_t len ) > fun );
	/**
	 * @brief 配置通讯超时时间，单位为s.在默认的情况下超时时间为10s
	 * @param t[ I ], 新的超时时间
	 */
	inline http& timeOut( long t )
	{
		m_respond_overtime = t;
		return *this;
	}
	/**
	 * @brief 获取当前的超时时间
	 * @return
	 */
	inline long timeOut(){ return m_respond_overtime; }
    /**
     * @brief __on_recv
     * @param ptr
     * @param size
     */
    void __on_recv( const char *ptr, size_t size );

private:
	/**
	 */
	int do_post( const char * data );
	int do_post( const std::string& data );
	/**
	 */
	int do_get( const char* data );
	int do_get( const std::string& data );
	/**
	 * @brief 初始化HTTP头信息
	 */
	int init_headers();
	/**
	 * @brief 构造HTTP 头结构体
	 */
	void init_header_struct();
	/**
	 * @brief 分析字符串，找出换行符号的位置。
	 * @param str, 要分析的字符串
	 * @param line_width ， 换行符号的宽度。
	 * @note 换行符号在不同的操作系统下会有不同的定义，在windows下为\r\n， 其宽度为2
	 *       在*unix下通常为\n，宽度为1.
	 * @return 返回换行符号的的位置。如果是回车换行符号则返回回车符号的位置
	*/
	size_t find_line_pos( const std::string& str , size_t &line_width );
};
}
#endif // HTTP_HPP
