/**
 * @brief URL 解析器
 * @version 1.2.0
 * @date 2020-2-13 ~ 2021-3-28
 * @author 宋炜
 */

/// 2020-10-14     1.1.0     增加urlDict, 用来进行编码和解码
/// 2021-3-28 宋炜 ADDED 添加赋值运算符重载和拷贝构造函数
#ifndef __URL_HPP__
#define __URL_HPP__

#include <string>
#include <map>
#include "misc.hpp"

class URL
{
public:
        /// @brief 字典类型
        /// @{
	struct urlDict{
		char   m_key;        // 字符值
		bool   m_en;         // 是否进行编码

		//urlDict():m_key(0 ) , m_en( false ){}
	};
        /// @}
        class Param
        {
                friend class URL;
        private:
            	std::map< std::string , std::string >  m_dict;  // URL中的参数部分
        private:
		/**
		 * @brief 解析参数部分，将参数分成参数名和值
		 * @param query , 参数字符串
		 * @return 解析成功返回true，否则返回false
		 */
		bool parse( const std::string& query );
        public:
		/**
		 * @brief 默认的构造
		 */
		Param();
		/**
		 * @brief 构造参数对象，并解析参数串
		 * @param query , 参数字符串
		 * @exception 如果解析失败拋出ERR_PARSE_QUERY
		 */
		explicit Param( const std::string& query );
		explicit Param( const Param& b );
		explicit Param( Param&& b );
		virtual ~Param();

		Param& operator=( const Param& b );
		Param& operator=( Param&& b );
		/**
		 * @brief 解析参数串，并更新对象
		 * @param query , 参数串
		 * @return 成功返回true， 否则返回false
		 */
		bool operator()( const std::string& query )
		{
			if( query.empty() ) return false;
			m_dict.erase( m_dict.begin() , m_dict.end() );
			return parse( query );
		}
		/**
		 * @brief 读取变量值
		 * @param name ，变量名称
		 * @return 变量值
		 * @excpetion 如果变量名称空， 拋出ERR_EMPTY_PARAM_KEY
		 * @exception 如果变量不存在， 拋出ERR_KEY_NOT_EXIST
		 */
		const std::string operator[ ]( const std::string& name );
		/**
		 * @brief 读取变量数量
		 * @return 变量数量
		 */
		size_t size();
		/**
		 * @brief 清除内容
		 */
		void clear()
		{
			m_dict.erase( m_dict.begin() , m_dict.end() );
		}
		/**
		 * @brief 查找指定变量名
		 * @return 变量map的叠代器
		 */
		std::map< std::string , std::string >::iterator
		find( const std::string& key )
		{
			return m_dict.find( key );
		}
		/**
		 * @brief 读取叠代器结束位置
		 * @return
		 */
		std::map< std::string , std::string>::iterator
		end()
		{
			return m_dict.end();
		}
    	};
	// 错误代码
	enum emErrCode{
                       ERR_PARSE_QUERY = -1000,
                       ERR_EMPTY_PARAM_KEY,
                       ERR_EMPTY_URL,
                       ERR_KEY_NOT_EXIST,
                       OK = 0
	};
private:
	std::string     m_url;                       // 初始的URL

	std::string     m_protocol;                  // 协议字符串
	std::string     m_user;                      // 用户帐号
	std::string     m_passwd;                    // 用户密码
	std::string     m_host;                      // 主机
	std::string     m_port;                      // 端口
	std::string     m_query;                     // 请求串
	std::string     m_path;                      // URL路径
	std::string     m_hash;                      //

	Param           m_param;                     // 参数表
private:
	/**
	 * @brief 解析协议部分
	 * @param url ， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_protocol( const std::string& url );
	/**
	 * @brief 解析用户部分
	 * @param url , URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_user( const std::string& url );
	/**
	 * @brief 解析密码部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_password( const std::string& url );
	/**
	 * @brief 解析服务器部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_host( const std::string& url );
	/**
	 * @brief 解析端口部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_port( const std::string& url );
	/**
	 * @brief 解析参数部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_query( const std::string& url );
	/**
	 * @brief 解析HASH部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_hash( const std::string& url );
	/**
	 * @brief 解析路径部分
	 * @param url， URL字符串
	 * @return 成功解析返回true, 否则返回false
	 */
	bool parse_path( const std::string& url );
public:
	/**
	 * @brief 构造
	 */
	URL();
	/**
	 * @brief 构造对象并进行解析
	 * @param url , URL字符串
	 * @exception 如果url空，拋出ERR_EMPTY_URL
	 */
	explicit URL( const std::string& url );
	URL( const URL& b );
	URL( URL&& b );
	URL& operator=( const URL& b );
	URL& operator=( URL&& b );
	virtual ~URL();
	/**
	 * 读取协议部分
	 */
	const std::string Protocol() const;
        void Protocol( const std::string& p );
	/**
	 * 读取用户部分
	 */
	const std::string AuthUser() const;
        void AuthUser( const std::string& user );
	/**
	 * 读取密码部分
	 */
	const std::string AuthPswd() const;
        void AuthPswd( const std::string& pswd );
	/**
	 * 读取主机部分
	 */
	const std::string HostName() const;
        void HostName( const std::string& host );
        void HostName( std::string&& host );
	/**
	 * 读取端口部分
	 */
	int HostPort() const;
        void HostPort( int port );
	/**
	 * 读取路径部分
	 */
	const std::string Path() const;
        void Path( const std::string& path );
        /**
         * @brief 读取文件名称
         */
        const std::string File() const;
        void File( const std::string& file );
	/**
	 * 读取参数部分字符串
	 */
	const std::string Query() const;
        void Query( const std::string& q );
	/**
	 * 读取参数值
	 * @param key, 变量名
	 * @return 变量值
	 */
	const std::string Parameters( const std::string& key );
	/**
	 * @brief 读取参表
	 */
	void Params( Param& param );
	/**
	 * 读取HASH部分
	 */
	const std::string Hash() const;
	/**
	 * @biref 对字符串进行URL编码
	 * @param from ，待编码串
	 * @return
	 */
	static std::string Encode( const std::string& from );
	/**
	 * @brief 对字符进行解码
	 * @param from ， 待解码串
	 */
	static std::string Decode( const std::string& from );
	/**
	 * @brief 解析URL
	 * @param URL , 要解析的URL
	 * @return
	 */
	bool Parse( const std::string& url );
        /**
         * @brief 将URL对象输出成一个完整URL字符串
         * @return 成功操作返回数据对象，否则返回空对象
         */
        std::string toString();
};
#endif
