#include <vector>
#include <sstream>
#include <regex>
#include <iostream>

#include "str_template.h"
//#include "error.hpp"
#include "cltrpbase.h"

/**
2011-3-13  增加连接检查功能,如果数据库连接失败则自动重新连接数据库
2011-3-14  调整数据结构将数据转化等操作封装在数据库接口中.从通用的数据库接口继承而来,保证了程序中使用不同数据库接口的一致性
2019-6-23  将原来的wxString接口全部换成std::string接口，同时增加了BLOB数据支持
*/

CMySQLTuple :: CMySQLTuple( MYSQL_ROW rows , int count , int blobIndex[] , int lenArray[] , int blobCount )
{
	if( rows == nullptr ) throw CltrpBase::ERR_NULL_ROWS;
	if( blobCount > 0 ){
		m_has_blob = true;

                for( int i = 0; i < blobCount; i ++ ){ // 转存blob类型
                        m_blob_data.insert(
                                std::make_pair(
                                        blobIndex[ i ] ,
                                        BufferByte( ( BufferByte::pointer)rows[ blobIndex[ i ] ] , lenArray[ i ])
                                )
                        );
                }
        }

	for( int i = 0; i < count; i ++ ){// 转存其他类型
		bool is_in_blob = false;
		for( int j = 0; j < blobCount; j ++ ){
                        if( i == blobIndex[ j ] ) is_in_blob = true;
		}
		if( is_in_blob )continue;

		row_data.push_back( std::string( rows[ i ]) );
	}
}

CMySQLTuple :: CMySQLTuple(MYSQL_ROW row, int num_cols )
: m_has_blob( false )
{
	if( row == nullptr ) throw CltrpBase::ERR_NULL_ROWS;
	for( int i = 0; i < num_cols && row != NULL; i ++){
		if( nullptr != row[ i ]){
			row_data.push_back( std::string( row[ i ]));
		}else{
			row_data.push_back( std::string(""));
		}
	}

}

CMySQLTuple :: CMySQLTuple( const CMySQLTuple& b )
{
	row_data = b.row_data;
	m_has_blob = b.m_has_blob;
	m_blob_data = b.m_blob_data;
}

CMySQLTuple :: CMySQLTuple( CMySQLTuple&& b )
{
	row_data = std::move( b.row_data );
	m_has_blob = b.m_has_blob;
        m_blob_data = std::move( b.m_blob_data );
}

CMySQLTuple ::~CMySQLTuple()
{

}

CMySQLTuple& CMySQLTuple :: operator=( const CMySQLTuple& b )
{
	row_data = b.row_data;
	m_has_blob = b.m_has_blob;
	m_blob_data = b.m_blob_data;

	return *this;
}

CMySQLTuple& CMySQLTuple :: operator=( CMySQLTuple&& b )
{
	row_data = std::move( b.row_data );
	m_has_blob = b.m_has_blob;
        m_blob_data = std::move( b.m_blob_data );

        return *this;
}

void CMySQLTuple ::SetRow(MYSQL_ROW row, int cols)
{
	for( int i = 0; i < cols && row != NULL; i ++)
		row_data.push_back( row[ i ] ) ;
}

void CMySQLTuple :: SetRow( MYSQL_ROW row , int count , int blobIndex[] , int lenArray[] ,int blobCount )
{
	if( row == nullptr ) return;
	if( blobCount > 0 )
		m_has_blob = true;

	for( int i = 0; i < blobCount; i ++ ){ // 转存blob类型
		m_blob_data.insert(
			std::make_pair(
				blobIndex[ i ] ,
				BufferByte( (BufferByte::pointer)row[ blobIndex[ i ] ] , lenArray[ i ])
			)
		);
	}
	for( int i = 0; i < count; i ++ ){// 转存其他类型
		bool is_in_blob = false;
		for( int j = 0; j < blobCount; j ++ ){
                        if( i == blobIndex[ j ] ) is_in_blob = true;
		}
		if( is_in_blob )continue;

		row_data.push_back( std::string( row[ i ]));
	}
}

