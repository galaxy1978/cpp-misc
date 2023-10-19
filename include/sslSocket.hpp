/**
 * @brief SSL通讯模块.在后端使用epoll或者IOCP, 由于openssl的验证过程使用了同步的操作需要注意启动事件循环的时机
 * @version 1.0
 * @author 宋炜
 * @date 2023-9-11
 *
 * 暂时仅仅实现了epoll模式的接口
 */

#pragma once
// sslsocket使用了模板来实现，实际上使用了桥接模式
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "url.hpp"
#include "misc.hpp"
#include "mallocSharedPtr.hpp"

#include "details/socketbase.hpp"

namespace private__{ // 这里对private__定义了
	// 模板参数是tcpLinux或者tcpWindows或者可以定义其他的
	template< typename impType >
	class sslSocket__ : public itfcSocket // 也是从itfcSocket继承，需要实现其中的所有虚函数
	{
	public:
		using native_socket = itfcSocket::native_socket;
	private:
		std::shared_ptr< impType >     pt_impl__;    // 后端TCP模块对象指针，针对impType，就是实际系统接口的实现对象指针
		SSL_CTX                      * p_ctx__;      // SSL CTX
		SSL                          * p_ssl__;      // ssl连接
		std::atomic< native_socket >   m_socket__;   // 后端tcp套接字
		std::string                    m_addr__;     // 服务器地址
		std::atomic<uint16_t>          m_port__;     // 服务器端口号
		// 实际数据对外通知的回调函数对象
		std::function< void (const uint8_t * data , size_t len) >  m_cb__;
		
