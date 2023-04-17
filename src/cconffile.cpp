//#include "defines.hpp"

#include "tinyxml.h"
#include <sstream>
#include <unistd.h>
#include <iostream>

#include "misc.hpp"
#include "cconffile.hpp"
/*
当前代码的版本号。这个版本号和文件中的VER字段对应
*/
static const char *CONFIG_FILE_VERSION = "1.0";

CConfFile ::
CPathTravler :: CPathTravler( const std::string& path , CConfFile* conf , TiXmlElement* /*current*/ )
{
	if( conf == NULL ){
		p_file = NULL;
		return;
	}

	p_current = conf->p_root;
	p_rst = p_current;
	p_file = conf;

	parse_path( path );       //
	run();
}


TiXmlElement* CConfFile ::
CPathTravler :: operator()()
{
	return p_rst;
}

TiXmlElement* CConfFile ::
CPathTravler :: operator()( const std::string& path )
{
	m_path_data.erase( m_path_data.begin() , m_path_data.end() );
	if( parse_path( path ) == 0 )
		run();
	return p_rst;
}


int CConfFile ::
CPathTravler :: parse_path( const std::string& path )
{
	int ret = 0;
	std::string str = path, tmp_str;

	if( path.empty() ){
		return PATH_DATA_EMPTY;
	}

    	m_path_data.erase( m_path_data.begin() , m_path_data.end() );

	do{
		size_t pos;
		pos = str.find( '/' );
		if( pos != std::string::npos ){
			if( pos != 0 )//是子目录
				tmp_str = str.substr( 0 , pos );
			else tmp_str.clear();//是根目录
			//拆分字符串，分成已经解析的和没有解析的
			str = str.substr( pos + 1 , std::string::npos );
			//将目录路径存储路径表
			if( tmp_str.empty() && !str.empty())//生成根目录路径
				tmp_str = "/";
			m_path_data.push_back( tmp_str );
		}
		else{//找不到'/'说明是叶子节点或者空节点
			if( str.empty() == false ){//是叶子节点，将叶子节点存入路径
				//表中。完成目录查找
				m_path_data.push_back( str );
				str.clear();
			}
		}
	}while( str.empty() == false );

    	return ret;
}

int CConfFile ::
CPathTravler :: run()
{
	int ret = OK;
	ArrayPath::iterator it = m_path_data.begin();
	if( it == m_path_data.end() ){
		ret = PATH_DATA_EMPTY;
	}
	for(; it < m_path_data.end() && ret == OK; ++it ){
		if( (*it) == "." ) continue;                        //忽略本地操作符号
		else if( (*it) == "/") ret = go_to_root();          //移动到根目录
		else if( (*it) == ".." ) ret = go_to_parent();      //移动到父目录
		else if( it != m_path_data.end() -- )               //移动到子目录
			ret = go_to_child((*it));
		else{                                               //移动到目标
			ret = go_to_brother( (*it));
		}
	}
	return ret;
}

TiXmlElement* CConfFile ::
CPathTravler :: Cd( const std::string& path )
{
	parse_path( path );

	run();

	return p_current;
}

int CConfFile ::
CPathTravler :: go_to_parent( )
{
	int ret = OK;
	if( p_current){
		if( p_current->Parent() ){  // 不是根结点
			p_current = (TiXmlElement*)(p_current->Parent());
		}else{
			if( p_file )
				p_current = p_file->p_root;
		}
	}else{
		if( p_file )
			p_current = p_file -> p_root;
	}



	return ret;
}

int CConfFile ::
CPathTravler :: go_to_root()
{
	int ret = OK;
	if( p_file )
		p_current = p_file->p_root;

	return ret;
}

