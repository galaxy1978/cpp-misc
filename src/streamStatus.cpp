
#include <fstream>

#include "streamStatus.hpp"
#include "sys_var.hpp"

ssStatusBar :: ssStatusBar( QWidget * parent ):
	QStatusBar( parent ),
	__p_status( nullptr ),
	__m_en_log( false ),
	__m_log_type( 0 )
{
	std::shared_ptr< CSysVar > ptvar = GetOrCreateSysVar();

	if( ptvar ){
		ptvar->GetValue( "/log/enable" , __m_en_log );

		if( __m_en_log == true ){
			ptvar->GetValue( "/log/type" , __m_log_type );
			std::string path;
			ptvar->GetValue( "/log/path" , path );
			switch( __m_log_type ){
			case 0: // 覆盖掉就内容
				path += "log.txt";
				__pt_os = std::shared_ptr< std::ofstream >( new std::ofstream( path.c_str() , std::ofstream::trunc) );
			break;
			case 1: // 追加内容
				path += "log.txt";
				__pt_os = std::shared_ptr< std::ofstream >( new std::ofstream( path.c_str() , std::ofstream::app) );
			break;
			case 2:{ // 创建新文件
				std::string str = nowStr();
				size_t pos;
				do{
					pos = str.find( ":" );
					if( pos != std::string::npos ){
						str.replace( pos , 1 , "_" );
					}
				}while( pos != std::string::npos );

				path += str + ".txt";
				__pt_os = std::shared_ptr< std::ofstream >( new std::ofstream( path.c_str() , std::ofstream::out ) );
			}
			break;
			}

			if( !__pt_os ){
				throw ERR_CREATE_LOG;
			}
		}
	}
	__p_status = new QLabel( "" , this );

	//__p_status->setStyleSheet( "background-color: red;" );
	addPermanentWidget( __p_status );
}

int ssStatusBar :: overflow( int c )
{
	if( ( char )c != '\n' ){
		__m_data += ( char )c;
	}else{
		clearMessage();
		showMessage( QString::fromUtf8( __m_data.c_str()) );
		if( __pt_os ){ // 输出到记录文件
			*__pt_os << __m_data;
		}

		for( auto it = __m_dcrts.begin(); it != __m_dcrts.end(); it ++ ){
			if( __m_data.empty() == false ){
				it->second->push( __m_data );
			}
		}
		__m_data.clear();
	}

	return c;
}

void ssStatusBar :: setLabelColor( const std::string& color , const std::string& txt)
{
	std::stringstream ss;
	ss << "QLabel{border-radius:6px;width:12px;height:12px;background-color:" << color << ";color:white;}";
	__p_status->setStyleSheet( ss.str().c_str() );
	__p_status->setText( txt.c_str() );
}

bool ssStatusBar :: dcrtIt( const std::string& id , dcrt_t * dcrt )
{
	bool ret = false;
	if( id.empty() ){
		return ret;
	}
	if( dcrt == nullptr ){
		return ret;
	}

	auto it = __m_dcrts.find( id );
	if( it == __m_dcrts.end() ){
		__m_dcrts.insert( std::make_pair( id , std::unique_ptr<dcrt_t>( dcrt ) ) );
		ret = true;
	}
	return ret;
}
void ssStatusBar :: unDcrtIt( const std::string& id )
{
	if( id.empty() ) return;

	auto it = __m_dcrts.find( id );
	if( it != __m_dcrts.end() ){
		__m_dcrts.erase( it );
	}
}

void ssStatusBar :: mouseDoubleClickEvent(QMouseEvent * )
{
	for( auto it = __m_dcrts.begin(); it != __m_dcrts.end(); it ++ ){
		it->second->run();
	}
}