	private:
		/**
		 * @brief 创建SSL连接对象。是openssl SSL的准备工作
		 */
		SSL_CTX* create_ssl_ctx__( const std::string& ca,  const std::string& cert,  const std::string& key ){
			SSL_CTX* ctx = NULL;
			const SSL_METHOD* method;
    
			// 初始化OpenSSL库
			SSL_library_init();
			OpenSSL_add_all_algorithms();
			SSL_load_error_strings();

			// 创建 SSL 上下文
			method = TLS_client_method();
			ctx = SSL_CTX_new(method);
			if (!ctx){
				ERROR_MSG( "Unable to create SSL context" );
				ERR_print_errors_fp(stderr);
				return nullptr;
			}

			// 加载客户端证书和私钥，这个实现需要验证服务器，服务也应该要求验证客户端。进行双向验证
			if (SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM) != 1) {
				ERROR_MSG( "Failed to load client certificate!" );
				SSL_CTX_free(ctx);
				return nullptr;
			}
			if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) != 1) {
				ERROR_MSG( "Failed to load client private key!" );
				SSL_CTX_free(ctx);
				return nullptr;
			}

			// 设置根证书
			if(SSL_CTX_load_verify_locations(ctx, ca.c_str() , nullptr) != 1) {
				ERROR_MSG( "Failed to load CA certificate");
				ERR_print_errors_fp(stderr);
				return nullptr;
			}

			return ctx;
		}
		/**
		 * @brief 实际响应时间处理读取数据内容
		 */
		void read_data__( native_socket fd ) {
			UNUSED( fd );
				
			wheels::mallocSharedPtr< uint8_t >  buff( 1024 * 10 );
			int len = SSL_read( p_ssl__ , buff.get() , 1024 * 10  );
			if( len > 0 && m_cb__ ){
				m_cb__( buff.get() , len );
			}else if( !m_cb__ ){
				ERROR_MSG( "没有指定数据处理程序" );
			}
				
		} ;
		/**
		 * @brief 处理错误代码，呈现错误信息
		 * @param err[ I ], 错误代码
		 */
		void process_err__( int err ){
			switch( err ){
			case SSL_ERROR_NONE:
				ERROR_MSG( "处理其他错误" );
				break;
			case SSL_ERROR_ZERO_RETURN:
				ERROR_MSG( "处理连接关闭等错误" );
				break;
			case SSL_ERROR_WANT_READ:
				ERROR_MSG( "处理需要读取的错误" );
				break;
			case SSL_ERROR_WANT_WRITE:
				ERROR_MSG( "处理需要写入的错误" );
				break;
			case SSL_ERROR_SYSCALL:
				ERROR_MSG( "处理系统调用错误" );
				break;
			case SSL_ERROR_SSL:
				ERROR_MSG( "处理SSL协议错误" );
				break;
			default:
				// 处理其他错误代码
				break;
			}
		}
	public:
		/**
		 * @brief 构造sslsocket 
		 * @param url[ I ]
		 * @param port[ I ]
		 * @param ca[ I ]
		 * @param cert[ I ]
		 * @param key[ I ]
		 */
		sslSocket__( const std::string& url , uint16_t port , const std::string& ca , const std::string& cert , const std::string key ):
			m_addr__( url ),
			m_port__( port ) {
			if( ca.empty() ){
				throw std::runtime_error( "CA证书不能为空" );
			}

			if( cert.empty() ){
				throw std::runtime_error( "客户端证书文件不能为空" );
			}

			if( key.empty() ){
				throw std::runtime_error( "客户端私匙不能为空" );
			}

			SSL_library_init();
			SSL_load_error_strings();
			// 初始化后端具体TCP的实现
			pt_impl__ = std::make_shared< impType >( url , port );
			// 将TCP socket读出保存在类中
			native_socket sock = pt_impl__->getHandle();
			m_socket__ = sock;
			// 准备ssl连接
			p_ctx__ = create_ssl_ctx__( ca , cert ,key);

			
		}

		virtual ~sslSocket__(){
			// 这里是析构，先处理SSL
			if( p_ssl__ ){
				SSL_shutdown( p_ssl__);
				SSL_free( p_ssl__ );
			}
			// 关闭TCP
			if( pt_impl__ ){
				pt_impl__->close();
			}

			if( p_ctx__ ){
				SSL_CTX_free( p_ctx__ );
			}
		}
		/**
		 * @brief 连接服务器。实际连接操作
		 * @return 成功操作返回true，否则返回false
		 */
		virtual bool connect() final{
			bool rst =  pt_impl__->connect( m_addr__ , m_port__ );
			if( rst ){
				// 创建 SSL 对象，并绑定到 socket.
				p_ssl__ = SSL_new( p_ctx__ );
				SSL_set_fd(p_ssl__, m_socket__.load() );
				// 建立 SSL 连接
				int rst = SSL_connect(p_ssl__);
				if ( rst < 0 ) {
					int err = SSL_get_error(p_ssl__, rst);
					process_err__( err );
					return false;
				}
				// 服务器证书验证。验证服务器，通过才可以继续
				if (SSL_get_verify_result(p_ssl__) != X509_V_OK) {
					ERROR_MSG( "服务器证书验证失败" );
					rst = false;
				}else{ // 这里很重要，应在SSL握手成功后再配置读取数据的回调函数
				// 否则因为epoll函数的异步特性可能导致SSL握手失败
				// 同样的后端TCP，epoll事件也应在在连接成功再启动。
					MSG( "服务器证书验证成功" , OK_MSG );
					// 完成证书验证后将会掉函数调整到数据读取
					pt_impl__->onRecv( [=]( native_socket socket ){
						read_data__(socket );
					});
					rst = true;
				}
			}else{
				ERROR_MSG( "尝试TCP连接失败" );
			}

			return rst;
		}
		/**
		 * @brief 连接服务器
		 * @param url[ I ], 标准的URL格式字符串
		 */
		virtual bool connect( const std::string& url ) final{
			bool ret = false;
			URL __url( url );
			std::string addr;
			uint16_t port;

			addr = __url.HostName();
			port = __url.HostPort();
			
			ret = this->connect( addr , port );
			return ret;
		}
		/**
		 * @brief 连接服务器
		 * @param url[ I ],服务器地址
		 * @param port[ I ], 服务器监听端口
		 */
		virtual bool connect( const std::string& url , uint16_t port ) final{
			// 执行TCP连接操作
			bool ret = pt_impl__->connect( url , port );
			if( ret ){
				// 创建 SSL 对象，并绑定到 socket.
				p_ssl__ = SSL_new( p_ctx__ );
				SSL_set_fd(p_ssl__, m_socket__.load() );

				if (SSL_connect(p_ssl__) == -1) {
					ERROR_MSG( "SSL连接失败" );
					return false;
				}

				// 服务器证书验证
				if (SSL_get_verify_result(p_ssl__) != X509_V_OK) {
					ERROR_MSG( "服务器证书验证失败" );
					return false;
				}else{
					MSG( "服务器证书验证成功" , OK_MSG );
					// 完成证书验证后将回调函数调整到数据读取
					pt_impl__->onRecv( [=]( native_socket socket ){
						read_data__(socket );
					});
					ret = true;
				}
			}

			return ret;
		}
		/**
		 * @brief 发送数据内容
		 * @param data[ I ],要发送的数据内容
		 * @param len[ I ],要发送的数据长度
		 * @return 成功操作返回发送的数据长度
		 * @exception std::runtime_error
		 */
		virtual size_t send( const uint8_t * data , size_t len ) final{
			// 这里就是为什么不在epoll中实现发送的原因，因为没有必要
			int ret = SSL_write( p_ssl__ , data , len );
			if( ret < 0 ){
				throw std::runtime_error( "发送数据失败" );
			}
			return ret;
		}
		/**
		 * @brief 关闭连接
		 */
		virtual void close( ) final{
			if( p_ssl__ ){
				SSL_shutdown( p_ssl__);
				SSL_free( p_ssl__ );
			}
			pt_impl__->close();
		}
		/**
		 * @brief 配置接收数据的回调函数。对外通知
		 * @param fun[ I ]，回调函数对象, 函数原型为
		 *    void callback( const uint8_t * data , size_t len );
		 */
		virtual void onData( std::function< void (const uint8_t* data , size_t len)> fun ) final{
			m_cb__ = fun;
		};
		/**
		 * @brief 数据接收事件处理,读取数据内容。这里就不需要了，读取操作处理在上面
		 * @param fun[ I ]
		 */
		virtual void onRecv( std::function< void ( native_socket fd ) > fun ) final{
			UNUSED( fun );
		}

		/**
		 * @brief 启动异步通讯事件循环.必须在SSL握手后调用启动
		 * @param sw[ I ], true启动,false关闭
		 * @note **** 这个函数应该在建立ssl连接之后调用。在SSL握手期间一定不能启动，否则容易握手失败
		 */
		virtual void run( bool sw ) final{
			if( sw ){
				pt_impl__->run( true );
			}else{
				pt_impl__->run( false );
			}
		}
		/**
		 * @brief 获取本地socket句柄
		 */
		virtual native_socket getHandle() final{
			native_socket ret = 0;
			ret = pt_impl__->getHandle();
			return ret;
		}
		/**
		 * @brief 保存服务器CA证书
		 */
		bool getPeerCert( const std::string& file ){
			bool ret = false;
			if( !p_ssl__ ){
				ERROR_MSG( "ssl对象没有初始化" );
				return ret;
			}
			
			X509* cert = SSL_get_peer_certificate( p_ssl__ );
			if (cert) {
				STACK_OF(X509)* certChain = SSL_get_peer_cert_chain(ssl);

				if (certChain != nullptr && sk_X509_num(certChain) > 0) {
					X509* caCert = sk_X509_value(certChain, 0);
					FILE* file = fopen(file.c_str(), "wb");
					if (file) {
						PEM_write_X509(file, caCert );
						fclose(file);					
					} else {
						std::cerr << "Failed to save server certificate." << std::endl;
					}
				}

				X509_free(cert);
			} else {
                std::cerr << "Failed to get server certificate." << std::endl;
			}

			this->close();
		}
	};

}

// 通过宏定义分别处理不同的实现，对外实际使用了统一的名字
#if defined( WIN32 ) || defined( WINNT )
#include "details/tcpwin.hpp"
// 使用了tcpWin作为模板参数。对外名字为sslSocket
using sslSocket = private__::sslSocket__< tcpWin >;
#elif defined( __LINUX__ )
#include "details/tcplinux.hpp"
// 使用了tcpLinux作为模板参数，对外名字为sslSocket
// private__ 是一个名字空间，表示具体实现进行了名字限制
using sslSocket = private__::sslSocket__< tcpLinux >;
#endif

// 完，谢谢。