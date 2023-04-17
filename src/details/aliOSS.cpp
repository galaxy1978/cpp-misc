#include <assert.h>
#include <unistd.h>

#if defined( __LINUX__ )
#include <sys/stat.h>
#endif
#include <memory>
#include <string>
#include <fstream>

#include "ary_str.hpp"
#include "sys_var.hpp"
#include "misc.hpp"
#include "details/aliOSS.hpp"

using namespace wheels;
using namespace AlibabaCloud::OSS;

aliOSS :: aliOSS( const std::string& )
{
	auto ptvar = GetOrCreateSysVar();
	if( !ptvar ){ throw ERR_SYS_VAR; }

    InitializeSdk();

	ptvar->GetValue( "/fileService/aliOSS/endpoint" , __m_endpoint );
    ptvar->GetValue( "/fileService/aliOSS/secId" , __m_acckeyId );
    ptvar->GetValue( "/fileService/aliOSS/secKey" , __m_acckeySec );
	ptvar->GetValue( "/fileService/aliOSS/bucketName" , __m_bktName );

	try{
        if( ::access( "./tmp" , F_OK ) != 0 ){
                mkdir( "./tmp"
#if defined( __LINUX__ )
					,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
#endif
				);
        }
        __pt_conf = std::make_shared< ClientConfiguration >();
        __pt_conf->verifySSL = false;
		
        __pt_client = std::make_shared<OssClient>( __m_endpoint , __m_acckeyId , __m_acckeySec , *__pt_conf );
	}catch( std::bad_alloc& e ){
		ERROR_MSG( e.what() );
		throw ERR_ALLOC_MEM;
	}
}

aliOSS :: aliOSS(const std::string& bucket , const std::string& endpoint , const std::string& acckeyId , const std::string& acckeySec ):
		__m_endpoint( endpoint ),    // 服务节点
		__m_acckeyId( acckeyId ),     // 访问服务身份ID
        __m_acckeySec( acckeySec ),    // 访问服务密码
		__m_bktName( bucket )      // bucket 名称，类似于windows下的盘符
{
	InitializeSdk();
	
	try{
        __pt_conf = std::make_shared< ClientConfiguration >();
        __pt_conf->verifySSL = false;
		
        __pt_client = std::make_shared<OssClient>( __m_endpoint , __m_acckeyId , __m_acckeySec , *__pt_conf );
	}catch( std::bad_alloc& e ){
		ERROR_MSG( e.what() );
		throw ERR_ALLOC_MEM;
	}
}
aliOSS :: ~aliOSS()
{
	ShutdownSdk();
}

bool aliOSS :: __real_download_file( const std::string& from , const std::string& dest )
{
	bool ret = false;
	std::string __dest , chkp_path;
	if( dest.empty() ){
		__dest = ::fileName( from );
	}else{
        __dest = dest;
	}

    chkp_path = "./tmp";

	DownloadObjectRequest request( __m_bktName, from, __dest, chkp_path);

	auto outcome = __pt_client->ResumableDownloadObject(request);
	
	if (outcome.isSuccess()) {    
		MSG_1( "文件%s下载成功" , TNORMAL , from.c_str() );
		ret = true;
	}else{ // 异常处理。
        MSG_1( "下载失败：%s" , TRED , outcome.error().Message().c_str() );
	 }

	return ret;
}
bool aliOSS :: download( const std::string& from , const std::string& dest  , bool autoRun ) 
{
	bool ret = false;
	if( autoRun ){
		return __real_download_file( from , dest );
	}else{
        __m_missions.push_back( [=]()->bool{
            std::string f = from , d = dest;
            return __real_download_file( f , d );
		} );
		ret = true;
	}
	return ret;
}

void aliOSS :: download( const std::string& from , const std::string& dest , std::function< bool ( errCode , size_t  ) > cb , bool autoRun )
{
	__m_cb = cb;
	if( autoRun ){
		std::thread thd([=]{
			std::string f = from , d = dest;
			bool rst = __real_download_file( f , d );
		       
			if( rst == true ){
				if( __m_cb ){
					__m_cb( OK , 0 );
				}
			}else{
				ERROR_MSG( "下载文件失败" );
				if( __m_cb ){
					__m_cb( ERR_DOWNLOAD_OSS , 0 );
				}
			}
		} );
		thd.detach();
	}else{
        __m_missions.push_back( [=]()->bool{
			std::string f = from , d = dest;
			bool rst = __real_download_file( f , d );
			if( rst == false ){
				ERROR_MSG( "下载失败" );
			}				

            return rst;
		} );

	}
}

bool aliOSS :: __real_download_file( const std::string& file , char * buffer )
{
	bool ret = false;
    GetObjectRequest request( __m_bktName, file);
	auto outcome = __pt_client->GetObject(request);
	if (outcome.isSuccess()) {
		auto& stream = outcome.result().Content();
		while (stream->good()) {
			stream->read(buffer, __m_buff_size.load() );
			auto count = stream->gcount();
			if( __m_cb ){
				__m_cb( OK , count );
			}
		}
	}else {
        MSG_1( "下载失败：%s", TRED , outcome.error().Message().c_str() );
		ret = false;
	}
	return ret;
}