int CConfFile ::
CPathTravler :: go_to_child( const std::string& child )
{
	std::string str , str1;
	int ret = 0;
	if( p_current )
		p_current = p_current->FirstChildElement();
	if( p_current != NULL ){
		while( p_current != NULL ){
			const char * c_str = p_current->Value();
			if( c_str == nullptr ){
				return ERR_GET_NAME;
			}
			str1.assign( c_str );

			if( str1 != "dir" && str1 != "var"){
				p_current = NULL;
				p_rst = NULL;
				return ERROR_FILE_DIR_FORMAT;
			}
			c_str = p_current->Attribute("name");
			if( c_str == nullptr ){
				ERROR_MSG( "找不到name属性" );
				return ERR_GET_ATTR;
			}
			str.assign( c_str );
			if( str == child ){
				ret = 0;
				p_rst = p_current;
				return ret;
			}
			p_current = p_current->NextSiblingElement();
		}

		if( p_current == NULL ){
			ret = PATH_NOT_EXIST;
		}else{
			ret = OK;
		}
	}else{
		ret = PATH_NOT_EXIST;
	}
	return ret;
}

int CConfFile ::
CPathTravler :: go_to_brother( const std::string& bro )
{
	std::string str;
	int ret = 0;

	if( p_current != NULL ){
		while( p_current != NULL ){
			const char * c_str = p_current->Attribute("name");
			if( c_str == nullptr ){
				ERROR_MSG( "找不到name属性" );
				return ERR_GET_ATTR;
			}
			str.assign( c_str );
			if( str == bro ){
				ret = 0;
				break;
			}
			p_current = p_current->NextSiblingElement();
		}
		if( p_current == NULL ){
			ret = PATH_NOT_EXIST;
			ERROR_MSG( bro  + " 不存在." );
		}else{
			ret = OK;
		}
	}
	else{
		ret = PATH_NOT_EXIST;
	}

	return ret;
}

CConfFile :: CConfFile( const std::string& file ):m_file()
{
	if( access( file.c_str() , F_OK ) != 0 ){
		throw ERR_FILE_NOT_EXIST;
	}
	m_doc.LoadFile( file );
	if( m_doc.Error() == false ){
		p_root = m_doc.FirstChildElement();//获取根节点
		p_current = p_root;
		if( p_root == NULL ){//获取根节点失败
			throw CAN_NOT_FIND_XML_ROOT;
		}
		else{//调整根节点位置
			if( check_file() ){
				//TODO , 留在将来调整文件根指针位置或者做其他调整
			}
			else{
				throw CAN_NOT_LOAD_FILE;
			}
		}
	}
	else{//装入文件失败
		std::string e_msg = m_doc.ErrorDesc();
		throw CAN_NOT_LOAD_FILE;
	}
}

void CConfFile :: Cd( const std::string& path )
{
	CPathTravler trler( path , this , p_current );

	p_current = trler();
}

bool CConfFile :: check_file()
{
	bool ret;
	std::string str = p_root->Value();
	ret = str.empty();
	if( ret == false ){             //文件根目录存在
		ret = (str == "CONF_FILE"); //配置文件的根节点命名为"CONF_FILE"
		if( ret == true ){          //检查文件版本号
			str = p_root->Attribute("VER");
			ret = str.empty();     //版本号为空，则说明文件格式不正确
			if( ret == false ){
				ret = (str == CONFIG_FILE_VERSION);
				if( ret == false ){
					throw ERROR_FILE_FORMAT;
				}
			}
			else{
				throw ERROR_FILE_FORMAT;
			}
		}
	}
	else{
		throw ERROR_FILE_ROOT_NAME;
	}

	return ret;
}

TiXmlElement* CConfFile ::find_var( const std::string& name )
{
	// 这个函数是能否支持分级目录的关键函数之一。
	// 为了能够让文件支持分支结构，这个函数必须能够进行树结构遍历功能
	// 在目前版本0的情况下，只支持平坦结构的变量定义

	TiXmlElement* ret = NULL;//p_root->FirstChildElement();
	CPathTravler trler( name , this , p_current );

	ret = trler();

	return ret;
}

CConfFile :: ~CConfFile(){}

