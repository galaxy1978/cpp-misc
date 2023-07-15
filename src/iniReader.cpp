#include <fstream>
#include <regex>
#include <vector>
#include <exception>

#include "misc.hpp"
#include "mallocSharedPtr.hpp"
#include "iniReader.hpp"
#include "ary_str.hpp"

using namespace wheels;

size_t iniRead :: split_line__( mallocSharedPtr<char>& buff , ArrayString& rst )
{
	size_t ret = 0 , len = buff.dataLen();
	// 这里扫描每一个字节进行处理
	std::string str;
	
	for( size_t i = 0; i < len; i ++ ){
		char c = buff[ i ];
		if( c == '\r' ){ continue; }  // 过滤掉回车符号，考虑到在windows下和linux下存在不同的换行符号进行处理
		// 但是这个代码没有处理在mac下的情况
		if( c == '\n' ){  // 过滤掉换行符号
			if( !str.empty() ){ // 过滤掉空行 ， 如果是正行则会将一行数据添加到字符串数组中，rst。
				rst.push_back( str );
				str = "";   // 清理行临时变量
				ret ++;     // 记录行数量
			}
			continue;
		}
 
		if( c == ' ' || c == '\t' ) continue;   // 过滤掉空白字符

		str.push_back( c );   // 记录字符
	}
	
	return ret;
}

iniRead :: emErrCode
iniRead :: process_root_item__( const std::string& str )
{
	// 这里处理自由变量
	std::string key , value;
	// 分隔键名，和值
	size_t pos = str.find( "=" );
	if( pos == std::string::npos ) return ERR_FIND_ASSIGN;
	
	key = str.substr( 0 , pos );
	value = str.substr( pos + 1 );
	// 添加记录
	auto rst = m_root_items__.insert( std::make_pair( key , value ) );
	if( rst.second == false ){
		return ERR_KEY_DUPLICATED;
	}
	return OK;
}

iniRead :: emErrCode
iniRead :: process_item__( const std::string& str )
{
	// 处理小节中的内容，方式基本相同。区别是需要检索当前的操作小节对象
	std::string key , value;
	size_t pos = str.find( "=" );
	if( pos == std::string::npos ) return ERR_FIND_ASSIGN;
	
	key = str.substr( 0 , pos );
	value = str.substr( pos + 1 );
	// 检索小节对象
	auto it = m_root_sections__.find( m_curr_section__ );
	if( it != m_root_sections__.end() ){
		// 将变量添加到所属小节中
		auto rst = it->second.insert( std::make_pair( key , value ) );
		if( rst.second == false ){
			return ERR_KEY_DUPLICATED;
		}
	}else{
		return ERR_SECTION_NOT_EXIST;
	}

	return OK;
}

bool iniRead :: is_comment__( const std::string& str )
{
	bool ret = false;
	// 第一个字符是 ; 的表示当前行是注释
	ret = ( str.front() == ';' );
	
	return ret;
}

bool iniRead :: is_section__( const std::string& str )
{
	bool ret = false;
	// 检索 [ ],判断为小节名称
	if( str.front() == '[' && str.back() == ']' ) ret = true;

	return ret;
}

bool iniRead :: is_item__( const std::string& str )
{
	auto it = str.find( "=" );
	// 检索 = ，并检查key存在，判定为变量
	if( it != std::string::npos && str.substr( 0, it ).length() > 0 ) return true;

	return false;
}

iniRead :: emErrCode
iniRead :: parse__( mallocSharedPtr<char>& buff )
{
	emErrCode ret = OK;
	ArrayString a_str;
	// 解析文件，首先将内容逐字节扫描，分开不同的行进行处理
	size_t count = split_line__( buff , a_str );
	// 然后针对每一行进行处理
	for( size_t i = 0; i < count; i ++ ){
		if( is_comment__( a_str[ i ] ) )continue;    // 跳过注释，不对注释进行处理和保存
		if( is_section__( a_str[ i ] ) ){            // 处理小节
			std::string str = a_str[ i ];
			m_curr_section__ = str.substr( 1 , str.length() - 2 );  // 过滤掉小节的 [ 和 ]
			auto it = m_root_sections__.find( m_curr_section__ );   // 检查是否已经存在小节
			if( it == m_root_sections__.end() ){                    // 如果不存在则添加，如果存在则跳过后面的
			    // 这个处理策略不会导致错误，但是会放弃部分内容的解析。后面可能会修改
				m_root_sections__.insert( std::make_pair( m_curr_section__ , item_t() ) );
			}
		}

		if( is_item__( a_str[ i ] ) ){  // 处理键值对
			if( m_curr_section__.empty() ){ // 在根项目下添加记录，处理自由变量的部分
				ret = process_root_item__( a_str[ i ] );			      
			}else{ // 在指定小节下添加记录，处理小节中的变量。
				ret = process_item__( a_str[ i ] );
			}

			if( ret != OK ) break;
		}
	}

	return ret;
}

iniRead :: iniRead( const std::string& file )
{
	assert( !file.empty() );
	if( access( file.c_str() , F_OK ) != 0 ) throw std::runtime_error( "文件不存在" );
	// getFileSize 在我视线的misc模块中
	size_t fsize = getFileSize( file ) , rsize = 0;
	// 在构造函数中根据文件的大小分配内存
	mallocSharedPtr< char >   buff( fsize + 10 );
	// 使用std::ifstream读取所有的内容然后进行处理
	std::ifstream ifs( file.c_str() , std::ios::binary | std::ios::in );
	if( ifs.is_open() == false ){
		throw std::runtime_error( "打开文件失败" );
	}

	bool rst = ifs.read( buff.get() , fsize ).bad();
	if( rst ){
		throw std::runtime_error( "读取文件失败" );
	}
	rsize = ifs.tellg();
	buff[ rsize ] = '\0';
	buff.dataLen( rsize );
	// 这里对文件进行解析
	auto rc = parse__( buff );
	if( rc != OK ){
		MSG_1( "解析INI文件错误 , %d" , TRED, rst );
		throw rst;
	}
}

iniRead :: ~iniRead(){}

bool iniRead :: save( const std::string& file )
{
	bool ret = false;
	// 这里是保存实现
	if( access( file.c_str() , F_OK ) == 0 ) {
		return false;
	}
	// 序列化使用 stringstream 
	std::stringstream ss;

	// 处理根项目，针对跟项目生成键值对的字符串。进行简单的拼接。
	// 无论在windows下还是linux下都使用了换行符号作为行的分隔符号
	for( auto it = m_root_items__.begin(); it != m_root_items__.end(); it ++ ){
		ss << it->first << " = " << it->second << "\n";
	}

	// 处理所有的小节
	for( auto sit = m_root_sections__.begin(); sit != m_root_sections__.end(); sit ++ ){
		ss << "\n[ " << sit->first << " ]\n";    // 添加小节行
		// 针对小节中的所有变量进行处理
		for( auto it = sit->second.begin(); it != sit->second.end(); it ++ ){
			ss << it->first << " = " << it->second << "\n";
		}
	}
	// 下面保存文件
	std::ofstream ofs( file.c_str() , std::ios::out | std::ios::binary );

	if( ofs.is_open() ){
		std::string str = ss.str();
		ofs.write( str.c_str() , str.length() );
		// 检查是否保存成功
		ret = !ofs.bad();
	}
	
	return ret;
}
