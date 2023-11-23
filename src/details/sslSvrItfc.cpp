#include "details/sslSvrItfc.hpp"

bool verify_cert(bool preverify_ok, X509_STORE_CTX* ctx){
	char cert_str[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx);
	X509_NAME_oneline(X509_get_subject_name(cert), cert_str, sizeof(cert_str));
		
	if (preverify_ok){
		MSG_1( "Certificate verification passed for: %s" , OK_MSG , cert_str );
	} else {
		MSG_1( "Certificate verification failed for: %s" , OK_MSG , cert_str );
	}

	// 获取SSL对象
	SSL* ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    
	// 获取客户端证书链
	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(ssl);
    
	// 验证客户端证书
	X509_STORE* store = X509_STORE_CTX_get0_store(ctx);
	if (store && chain) {
		int chain_length = sk_X509_num(chain);
		for (int i = 0; i < chain_length; ++i) {
			X509* cert = sk_X509_value(chain, i);
            
			// 对证书进行校验
			int verify_result = X509_verify_cert(store, cert);
			if (verify_result != 1) {
				std::cerr << "Certificate chain verification failed for: " << cert_str << std::endl;
				return 0;
			}
		}
	}
		
	return preverify_ok;
}

sslSvrItfc :: sslSvrItfc( const std::string& ca , const std::string& cert , const std::string& key ):
	p_ctx__( nullptr )
{
	// 初始化OpenSSL库
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	// 创建SSL上下文
	p_ctx__ = SSL_CTX_new(SSLv23_server_method());
	if (!ssl_ctx) {
		throw std::runtime_error( "Failed to create SSL context" );
	}

	// 加载CA证书文件
	if (SSL_CTX_load_verify_locations(ssl_ctx, ca.c_str(), nullptr) <= 0) {
		std::cerr << "Failed to load CA certificate file" << std::endl;
		ERR_print_errors_fp(stderr);
		return 1;
	}
	// 加载证书和私钥文件
	if (SSL_CTX_use_certificate_file( p_ctx__ , cert.c_str() , SSL_FILETYPE_PEM) <= 0) {
		std::cerr << "Failed to load certificate file" << std::endl;
		ERR_print_errors_fp(stderr);
		return 1;
	}
	if (SSL_CTX_use_PrivateKey_file( p_ctx__ , key.c_str() , SSL_FILETYPE_PEM) <= 0) {
		std::cerr << "Failed to load private key file" << std::endl;
		ERR_print_errors_fp(stderr);
		return 1;
	}

	// 设置验证回调函数
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_cert );
}

bool sslSvrItfc :: accept( native_socket sock )
{
	bool ret = false;

	SSL* ssl = SSL_new( p_ctx__ );
        SSL_set_fd(ssl, sock);

        // 建立SSL握手
        if ( SSL_accept(ssl) <= 0 ) {
		ERROR_MSG( "Failed to complete SSL handshake" );
		close( sock );
		SSL_free(ssl);
        }else{ // 构建
		auto * egn = sslEpollEgn::get();
		egn->add( sock );
		
		auto c = std::make_shared<>( ssl );
		m_connections__.insert( sock , ssl );
		
		ret = true;
	}
	
	return ret;
}

connection_t * sslSvrItfc :: get( native_socket h )
{
}
connection_t * sslSvrItfc :: operator[]( native_socket h )
{
}

void sslSvrItfc :: erase( native_socket h )
{
}