void CConfFile :: GetValue( const std::string& var_name , std::string& value )
{
	TiXmlElement *obj = NULL;
	if( var_name.empty() ){
		throw ERR_GET_NAME;
	}
	obj = find_var( var_name );
	if( obj != NULL ){
		const char *p = obj->Attribute( "value" );
		if( p!= NULL ) value = p;
	}
	else{
		value.clear();
        throw ERR_PATH_NOT_EXIST;
	}
}

void CConfFile :: GetValue( const std::string& var_name , int& value )
{
	TiXmlElement *obj;
	if( var_name.empty() ){
		throw ERR_GET_NAME;
	}
	obj = find_var( var_name );
	std::string str_value;
	if( obj != NULL ){
		const char * c_str = obj->Attribute( "value" );
		if( c_str == nullptr ) throw ERR_GET_ATTR;
		str_value = c_str;     //获取值字符串
		if( str_value.empty() == false ){
		    std::string str = obj->Attribute( "type" );
			if( str.empty() == false ){
				if( str == "int" || str=="long"){//一切检查都正常，进行数值转化
					std::stringstream ss;
					ss << str_value;
					ss >> value;
				}
				else{//数据类型不正确
                    throw ERROR_VAR_VALUE_TYPE;
				}
			}
			else{//文件格式不正确
				throw  ERROR_FILE_FORMAT;
			}
		}
        else{//值字符串为空
			value = 0;
		}
    }else{
        throw ERR_PATH_NOT_EXIST;
    }
}

void CConfFile :: GetValue( const std::string& var_name , long& value )
{
	TiXmlElement *obj;
	if( var_name.empty() ){
		throw ERR_GET_NAME;
	}
	obj = find_var( var_name );
	std::string str_value;
	if( obj != NULL ){
		const char * c_str = obj->Attribute( "value" );
		if( c_str == nullptr )  throw ERR_GET_ATTR;
		str_value = c_str;     //获取值字符串
		if( str_value.empty() == false ){
			std::string str = obj->Attribute( "type" );
			if( str.empty() == false ){
				if( str == "int" || str=="long"){//一切检查都正常，进行数值转化
					std::stringstream ss;
					ss << str_value;
					ss >> value;
				}
				else{//数据类型不正确
					std::cerr << nowStr() << " Variable Type not matched for int" << std::endl;
                    throw ERROR_VAR_VALUE_TYPE;
				}
			}
			else{//文件格式不正确
				std::cerr << nowStr() << "File format error.Can not find type keyword" << std::endl;
				throw  ERROR_FILE_FORMAT;
			}
		}
		else{//值字符串为空
			value = 0;
		}
    }else{
        throw ERR_PATH_NOT_EXIST;
    }
}

void CConfFile :: GetValue( const std::string& var_name , bool& value )
{
	TiXmlElement *obj;
	if( var_name.empty() ){
		throw ERR_GET_NAME;
    }
	obj = find_var( var_name );
	std::string str_value;
	if( obj != NULL ){
		const char * c_str = obj->Attribute( "value" );
		if( c_str == nullptr )  throw ERR_GET_ATTR;
		str_value = c_str;     //获取值字符串
		if( str_value.empty() == false ){
			std::string str = obj->Attribute( "type" );
			if( str.empty() == false ){
				if( str == "bool" ){//一切检查都正常，进行数值转化
					if( str_value == "true" || str_value == "TRUE")
						value = true;
					else value = false;
				}
				else{//数据类型不正确
					ERROR_MSG( " Variable Type not matched for bool");
                    throw ERROR_VAR_VALUE_TYPE;
				}
			}
			else{//文件格式不正确
				ERROR_MSG( " File format error.Can not find type keyword" );
				throw  ERROR_FILE_FORMAT;
			}
		}
		else{//值字符串为空
			value = false;
		}
    }else{
        throw ERR_PATH_NOT_EXIST;
    }
}