bool CMySQLTuple ::GetValue( int col, bool& value)
{
	bool   ret = true;
	try{
		value = ( row_data.at( col ) == std::string("1") ? true : false );
	}catch( std::out_of_range & e ){
		ret = false;
		std::cerr << e.what() << std::endl;
	}
	return ret;
}

bool CMySQLTuple ::GetValue( int col, uint8_t& value )
{
	bool        ret = false;
	long        tmp_ul = 0;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}

	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> tmp_ul;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> tmp_ul;
		ret = true;
	}
	if( ret ) value = (uint8_t)(tmp_ul & 0xFF);
    	return ret;
}

bool CMySQLTuple ::GetValue( int col, uint16_t& value )
{
	bool        ret = false;
	long        tmp_ul = 0;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}


	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> tmp_ul;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> tmp_ul;
		ret = true;
	}

	value = (uint16_t)(tmp_ul & 0xFFFF);

	return ret;
}

bool CMySQLTuple ::GetValue( int col, int& value )
{
	bool        ret = false;
	long        tmp_ul = 0;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}

	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> tmp_ul;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> tmp_ul;
		ret = true;
	}

        value = (int)(tmp_ul &0xFFFFFFFF);

	return ret;
}
bool CMySQLTuple ::GetValue( int col, unsigned int& value)
{
	bool        ret = false;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}


	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> value;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> value;
		ret = true;
	}

	return ret;
}

bool CMySQLTuple ::GetValue( int col, long& value )
{
	bool        ret = false;

	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}


	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> value;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> value;
		ret = true;
	}

	return ret;
}

bool CMySQLTuple ::GetValue( int col, long long& value )
{
    bool        ret = false;

    std::string str;
    try{
        str = row_data.at( col );
    }catch( std::out_of_range& e ){
        std::cerr << e.what() << std::endl;
        return ret;
    }


    if( CStrTmpt::isDigit( str ) ){
        std::stringstream ss;
        ss << str;
        ss >> value;
        ret = true;
    }else if( CStrTmpt::isHex( str )){
        std::stringstream ss;
        ss << str;
        ss >> std::hex >> value;
        ret = true;
    }

    return ret;
}

bool CMySQLTuple ::GetValue( int col, uint64_t& value )
{
    bool        ret = false;

    std::string str;
    try{
        str = row_data.at( col );
    }catch( std::out_of_range& e ){
        std::cerr << e.what() << std::endl;
        return ret;
    }


    if( CStrTmpt::isDigit( str ) ){
        std::stringstream ss;
        ss << str;
        ss >> value;
        ret = true;
    }else if( CStrTmpt::isHex( str )){
        std::stringstream ss;
        ss << str;
        ss >> std::hex >> value;
        ret = true;
    }

    return ret;
}

bool CMySQLTuple :: GetValue( int col, unsigned long& value )
{

	bool        ret = false;

	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}

	if( CStrTmpt::isDigit( str ) ){
		std::stringstream ss;
		ss << str;
		ss >> value;
		ret = true;
	}else if( CStrTmpt::isHex( str )){
		std::stringstream ss;
		ss << str;
		ss >> std::hex >> value;
		ret = true;
	}

	return ret;
}
bool CMySQLTuple ::GetValue( int col, float& value)
{
	bool        ret = false;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}

	if( CStrTmpt::isReal( str ) == true){
		std::stringstream ss;
		ss << str;
		ss >> value;
	}

	return ret;
}

