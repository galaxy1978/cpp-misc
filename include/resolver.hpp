/**
 * @brief 域名解析器
 * @version 1.0
 * @date 2018-5-2
 * @author 宋炜
*/

#ifndef __RESOVER_HPP__
#define __RESOVER_HPP__
#include <memory.h>

#include <string>
#include <functional>

#include "uvlper.hpp"
#include "ary_str.hpp"

namespace wheels
{
class resolver

{
public:
	enum protocol{ IP4 , IP6 };
	enum err_code{
		ERR_BASE = -1000,
		ERR_ALLOC_MEM,
		ERR_UV_DNS_RESOLVE,
		OK
	};
    using emErrCode = err_code;
private:
	std::function< void ( struct addrinfo *)>  on_success;
	std::function< void ( int )> on_fail;

    int                     m_error;
    int                     m_state;
    std::atomic< bool >     m_finish;
    struct addrinfo       * pt_result;
private:
  /**	
   * @brief 释放内存. 
   * @note 这个函数使用递归调用的方式释放内存
   */
  void free_result( struct addrinfo* );
public:
	resolver();
	/**	
	 * @brief 对象构造函数，对象建立后立即进行域名解析。函数调用后立即返回，操作结果通过回调
	 *      函数获取
	 * @param url
	 * @param fun
	 * @param on_success
	 * @param on_fail
	 */
	resolver( const std::string& url , int port , bool type , std::function< void ( struct addrinfo * )> fun);
	resolver( const std::string& url , const std::string& service , std::function< void ( struct addrinfo * )> fun);
	resolver( const std::string& url ,
		int port ,
		bool type ,
		std::function< void ( struct addrinfo * ) > on_success ,
		std::function< void ( int )> on_fail );
	resolver( const std::string& url ,
		const std::string& service ,
		std::function< void ( struct addrinfo * ) > on_success ,
		std::function< void ( int )> on_fail );
	virtual ~resolver();
	/**
	 * @brief 解析给定的域名。
	 * @param fun ， 通过回调函数发挥addrinfo结构数据
	 * @param ips ， 返回点分制的字符串
	 */
	void resolve( const std::string& url , int port , bool type , std::function<void ( struct addrinfo * )> fun);
	void resolve( const std::string& url ,
			int port ,
			bool type , 
			std::function<void ( struct addrinfo * )> funok ,
			std::function<void ( int )> funfail
		);  
	void resolve( const std::string& url , const std::string&  service , std::function<void ( struct addrinfo * )> fun);
	void resolve( const std::string& url ,
			const std::string&  service ,
			std::function<void ( struct addrinfo * )> funok,
			std::function<void ( int )> funfail
		);
  
	void resolve( const std::string& url , int port , ArrayString& ips , protocol ptcol = IP4 );
	void resolve( const std::string& url , int port , struct addrinfo * res , protocol ptcol = IP4 );
	void resolve( const std::string& url , const std::string& service , ArrayString& ips , protocol ptcol = IP4 );
	void resolve( const std::string& url , const std::string& service , struct addrinfo * res , protocol ptcol = IP4 );
	/**
	 * @brief 
	 */
	void on_resolve( struct addrinfo * info );
	
	void seterror( int error ) { m_error = error; }
	
	void clear(){ pt_result = nullptr;}
	
	bool finish(){ return m_finish.load(); }

    static std::string errMsg( int err );
};
}
#endif
