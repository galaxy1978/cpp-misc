#include <sys/stat.h>
#include <algorithm>

#include "fs.hpp"
#include "logFile.hpp"

using namespace wheels;

const size_t logFile :: MAX_LINE_LEN = 160;

void logFile :: configure( size_t maxF , size_t maxLine , const std::string& logroot )
{
	__m_max_line = maxLine;
	__m_max_files = maxF;
	__m_log_root = logroot;

	while( __m_file.size() >= __m_max_files ){
		bool rst = removeFile( __m_log_root + PATH_DILMTER + "log" , __m_file.begin()->m_file_name , false );
		if( rst ){
			__m_file.erase( __m_file.begin() );
		}
	}
}

int logFile :: overflow( int c )
{
	int ret = 0;
	char d = ( char )c;
	auto it = __m_file.rbegin();
	std::ofstream * ofs = it->pt_ofs.get();

	if( d != '\n' ){
		__m_data_buf += d;
	}else if( ofs && ofs->is_open() ){
		ofs->write( ( __m_data_buf  + "\n" ).c_str() , __m_data_buf.length() + 1 );
		it->m_curr_line ++;

		if( it->m_curr_line + 0.1 * __m_max_files > __m_max_line ){
			if( __p_current_file == nullptr ){
				std::thread thd([&]{
						std::scoped_lock< std::mutex>  l( __m_mutex );	
						__new_file();
					});
				thd.join();
			}
			if( it->m_curr_line > __m_max_line ){
				std::scoped_lock< std::mutex>  l( __m_mutex );
			
				ofs->close();
				it->pt_ofs.reset( );
				it->m_is_current = false;
				if( __p_current_file ){
					stFile item( __m_file_name );
					item.pt_ofs.reset( __p_current_file );
					__p_current_file = nullptr;
					item.m_is_current = true;
	
					item.m_curr_line = 0;
					__m_file.rbegin()->m_is_current = false;
	
					__m_file.push_back( item );
				}
			}
		}
		__m_data_buf = "";
	}
	return ret;
}

bool logFile :: __new_file()
{
	bool ret = false;
	std::string file;
	
	file = __make_file_name();
	__m_file_name = file;
	__p_current_file = new std::ofstream( ( __m_log_root + PATH_DILMTER + "log" + PATH_DILMTER + file).c_str());
	
	if( __m_file.size() >= __m_max_files ){
		ret = removeFile( __m_log_root + PATH_DILMTER + "log", __m_file.begin()->m_file_name , false );
		if( ret ){
			__m_file.erase( __m_file.begin() );
		}
	}

	return ret;
}

std::string logFile :: __make_file_name()
{
	std::string name = nowStr();

	size_t pos;
	do{
		pos = name.find( ":" );
		if( pos != std::string::npos ){
			name.replace( pos , 1 , "_" );
		}
	}while( pos != std::string::npos );

	do{
		pos = name.find( " " );
		if( pos != std::string::npos ){
			name.replace( pos , 1 , "_" );
		}
	}while( pos != std::string::npos );

	do{
		pos = name.find( "." );
		if( pos != std::string::npos ){
			name.replace( pos , 1 , "_" );
		}
	}while( pos != std::string::npos );

	name += ".txt";
	
	return name;
}

bool logFile :: __create_new_log( const std::string& file )
{
	bool ret = false;
	try{
		stFile item( file );
		item.pt_ofs.reset(
			new std::ofstream(
				( __m_log_root + PATH_DILMTER + "log" + PATH_DILMTER + file).c_str() ,
				std::ios_base::out ));
			item.m_is_current = true ;
			item.m_curr_line = 0;

			__m_file.push_back( item );
	    
		ret = true;
	}catch( std::bad_alloc& e ){
		ERROR_MSG( e.what() );
	}
	return ret;
}

std::string logFile :: __find_old_file( const std::string& path )
{
	std::string ret;
	if( __m_file.size() == 0 ){
		__init_dir( path );
	}

	while( __m_file.size() >= __m_max_files ){
		bool rst = removeFile( __m_log_root + PATH_DILMTER + "log" , __m_file[ 0 ].m_file_name , false );
		if( rst ){
			__m_file.erase( __m_file.begin() );
		}
	}
	ret = __make_file_name();

	return ret;
}

bool logFile :: __init_dir( const std::string& path )
{
	bool ret = false;
	fs::fileTree root;
	std::string p = path + PATH_DILMTER + "log";
	if( access( p.c_str() , F_OK ) == 0 ){
		auto e = fs::enumFile( p , root );
		if( e != fs::OK ){
			return false;
		}

		root.for_each( [&]( fs::fileTree::treeNode & node )->bool{
				const fs::stFileItem * f = node.value_ptr();
				if( f ){
					std::string name = f->m_name;
					stFile item( name );

					__m_file.push_back( item );
				
					return true;
				}

				return false;
			}
		);
		
		ret = true;
	}else{
#if defined( __WIN32 ) || defined( __WIN64 )		
		int rst = mkdir( p.c_str() );
#elif defined( __LINUX__ )
		int rst = mkdir( p.c_str() , S_IFDIR );
#endif
		ret = (rst == 0 );
	}

	if( __m_file.size() > 0 ){
		__sort_file();
	}
	return ret;
}

logFile :: logFile( const std::string& logroot , bool stdo , bool stde ):
	__m_max_line( 100000 ),__m_max_files( 10 ), __p_current_file( nullptr ), __m_log_root( logroot )
{
	if( __init_dir( __m_log_root ) == false ){
		throw ERR_INIT_DIR;
	}

	if( stdo ){
		__p_stdout = std::cout.rdbuf();
		std::cout.rdbuf( this );
	}

	if( stde ){
		__p_stderr = std::cerr.rdbuf();
		std::cerr.rdbuf( this );
	}

	std::string file;
	file = __make_file_name();
	if( file.empty() == false ){
		bool rst = __create_new_log( file );
		if( rst == false ){
			throw ERR_INIT_FILE;
		}
	}else{
		throw ERR_INIT_FILE;
	}
}

logFile :: ~logFile()
{
	__clear();
}

void logFile :: add( const std::string& data )
{
	auto d = data;
	auto it = __m_file.rbegin();
	*(it->pt_ofs) << data;
	size_t pos = 0;
	while( ( pos = d.find( "\n" ) )!= std::string::npos ){ 
		it->m_curr_line ++;

		if( it->m_curr_line >= __m_max_line ){
			std::thread thd([&]{
				__new_file();
				it->m_is_current = false;
			});
			thd.detach();
		}

		d = d.substr( pos + 1 );
	}
}

void logFile :: __clear()
{
	if( __m_file.size() > 0 ){
		auto it = __m_file.rbegin();
		it->pt_ofs->close();
		
		__m_file.erase( __m_file.begin() , __m_file.end() );
	}
}

void logFile :: __sort_file()
{
	std::sort( __m_file.begin() , __m_file.end() , [&]( const stFile& i , const stFile& j )->bool{
			if( i.m_file_name.compare( j.m_file_name ) < 0 ){
				return true;
			}

			return false;
		}
	);
}