bool CMySQLTuple ::GetValue( int col, double& value )
{
	bool        ret = false;
	std::string str;
	try{
		str = row_data.at( col );
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return ret;
	}

	if( CStrTmpt::isReal( str ) == true){
		std::stringstream ss;
		ss << str;
		ss >> value;
	}

	return ret;
}
bool CMySQLTuple ::GetValue( int col, std::string& value )
{
	try{
		value = row_data[ col ];
	}catch( std::out_of_range& e ){
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}

bool CMySQLTuple :: GetValue( int col, void *buffer , size_t s )
{
	bool   ret = true;
	if( buffer == nullptr ) return ret;
	try{
		if( s < m_blob_data.at( col ).size() ){
			memcpy( buffer , m_blob_data.at( col ).data() , s );
		}else{
			memcpy( buffer , m_blob_data.at( col ).data() , m_blob_data.at( col ).size() );
		}
		ret = true;
	}catch( std::out_of_range &e ){
		std::cerr << e.what() << std::endl;
		return false;
	}catch( ... ){
		ret = false;
	}
	return ret;
}

CMySQLField :: CMySQLField(MYSQL_FIELD& p_result):
	field_width( 0 ),
	max_length( 0 ),
	flags(0),
	num_decimals(0 ),
	charset_num(0),
	type(MYSQL_TYPE_DECIMAL),
	m_has_blob( false )
{
	field_name 	= p_result.name;
	field_org_name 	=  p_result.org_name ;
	table_name 	= p_result.table ;
	table_org_name 	= p_result.org_table;
	db_name 	= p_result.db ;
	db_org_name 	= db_name;

	field_width 	= p_result.length;
	max_length 	= p_result.max_length;
	flags 		= p_result.flags;
	num_decimals 	= p_result.decimals;
	charset_num 	= p_result.charsetnr;
	type 		= p_result.type;

	m_has_blob = false;
}

CMySQLField& CMySQLField :: operator=( const CMySQLField& b )
{
	field_name 	= b.field_name;
	field_org_name 	= b.field_org_name ;
	table_name 	= b.table_name ;
	table_org_name 	= b.table_org_name;
	db_name 	= b.db_name ;
	db_org_name 	= db_name;

	field_width 	= b.field_width;
	max_length 	= b.max_length;
	flags 		= b.flags;
	num_decimals 	= b.flags;
	charset_num 	= b.charset_num;
	type 		= b.type;

	m_has_blob 	= b.m_has_blob;
	m_blob_data 	= b.m_blob_data;
	return *this;
}

CMySQLField :: CMySQLField( const CMySQLField& b ) :
	field_width(0),
	max_length(0),
	flags(0),
	num_decimals(0),
	charset_num(0),
	type(MYSQL_TYPE_DECIMAL),
	m_has_blob(false)
{
	field_name 	= b.field_name;
	field_org_name 	= b.field_org_name ;
	table_name 	= b.table_name ;
	table_org_name 	= b.table_org_name;
	db_name 	= b.db_name ;
	db_org_name 	= db_name;

	field_width 	= b.field_width;
	max_length 	= b.max_length;
	flags 		= b.flags;
	num_decimals 	= b.flags;
	charset_num 	= b.charset_num;
	type 		= b.type;

	m_has_blob 	= b.m_has_blob;
	m_blob_data 	= b.m_blob_data;
}
CMySQLField :: CMySQLField( CMySQLField&& b ) :
	field_width(0),
	max_length(0),
	flags(0),
	num_decimals(0),
	charset_num(0),
	type(MYSQL_TYPE_DECIMAL),
	m_has_blob(false)
{
	field_name 	= std::move( b.field_name  );
	field_org_name 	= std::move( b.field_org_name );
	table_name 	= std::move( b.table_name );
	table_org_name 	= std::move( b.table_org_name);
	db_name 	= std::move( b.db_name );
	db_org_name 	= db_name;

	field_width 	= b.field_width;
	max_length 	= b.max_length;
	flags 		= b.flags ;
	num_decimals 	= b.num_decimals;
	charset_num 	= b.charset_num;
	type 		= b.type;

	m_has_blob 	= b.m_has_blob;
	m_blob_data 	= std::move( b.m_blob_data );
}

///////////////////////////////////////////////////////////////////////////////////////////
CltrpBase :: CltrpBase(	const std::string& server, const std::string& user, const std::string& pass, const std::string& db, int _port ):
			p_mysql_connect( nullptr ),
			p_result( nullptr ),
			p_fd( nullptr ),
			m_row( nullptr ),
			m_conn_status( false )
{
	p_mysql_connect = mysql_init(NULL);
	if(!p_mysql_connect){
		err_code =DB_LIB_INIT_ERR;
		err_msg = "MySQL database initialize fail.";
		m_conn_status = false;
	}else { //存储服务器连接信息
		server_add = server;
		port = _port;
		user_name = user;
		db_name = db;
		password = pass;
		int reconnect = 1;
		mysql_options( p_mysql_connect , MYSQL_OPT_RECONNECT , &reconnect);
		///真正连接服务器,如果连接成功,做出成功标记
		do_connect();
	}
	if( m_conn_status ){
		init_connect_info();  ///初始化连接信息
	}
	p_result = NULL;
	p_fd = NULL;
}

CltrpBase::~CltrpBase()
{
	if(p_result){
		mysql_free_result(p_result);
		p_result = NULL;
	}

	mysql_close(p_mysql_connect);
}

int CltrpBase::MySQL_CreatUser( const std::string& username)
{
	std::stringstream  sql;
	int          ret;

	sql << "CREATE USER " << username << ";";

	ret = mysql_query(p_mysql_connect, sql.str().c_str() );

	if( ret != 0 ){
		err_code = 4;
		err_msg = mysql_error(p_mysql_connect);
	}
	return ret;
}


int CltrpBase::Del_User( std::string& user)
{
	std::stringstream sql;
	int         ret;

	sql << "DROP USER " << user << ";";

	ret = mysql_query(p_mysql_connect, sql.str().c_str() );
	if( ret != 0 ){
		err_code = 4;
		err_msg = mysql_error(p_mysql_connect);
	}
	return ret;
}


bool CltrpBase::Is_ResultOK()
{
	bool tmp = false;

	if(p_result != NULL) {
		if( mysql_num_rows(p_result) > 0) tmp=true;
		else tmp=false;
	}
	return tmp;
}


void CltrpBase::FreeResult()
{
	mysql_free_result(p_result);
	p_result = NULL;
}


MYSQL_ROW CltrpBase:: MySQL_Fetch_Row()
{
	MYSQL_ROW       ret = nullptr;
	if(p_result)
		ret = mysql_fetch_row(p_result);

	return ret;
}


unsigned long* CltrpBase::MySQL_fetch_length()
{
	unsigned long *tmp_p = NULL;
	if(p_result != NULL)
		tmp_p = mysql_fetch_lengths(p_result);
	return tmp_p;
}

unsigned long CltrpBase::MySQL_num_rows()
{
	unsigned int tmp_ret = 0;
	if(p_result)
		tmp_ret = mysql_num_rows(p_result);
	return tmp_ret;
}

// 获取某一列的列信息
MYSQL_FIELD* CltrpBase::MySQL_fetch_field()
{
	MYSQL_FIELD   *ret = nullptr;

	if(p_result){
		p_fd = mysql_fetch_field(p_result);
		if(p_fd == NULL){
			mysql_field_seek(p_result,0);
			p_fd = mysql_fetch_field(p_result);
		}
		ret = p_fd;
	}

	return ret;
}


MYSQL_FIELD* CltrpBase::MySQL_fetch_fields()
{

	if(p_result)
		p_fd = mysql_fetch_fields(p_result);
	else
		p_fd = NULL;

	return p_fd;
}


std::string
CltrpBase::MySQL_get_field_name()
{
	std::string ret ;
	if( p_fd )
		ret = p_fd->name;
	else
		throw ERR_NULL_FIELD_RESULT;

        return ret;
}

unsigned int CltrpBase::MySQL_get_col_num()
{
	unsigned int tmp;
	if(m_conn_status)
		tmp = mysql_field_count(p_mysql_connect);
	else
		tmp=0;
	return tmp;
}

int CltrpBase::MySQL_select_db(std::string& str1)
{
	int ret = 0;
	if(str1.empty())
		ret = -1;
	else if(p_mysql_connect != NULL)
		ret = mysql_select_db(p_mysql_connect,str1.c_str() );

	if( ret ){
		 err_code = DB_QUERY_ERR;
		 err_msg = mysql_error(p_mysql_connect);
	}

	return ret;
}

int CltrpBase::MySQL_use_table(std::string& str)
{
	int ret = 0;
	std::string tmp_str;

	if(p_mysql_connect) {
		tmp_str = "USE " ;
		tmp_str += str + ";";

		ret = mysql_query(p_mysql_connect , tmp_str.c_str() );
	}else{
		ret = -1;
	}

	return ret;
}

void CltrpBase:: MySQL_error_message(std::string& error)
{
    error = err_msg;
}

int CltrpBase:: MySQL_set_char_code(const std::string& str)
{
	std::string tmp("SET NAMES ");
	int         err;

	tmp += str + ";";

	if(p_mysql_connect != NULL)
		err = mysql_query(p_mysql_connect,tmp.c_str());
	else{
		err = -1;
	}
	if(err !=0 ){
		err_code = DB_QUERY_ERR;
		err_msg = mysql_error(p_mysql_connect) ;
	}

	return err;
}

void CltrpBase::MySQL_SetPassword(const std::string& user, const std::string& p)
{
	std::string para("SET PASSWORD FOR ");

	if( !user.empty()){
		para += user + " = PASSWORD(\'" + p + "\');";

		int err_code;
		err_code = mysql_query(p_mysql_connect,para.c_str());
		if(err_code){
			err_code = DB_QUERY_ERR;
			err_msg = mysql_error(p_mysql_connect);
			throw err_code;
		}
	}else{
		throw ERR_EMPTY_USER;
	}
}



CTRP_CRM_DB :: errCode
CltrpBase :: MySQL_Query( const std::string& sql_com )
{
	errCode err = CTRP_CRM_DB::OK;
	int num_cols = 0, __num_rows__ = 0;
	if(p_mysql_connect != NULL && !sql_com.empty() ){
		int err1 = mysql_query(p_mysql_connect, sql_com.c_str());
		if(err1 == 0){ /// 存储查询结果；查询结果分成两种类型，其一是由返回值得查询结果；其二是查询
                             /// 结果描述。通过检查mysql_store_result函数的返回结果来分别是否是返回值的查询
			if( p_result ){ /// 如果数据库接口查询结果中已经保存的数据，将其释放掉
				mysql_free_result(p_result);
				p_result = NULL;
			}
			p_result = mysql_store_result(p_mysql_connect);///如果是查询则保存查询结果，否则则保存查询的信息。
			if( p_result != NULL ){
				__num_rows__ = mysql_num_rows( p_result );
				num_cols = mysql_num_fields( p_result );
				m_field_info.erase( m_field_info.begin() , m_field_info.end() );
				data_rows.erase( data_rows.begin() , data_rows.end());
			}else{
				const char * info_str = mysql_info( p_mysql_connect );   ///保存查询的返回信息
				if( info_str != nullptr ){
					information = info_str;
				}
				num_cols = 0;
			}

			///存储列信息
			for( int i = 0; i < num_cols ; i ++ ){
				p_fd = mysql_fetch_field_direct(p_result, i );
				m_field_info.push_back( CMySQLField( *p_fd ) );
			}
			///将数据库结果转化出来，存储在本地计算计串结构中
			for( int j = 0; j < __num_rows__ && p_result != NULL; j ++){
				if( 0 ){//如果数据是二进制数据，则保存为二进制数据
				}else{
					m_row = mysql_fetch_row( p_result );
					data_rows.push_back( CMySQLTuple( m_row, num_cols));
				}
			}

			if( p_result != NULL )
				mysql_data_seek(p_result, 0 ); ///取用完数据后,将row指针返回初始位置,为旧版的数据库应用提供合理的数据位置
		}else{
			err_code = DB_QUERY_ERR;
			err_msg = mysql_error( p_mysql_connect);
			return (CTRP_CRM_DB::errCode)DB_QUERY_ERR;
		}
	}else{
		return ERR_MYSQL_NOT_CONNECTED;
	}

	return err;
}

my_ulonglong CltrpBase :: MySQLLastInserId()
{
	return mysql_insert_id(p_mysql_connect);
}

int CltrpBase :: CDB_Disconnect()
{
	if(p_result){
		mysql_free_result(p_result);
		p_result = NULL;
	}
	mysql_close(p_mysql_connect);

	return 0;
}
int CltrpBase :: CDB_NumCols()
{
	int     ret;
	if( p_result ){
		ret = mysql_num_fields( p_result );
	}else{
		ret = 0;
	}
	return ret;
}

 int CltrpBase :: CDB_NumRows()
{
	unsigned int ret;
	if( p_result!= NULL )
		ret = mysql_num_rows(p_result );
	else
		ret = 0;

	return ret;
}

int CltrpBase :: CDB_ChangePass(const std::string& /*old*/, const std::string& _new )
{
	int         ret;
	std::string sql;

	sql = "SET PASSWORD = PASSWORD('" + _new + "');";
	ret = mysql_query( p_mysql_connect, sql.c_str());
	if( ret ){
		ret = DB_QUERY_ERR;
		err_msg =  mysql_error( p_mysql_connect);
	}

	return ret;
}

int CltrpBase :: CDB_EnumColName(const std::string& table, ArrayString& col_names )
{
	if( table.empty() ) return ERR_TABLE_NAME_EMPTY;
	int        ret, num_row;
	std::string   sql;

	sql = "SHOW COLUMNS FROM " + table + ";";
	ret = mysql_query(p_mysql_connect, sql.c_str() );
	if( ret == 0 ){
		num_row = mysql_num_rows(p_result);
		for( int i = 0;i < num_row; i ++ ){
			m_row = mysql_fetch_row(p_result);
			col_names.push_back( m_row[ 0 ] );
		}
	}

	return ret;
}

int  CltrpBase :: CDB_GetRow(int pos, ArrayString& row)
{
	int   ret =0;

	if( pos > (int)(data_rows.size() )){
		ret = DB_RES_OUTOFNUM;
		err_code = ret;
	}else{
		data_rows[ pos ].GetRow(row);
		ret = 0;
	}
    return ret;
}

//服务器管理类函数
bool CltrpBase :: CDB_GetServerInfo( std::string& info)
{
	bool ret = false;
	if( p_mysql_connect ) {
		info = mysql_get_server_info(p_mysql_connect );
		ret = true;
	}

	return ret;
}
bool CltrpBase :: CDB_GetClientInfo( std::string& info)
{
	bool ret = false;

	if( p_mysql_connect ) {
		info = mysql_get_client_info();
		ret = true;
	}

	return ret;
}

bool CltrpBase :: CDB_ProcessList()
{
	bool  ret;
	std::string sql;

	sql = "SHOW FULL PROCESSLIST;";
	ret = ( MySQL_Query(sql) == 0);

	return ret;
}

bool CltrpBase :: CDB_Kill(unsigned long pid)
{
	bool            ret = false;
	ret = (mysql_kill(p_mysql_connect, pid) == 0 ? true : false );

	return ret;
}

unsigned long CltrpBase :: CDB_NumQuerys()
{
	// TODO (Administrator#1#): finish these later
	unsigned long    ret = 0;

	return ret;
}

unsigned long CltrpBase :: CDB_NetLoad()
{
	// TODO (Administrator#1#): finish these later
	unsigned long ret = 0;
	return ret;
}

bool CltrpBase :: CDB_GetValue(int row, int col, int& value)
{
    bool                    ret = false;

    ret = data_rows[ row ].GetValue( col, value );
    return ret;
}

bool CltrpBase :: CDB_GetValue(int row, int col, long& value)
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue(int row, int col, bool& value)
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue(int row, int col, double& value)
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue( int row, int col, std::string& value)
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue( int row , int col, long long& value )
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue( int row , int col, uint64_t& value )
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_GetValue(int row, int col, void *buffer)
{
    data_rows[ row ].GetValue( col, buffer, -1 );
    return true;
}
bool CltrpBase :: CDB_GetValue(int row, int col, unsigned long& value )
{
    return data_rows[ row ].GetValue( col, value);
}
bool CltrpBase :: CDB_GetValue(int row, int col, unsigned int& value )
{
    return data_rows[ row ].GetValue( col, value);
}
bool CltrpBase :: CDB_GetValue(int row, int col, uint8_t& value )
{
    return data_rows[ row ].GetValue( col, value);
}
bool CltrpBase :: CDB_GetValue(int row, int col, uint16_t& value )
{
    return data_rows[ row ].GetValue( col, value);
}

bool CltrpBase :: CDB_Ping()
{
    bool ret = false;
    if( p_mysql_connect == nullptr ) return ret;

    int rc = mysql_ping( p_mysql_connect );

    if( rc != 0 ){
        err_msg = mysql_error( p_mysql_connect );
        m_conn_status = false;
    }else{
        ret = true;
        m_conn_status = true;
    }

    return ret;
}
/*
连接数据库
*/
void CltrpBase :: do_connect()
{
    if(mysql_real_connect( p_mysql_connect, server_add.c_str(), user_name.c_str() , password.c_str() , db_name.c_str() , port,
			0,CLIENT_MULTI_STATEMENTS)==NULL) {
	    err_code = DB_CONN_ERR;
	    err_msg = mysql_error(p_mysql_connect) ;
	    m_conn_status = false;
    }else{
	    m_conn_status = true;
	    err_code = 0;
	    err_msg.clear();
    }
}

void CltrpBase :: init_connect_info()
{
	server_info =  mysql_get_server_info(p_mysql_connect) ;
	client_info =  mysql_get_client_info() ;
	host_info = mysql_get_host_info(p_mysql_connect) ;
}

int CltrpBase :: CDB_EnumDBS( ArrayString& dbs )
{
	int ret = 0;
	std::string  sql = "SHOW DATABASES;";
	std::string  str;

	ret = mysql_query(p_mysql_connect, sql.c_str() );
	if( ret == 0 ){
		if( p_result != NULL ){
			mysql_free_result( p_result );
			p_result = NULL;
		}
		p_result = mysql_store_result( p_mysql_connect );
		if( p_result != NULL ){
			while( (m_row = mysql_fetch_row( p_result )) != NULL ){
				str =  m_row[ 0 ];
				dbs.push_back( str );
			}
		}else{ // SQL 语句不是有返回结果的类型
			err_code = ret;
			err_msg.clear();
		}
	}else{
		err_code = ret;
		err_msg = mysql_error( p_mysql_connect);
	}

	return ret;
}

int CltrpBase :: CDB_EnumTables(ArrayString& tables)
{
	int         ret = 0;
	std::string    str,sql = "SHOW TABLES;";

	ret = mysql_query( p_mysql_connect , sql.c_str() );
	if( ret == 0 ){
		if( p_result != NULL ){
			mysql_free_result( p_result );
			p_result = NULL;
		}
		p_result = mysql_store_result(  p_mysql_connect );
		while( (p_result != NULL ) && ( m_row = mysql_fetch_row( p_result ) ) != NULL ){
			str = m_row[ 0 ];
			tables.push_back( str );
		}
	}else{
		err_code = ret;
		err_msg = mysql_error( p_mysql_connect );
	}

	return ret;
}

int CltrpBase :: CDB_EnumTables( const std::string& db, ArrayString& tables )
{
	int         ret = 0;
	std::string    str, sql = "SHOW TABLES " + db + ";";

	ret = mysql_query( p_mysql_connect , sql.c_str() );
	if( ret == 0 ){
		if( p_result != NULL ){
			mysql_free_result( p_result );
			p_result = NULL;
		}
		p_result = mysql_store_result(  p_mysql_connect );
		while( (p_result != NULL ) && ( m_row = mysql_fetch_row( p_result ) ) != NULL ){
			str = m_row[ 0 ];
			tables.push_back( str );
		}
	}else{
		err_code = ret;
		err_msg = mysql_error( p_mysql_connect );
	}

	return ret;
}

void CltrpBase :: CDB_AutoCommit( bool flag )
{
	mysql_autocommit( p_mysql_connect, flag );
}

bool CltrpBase :: CDB_Commit()
{
	return mysql_commit( p_mysql_connect );
}