void CConfFile :: GetValue( const std::string& var_name , float& value )
{
	TiXmlElement *obj;
	if( var_name.empty() ){
		std::cerr << nowStr() << " variable name can't be empty" << std::endl;
		throw ERR_GET_NAME;
	}
	obj = find_var( var_name );
	std::string str_value;
	if( obj != NULL ){
		const char * c_str = obj->Attribute( "value" );
		if( c_str == nullptr ) throw ERR_GET_ATTR;
		str_value = c_str;     //获取值字符串
		if( str_value.empty() == false ){
			std::string str = obj->Attribute( "type" );
			if( str.empty() == false ){
				  if( str == "float" ){//一切检查都正常，进行数值转化
					std::stringstream ss;
					ss << str_value;
					ss >> value;
				  }else{//数据类型不正确
					std::cerr << nowStr() << " Variable Type not matched for float" << std::endl;
                    throw ERROR_VAR_VALUE_TYPE;
				  }
			}else{//文件格式不正确
				std::cerr << nowStr() << " File format error.Can not find type keyword" << std::endl;
				throw  ERROR_FILE_FORMAT;
			}
		}
		else{//值字符串为空
			value = (float)0.0;
		}
    }else{
        throw ERR_PATH_NOT_EXIST;
    }
}

void CConfFile :: GetValue( const std::string& var_name , double& value )
{
	TiXmlElement *obj;
	if( var_name.empty() ){
		std::cerr << nowStr() << " variable name can't be empty" << std::endl;
		throw ERR_GET_NAME;
	}
	obj = find_var( var_name );
	std::string str_value;
	if( obj != NULL ){
		const char * c_str = obj->Attribute( "value" );
		if( c_str == nullptr ) throw ERR_GET_ATTR;
		str_value = c_str;     //获取值字符串
		if( str_value.empty() == false ){
			std::string str = obj->Attribute( "type" );
			if( str.empty() == false ){
				if( str == "float" ){//一切检查都正常，进行数值转化
					std::stringstream ss;
					ss << str_value;
					ss >> value;
				}
				else{//数据类型不正确
					std::cerr << nowStr() << " Variable Type not matched for double" << std::endl;
                    throw ERROR_VAR_VALUE_TYPE;
				}
			}
			else{//文件格式不正确
				std::cerr << nowStr() << " File format error.Can not find type keyword" << std::endl;
				throw  ERROR_FILE_FORMAT;
			}
		}
		else{//值字符串为空
			value = 0.0;
		}
    }else{
        throw ERR_PATH_NOT_EXIST;
    }
}

void CConfFile :: SetValue( const std::string& var , const std::string& v )
{
	if( var.empty() ){
		std::cerr << nowStr() << " variable name can't be empty" << std::endl;
		return;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){
		if( strcmp( obj->Attribute( "type" ) , "string" ) == 0 )
			obj->SetAttribute( "value" , v );
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}

void CConfFile :: SetValue( const std::string& var , int v )
{
	if( var.empty() ){
		std::cerr << nowStr() << " variable name can't be empty" << std::endl;
		return;
	}

	TiXmlElement * obj = find_var( var );

	if( obj != nullptr ){
		std::stringstream ss;
		ss << v;
		if( strcmp(obj->Attribute("type") , "int" ) == 0 )
			obj->SetAttribute( "value" , ss.str() );
		else{
			throw ERROR_VAR_VALUE_TYPE;
		}
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}


void CConfFile :: SetValue( const std::string& var , bool v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}

	TiXmlElement * obj = find_var( var );

	if( obj != nullptr ){
		if( strcmp( obj->Attribute( "type" ), "bool" ) == 0 ){
			if( v )
				obj->SetAttribute( "value" , "true" );
			else
				obj->SetAttribute( "value" , "false" );
		}else{
			throw ERROR_VAR_VALUE_TYPE;
		}
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}

void CConfFile :: SetValue( const std::string& var , long v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}

	TiXmlElement * obj = find_var( var );

	if( obj != nullptr ){
		if( strcmp( obj->Attribute("type") , "int" ) == 0 ){
			std::stringstream ss;
			ss << v;

			obj->SetAttribute( "value" , ss.str());
		}else{
			throw ERROR_VAR_VALUE_TYPE;
		}
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}


void CConfFile::SetValue( const std::string& var , float v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}

	TiXmlElement * obj = find_var( var );

	if( obj != nullptr ){
		if( strcmp( obj->Attribute("type") , "float" ) == 0 ){
			std::stringstream ss;
			ss << v;

			obj->SetAttribute( "value" , ss.str() );
		}else{
			throw ERROR_VAR_VALUE_TYPE;
		}
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}

void CConfFile::SetValue( const std::string& var , double v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}

	TiXmlElement * obj = find_var( var );

	if( obj != nullptr ){
		if( strcmp( obj->Attribute("type") , "float" ) == 0 ){
			std::stringstream ss;
			ss << v;

			obj->SetAttribute( "value" , ss.str() );
		}else{
			throw ERROR_VAR_VALUE_TYPE;
		}
	}else{
		throw ERROR_VAR_NOT_EXIST;
	}
}


void CConfFile :: AddValue( const std::string& var , const std::string& v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "string" );
			p.SetAttribute( "value" , v );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}
