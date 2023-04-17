#include <map>
#include "sys_var.hpp"

CSysVar :: CSysVar( const std::string& file ):CConfFile( file ){}

CSysVar :: ~CSysVar(  ){}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::map< std::string , std::shared_ptr<CSysVar> >  globe_conf_file;

std::shared_ptr<CSysVar>  GetOrCreateSysVar( const std::string& file  )
{
	auto it = globe_conf_file.find( file );
	if( it == globe_conf_file.end() ){
		auto pt = std::shared_ptr<CSysVar>( new CSysVar( file ) );
		globe_conf_file.insert( std::make_pair( file , pt ) );
	}
    std::weak_ptr< CSysVar> ptr( globe_conf_file.begin()->second );
    return ptr.lock();
}

std::shared_ptr<CSysVar>  GetOrCreateSysVar( )
{
    std::weak_ptr<CSysVar> ptr( globe_conf_file.begin()->second);
    return ptr.lock();
}

#if defined( __cplusplus )
extern "C"{
#endif
/**
 * @brief 讀取配置文件的接口。
 * @return 返回配置文件接口的ctx
 */
void * getOrCreateConfCtx( const char * path )
{
	std::shared_ptr< CSysVar > ptvar = GetOrCreateSysVar( path );
	if( ptvar ){
		return ptvar.get();
	}

	return nullptr;
}

bool getConfBool( void * ctx , const char * path )
{
	bool ret = false;

	assert( ctx );
	assert( path );

	CSysVar * conf = (CSysVar*)ctx;
	conf->GetValue( path , ret );
	
	return ret;
}
void getConfStr( void * ctx , const char * path , char * data )
{
	assert( ctx );
	assert( path );
	assert( data ); 

	CSysVar * conf = (CSysVar*)ctx;
	std::string str;
	conf->GetValue( path , str );

	strcpy( data , str.c_str() );
}
#if defined( __cplusplus )
}
#endif