void aliOSS :: download( const std::string& from , char * buffer , std::function< bool ( errCode , size_t ) > cb , bool autoRun )
{
	__m_cb = cb;
	assert( buffer );
    assert( !from.empty() );

	if( autoRun == true ){
		// 如果选择了立即执行，则按照线程的方式异步启动下载操作。完成后以回调函数的方式
		// 通知操作结果
		std::thread thd([=]{
			std::string f = from;
			auto rst = __real_download_file( f , buffer );
		       
			if( rst == false ){
				ERROR_MSG( "下载文件失败" );
				if( __m_cb ){
					__m_cb( ERR_DOWNLOAD_OSS , 0 );
				}
			}
		} );
		thd.detach();
	}else{
		// 如果不是立即执行，首先将生成函数对象，并将这个函数对象加入到任务表中。此后
		// 在run函数中执行调用操作
        __m_missions.push_back( [=]()->bool{
			std::string f = from;
			bool rst = __real_download_file( f , buffer );
			if( rst == false ){
				ERROR_MSG( "下载文件失败" );
				if( __m_cb ){
					__m_cb( ERR_DOWNLOAD_OSS , 0 );
				}
			}

            return rst;
		});
	}
}

bool aliOSS :: download( const ArrayString& from , const ArrayString& dest , bool autoRun ) 
{
	bool ret = true;

	if( from.size() != dest.size() ){
		ERROR_MSG( "目标文件数量和源文件数量不相符" );
		return false;
	}
	
	for( size_t i = 0; i < from.size(); i ++ ){
		bool rst = download( from[ i ] , dest[ i ]  , autoRun );
		if( rst == false ){
            MSG_1( "下载文件 %s 失败" , TNORMAL , from[ i ].c_str() );
			break;
		}
	}
	return ret;
}

void aliOSS :: download( const ArrayString& from , const ArrayString& dest , std::function< bool ( errCode , size_t ) > cb, bool autoRun )
{
	if( from.size() != dest.size() ){
		ERROR_MSG( "目标文件数量和源文件数量不相符" );
        return;
	}

	__m_cb = cb;
	
	for( size_t i = 0; i < from.size(); i ++ ){
		if( autoRun ){
			std::string f = from[ i ] , d = dest[ i ];
			bool rst = __real_download_file( f , d );
		       
			if( rst == true ){
				if( __m_cb ){
					__m_cb( OK , 0 );
				}
			}else{
				ERROR_MSG( "下载文件失败" );
				if( __m_cb ){
					__m_cb( ERR_DOWNLOAD_OSS , 0 );
				}
			}			
		}else{
            __m_missions.push_back( [=]()->bool{
				std::string f = from[ i ] , d = dest[ i ];
				bool rst = __real_download_file( f , d );
				if( rst == false ){
					ERROR_MSG( "下载失败" );
				}				
                return rst;
			} );
		}
	}
}

bool aliOSS :: __real_upload_file( const std::string& file , const std::string& rfile )
{
	bool ret = true;

	std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>( file.c_str() , std::ios::in | std::ios::binary );
	
    PutObjectRequest request( __m_bktName, rfile, content);

	/*（可选）请参见如下示例设置访问权限ACL为私有（private）以及存储类型为标准存储（Standard）。*/
	//request.MetaData().addHeader("x-oss-object-acl", "private");
	//request.MetaData().addHeader("x-oss-storage-class", "Standard");

	auto outcome = __pt_client->PutObject(request);
	if (!outcome.isSuccess()) {
        MSG_1( "上传文件失败：%s", TRED , outcome.error().Message().c_str() );
		return false;
	}
	
	return ret;
}

bool aliOSS :: upload( const std::string& file ,  const std::string& remote_file ,  bool autoRun ) 
{
	bool ret = true;
	assert( !file.empty() );
	
	std::string f = ::fileName( file );
	
	if( autoRun ){
        return __real_upload_file( file , remote_file.empty() ? f : remote_file.substr( 1 ) );
	}else{
        __m_missions.push_back([=]()->bool{
			bool rst = __real_upload_file( file , remote_file.empty() ? f : remote_file );
			if( rst == false ){
				ERROR_MSG( "上传文件失败" );
			}

			return rst;
		});
	}

	return ret;
}

void aliOSS :: upload( const std::string& file ,  const std::string& remote_file, std::function<bool ( errCode , size_t )> cb ,bool autoRun ) 
{
	assert( !file.empty() );
	
	std::string f = ::fileName( file );
	__m_cb = cb;
	if( autoRun ){
		std::thread thd([=]{
			bool ret = __real_upload_file( file , remote_file.empty() ? f : remote_file );
			if( __m_cb ){
				if( ret == false ){
                    __m_cb( ERR_UPLOAD_OSS , 0 );
				}else{
                    __m_cb( OK , 0 );
				}
			}
		});
		thd.detach();
		
	}else{
        __m_missions.push_back([=]()->bool{
			bool rst = __real_upload_file( file , remote_file.empty() ? f : remote_file );
			if( __m_cb ){
				if( rst == false ){
                    __m_cb( ERR_UPLOAD_OSS , 0 );
				}else{
                    __m_cb( OK , 0 );
				}
			}

			return rst;
		});
	}
}