void CConfFile :: AddValue( const std::string& var , int v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "int" );
			std::stringstream ss;
			ss << v;
			p.SetAttribute( "value" , ss.str() );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}
void CConfFile :: AddValue( const std::string& var , long v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "int" );
			std::stringstream ss;
			ss << v;
			p.SetAttribute( "value" , ss.str() );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}
void CConfFile :: AddValue( const std::string& var , bool v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "string" );
			if( v )
				p.SetAttribute( "value" , "true" );
			else
				p.SetAttribute( "value" , "false" );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}
void CConfFile :: AddValue( const std::string& var , float v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "string" );
			std::stringstream ss;
			ss << v;
			p.SetAttribute( "value" , ss.str() );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}
void CConfFile :: AddValue( const std::string& var , double v )
{
	if( var.empty() ){
		throw ERROR_VAR_NO_NAME;
	}
	TiXmlElement * obj = find_var( var );
	if( obj != nullptr ){			// 指定的变量名称不能已经存在
		throw ERROR_VAR_NOT_EXIST;
	}
	std::string   dir , name;
	size_t pos = var.find_last_of( "/" );
	if( pos == std::string::npos ){
		throw ERR_PATH_GRRAMA;
	}
	dir = var.substr( 0 , pos );
	name = var.substr( pos + 1 );
	if( name.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	obj = find_var( dir );

	if( obj != nullptr ){			// 指定的变量路径不存在
		throw ERR_PATH_NOT_EXIST;
	}else{
		try{
			TiXmlElement p( "var" );// = new TiXmlElement( "var" );
			p.SetAttribute( "name" , name );
			p.SetAttribute( "type" , "string" );
			std::stringstream ss;
			ss << v;
			p.SetAttribute( "value" , ss.str() );
			obj->InsertEndChild( p );
		}catch( std::bad_alloc& e ){
			std::cerr << nowStr() << " Allocate memory fail" << std::endl;
			abort();
		}
	}
}

bool CConfFile :: is_var( TiXmlElement * node )
{
	std::string   value;
	bool ret = false;

	const char * d = node->Value();
	if( d ){
		std::string str = d;
		if( str == "var" ) ret = true;
	}else{
		throw ERR_OBJ_NULL;
	}

	return ret;
}
void CConfFile :: Del( const std::string& var , bool del_ch )
{
	if( var.empty() ){
		throw ERR_VAR_NAME_EMPTY;
	}

	TiXmlElement * obj = find_var( var ) , * parent = nullptr;
	parent = reinterpret_cast<TiXmlElement*>( obj->Parent() );
	if( obj ){
		try{
			if( is_var( obj ) ){// 是变量，则直接删除
				if( parent == nullptr ){
					std::cerr << nowStr() << " Can not find variable:"  << var << std::endl;
					throw ERR_DEL_ROOT;
				}else{
					if( !parent->RemoveChild( obj ) ){
						std::cerr << nowStr() << " Delete variable fail." << std::endl;
					}
				}
			}else{ // 是目录，则根据预设
				if( del_ch ){  // 删除所有的子节点
					TiXmlElement * child = obj->FirstChildElement() , * child1 = nullptr;
					std::string child_var;
					while( child ){
						child1 = child->NextSiblingElement();
						child_var = var + "/" + child->Attribute("name");
						Del( child_var , del_ch );		// 递归执行删除操作
						child = child1;
					}// else
				}else if( obj->NoChildren()== true && !parent->RemoveChild( obj ) ){ // 如果无子节点则直接删除
					std::cerr << nowStr() << " Remove Children nodes fail." << std::endl;
				}// if ( del_ch ) else
			}// if( is_var ) else
		}catch( err_code e ){
			std::cerr << nowStr() << " Can not find variable:" << var << std::endl;
			throw ERROR_VAR_NOT_EXIST;
		}
	}else{
		std::cerr << nowStr() << " Can not find variable:" << var << std::endl;
		throw ERROR_VAR_NOT_EXIST;
	}
}

std::string 
CConfFile :: errMsg( emErrCode e )
{
	std::string ret;
	switch( e ){
	case CAN_NOT_LOAD_FILE:
		ret = "Can not load configure file.";
		break;
	case ERR_FILE_NOT_EXIST:
		ret = "Configure file is not exist";
		break;
	case CAN_NOT_FIND_XML_ROOT:
		ret = "Can not find XML root node.";//无法找到XML根节点
		break;
	case ERROR_FILE_VERSION:                 //文件版本错误
		ret = "Bad Configure file version.";
		break;
	case ERROR_FILE_FORMAT:                  //文件格式错误
		ret = "Bad configure file format.";
		break;
	case ERROR_FILE_ROOT_NAME:
		ret = "Bad root name";//文件跟节点名称错误
		break;
	case ERROR_VAR_NO_NAME:                  //变量没有名称
		ret = "Missing variable name.";
		break;
	case ERROR_VAR_VALUE_TYPE:               //变量类型不匹配
		ret = "Variable type is not matched.";
		break;
	case ERROR_FILE_DIR_FORMAT:
		ret = "file path format error";
		break;
	case ERROR_VAR_NOT_EXIST:                // 变量不存在
		ret = "Variable is not exist.";
		break;
	case ERROR_SAVE_FILE:
		ret = "Save configure file failed.";
		break;
	case ERR_PATH_GRRAMA:                    // 路径格式错误
		ret = "Bad path gramma";
		break;
	case ERR_VAR_NAME_EMPTY:          // 变量名称空
		ret = "Variabled name should not be empty.";
		break;
	case ERR_PATH_NOT_EXIST:                 // 路径不存在
		ret = "Path is not exist;";
		break;
	case ERR_DEL_ROOT:                       // 执行了删除根节点的操作
		ret = "Can't delete root node";
		break;
	case ERR_OBJ_NULL:
		ret = "Object is null";
		break;
	default: ret = ""; break;
	}
	return ret;
}

CConfFile :: e_error_conf_file
CConfFile :: Save()
{
	if( m_doc.SaveFile() )
		return OK;
	return ERROR_SAVE_FILE;
}

CConfFile :: e_error_conf_file
CConfFile :: Save( const std::string& file )
{
	FILE * fp = fopen( file.c_str() , "w" );
	if( fp ){
		if( m_doc.SaveFile( fp ) ){
			fclose( fp );
			delete this;
			return OK;
		}else{
		}
	}else{
		std::cerr << nowStr() << " Can not open file " << file << " to write;" << std::endl;
	}
	return ERROR_SAVE_FILE;
}