bool aliOSS :: upload( const ArrayString& from , bool autoRun  ) 
{
	bool ret = true;

	for( size_t i = 0; i < from.size(); i ++ ){
		if( autoRun == true ){
			std::string tf = ::fileName( from[ i ] );
			ret = __real_upload_file( from[ i ] , tf );
			if( ret == false ){
				MSG_1( "%s 上传失败" , TRED , from[i].c_str() );
				break;
			}
		}else{
			std::string tf = ::fileName( from[ i ] );
			__m_missions.push_back([=]()->bool{
				std::string tf = ::fileName( from[ i ] );
                bool rst = __real_upload_file( from[ i ] , tf );
                if( rst == false ){
					MSG_1( "%s 上传失败" , TRED , from[i].c_str() );
				}
                return rst;
			});
		}
	}
	return ret;
}

void aliOSS :: upload( const ArrayString& file , std::function< bool ( errCode , size_t )> cb , bool autoRun ) 
{
    for( size_t i = 0; i < file.size(); i ++ ){
		if( autoRun == true ){
            std::string tf = ::fileName( file[ i ] );
            bool ret = __real_upload_file( file[ i ] , tf );
			if( ret == false ){
                MSG_1( "%s 上传失败" , TRED , file[i].c_str() );
				break;
			}
		}else{
            std::string tf = ::fileName( file[ i ] );
			__m_missions.push_back([=]()->bool{
                std::string tf = ::fileName( file[ i ] );
                bool rst = __real_upload_file( file[ i ] , tf );
                if( rst == false ){
                    MSG_1( "%s 上传失败" , TRED , file[i].c_str() );
				}

                return rst;
			}); 
		}
	}
}

bool aliOSS :: del( const std::string& rem ) 
{
	bool ret = true;
	DeleteObjectRequest request( __m_bktName, rem );

	auto outcome = __pt_client->DeleteObject(request);

	if (!outcome.isSuccess()) {
        ERROR_MSG( outcome.error().Message().c_str() );
		ret = false;   
	}
	
	return ret;
}

void aliOSS :: setUser( const std::string& acc , const std::string& pswd ) 
{
	__m_acckeySec = pswd;
	__m_acckeyId = acc;
}

void aliOSS :: addUrl( const std::string& url )
{
	ERROR_MSG( "aliOSS 模块 addUrl不可用" );
}

void aliOSS :: run() 
{
	for( auto i : __m_missions ){
		bool rst = i();
		if( rst == false ){
			break;
		}
	}
}

void aliOSS :: run( std::function< bool ( emErrCode , size_t , int , const std::string& ) > cb )
{
    __run_cb = cb;

	for( auto i : __m_missions ){
		bool rst = i();
		if( rst == false ){ break; }
	}
}

aliOSS :: errCode
aliOSS :: ls( ArrayString& rst ) 
{
	errCode ret = OK;

	ListObjectsRequest request( __m_bktName );
	auto outcome = __pt_client->ListObjects(request);

	if (!outcome.isSuccess())  {    
		/* 异常处理。*/
		ERROR_MSG( outcome.error().Message() );
		return ERR_LS_OSS_FILE;
	}else {
		for (const auto& object : outcome.result().ObjectSummarys()) {
			rst.push_back( object.Key());
		}      
	}
	return ret;
}

void aliOSS :: ls( std::function< void ( errCode , ArrayString& ) > cb ) 
{
	assert( cb );
	
	std::thread thd([=]{
		ListObjectsRequest request( __m_bktName );
		auto outcome = __pt_client->ListObjects(request);
		ArrayString rst;
		
		if (!outcome.isSuccess())  {    
			ERROR_MSG( outcome.error().Message() );
			if( cb ){
				cb( ERR_LS_OSS_FILE , rst );
			}
		}else {
			for (const auto& object : outcome.result().ObjectSummarys()) {
				rst.push_back( object.Key());
			}
			if( cb ){
				cb( OK , rst );
			}
		}
	});
	thd.detach();
	
}

aliOSS :: errCode
aliOSS :: access( const std::string& url , int type ) 
{
	errCode ret = OK;

	(void)type;
	
	auto outcome = __pt_client->DoesObjectExist( __m_bktName, url );
	if( !outcome ){
		ret = ERR_FILE_NOT_FOUND;
	}
		
	return ret;
}

aliOSS :: errCode
aliOSS :: cd( const std::string& path ) 
{
	errCode ret = OK;
	
	ERROR_MSG( "aliOSS 模块 addUrl不可用" );
	return ret;
}

size_t aliOSS :: size( const std::string& file ) 
{
	size_t ret = -1;

    auto outcome = __pt_client->ListObjects( __m_bktName , file );

	if( outcome.isSuccess() ){
		for (const auto& object : outcome.result().ObjectSummarys()) {
			if( object.Key() == file ){
                ret = object.Size();
				break;
			}
		}
	}
	
	return ret;
}

