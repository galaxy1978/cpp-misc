/**
 * @brief MySQL C++接口封装
 * @version 1.1
 * @date 2009-8-9 ~ 2021-2-25
 * @author 宋炜
 */
/**
2011-3-12， 为了增强数据库访问和对数据类型的控制，在原有代码的基础扩展该类的结构，使能够满足任何
            结构元组的访问并能够提供需要数据类型。虽然，为了保证模块在原有代码上的运行情况原有代码不做
            删减;此外为了保证对不同数据库的支持，数据库需要从一个公共的纯虚类继承。保证不同数据库在执行
            过程中的多态性。
2011-3-14,  在代码中增加了数据元组的支持和数据域信息的支持,是数据库接口更符合数据库的结构.并且能够将数据类型的
                   转化封装在数据库接口中,减少了外部程序编写的复杂程度和编码的劳动强度

2021-2-25 ADDED 宋炜 增加MYSQL_OPT_RECONNECT，连接重连机制。当连接长时间没有使用的时候使用mysql_ping来保持
	    连接。
*/
#ifndef   __CLTRPBASE_H__
#define   __CLTRPBASE_H__

#include <mysql/mysql.h>

#include <string>
#include <vector>
#include <map>

#include "trp_crm_db_base.h"
#include "ary_str.hpp"
#include "buffer.hpp"

/**
 * 数据元组类定义
*/
class CMySQLTuple : public CDBTuple
{
private:
	std::vector< std::string >	row_data;	// 对于非BLOB类型的数据可以使用字符串数组的方式
                                                        // 保存,但是对  于blob类型的数据则不能够使用这个方
                                                        // 式保存
        bool m_has_blob;		                // 是否存在blob类型的数据
	std::map< int , BufferByte >  m_blob_data;	// BLOB类型的数据
public:
	CMySQLTuple():m_has_blob( false ) {}
        CMySQLTuple( MYSQL_ROW rows , int count , int blobIndex[] , int lenArray[] , int blobCount );
	CMySQLTuple(MYSQL_ROW row, int num_rows );

	CMySQLTuple( const CMySQLTuple& b );
	CMySQLTuple( CMySQLTuple&& b );

	virtual ~CMySQLTuple();

	CMySQLTuple& operator=( const CMySQLTuple& b );
	CMySQLTuple& operator=( CMySQLTuple&& b );
	/**
	 * @brief 指定结果集合
	 * @param row
	 * @param cols
	 * @param blobIndex
	 * @param blobCount
	 * @exception CltrpBase::ERR_NULL_ROWS
	*/
	void SetRow(MYSQL_ROW row, int cols );
	void SetRow( MYSQL_ROW row , int cols , int blobIndex[] , int blobCount );
	void SetRow( MYSQL_ROW row , int cols , int blobIndex[] , int lenArray[] ,int blobCount );
	void GetRow( ArrayString& row)
	{
		for( auto d : row_data ){
			row.push_back( d );
		}
	}
	bool GetValue( int col, bool& value) override;
	bool GetValue( int col, uint8_t& value ) override;
	bool GetValue( int col, uint16_t& value ) override ;
	bool GetValue( int col, int& value ) override;
	bool GetValue( int col, unsigned int& value) override;
	bool GetValue( int col, long& value ) override;
	bool GetValue( int col, unsigned long& value ) override;
	bool GetValue( int col, float& value) override;
	bool GetValue( int col, double& value ) override;
	bool GetValue( int col, std::string& value ) override;
    bool GetValue( int col, long long& value );
    bool GetValue( int col, uint64_t& value );

	/**
	* @brief 读取BLOB类型的数据
	* @param col ， 列号
	* @param buffer ， 数据保存区
	* @param len ， 要读取的数据的字节长度
	*/
	bool GetValue( int col, void *buffer , size_t len ) override;
};

class CMySQLField : public CDBField
{
public:

private:
	std::string         field_name;
	std::string         field_org_name;
	std::string         table_name;
	std::string         table_org_name;
	std::string         db_name;
	std::string         db_org_name;
	unsigned long       field_width;
	unsigned long       max_length;
	unsigned long       flags;
	unsigned long       num_decimals;
	unsigned long       charset_num;
	enum_field_types    type;
	bool                m_has_blob;
    std::map< int , Buffer< char > >   m_blob_data;
public:
	CMySQLField( MYSQL_FIELD& p_result);
	CMySQLField( const CMySQLField& b );
	CMySQLField( CMySQLField&& b );
	CMySQLField() :
		field_width(0),
		max_length(0),
		flags(0),
		num_decimals(0),
		charset_num(0),
		type(MYSQL_TYPE_DECIMAL),
		m_has_blob(false) {}
	virtual ~CMySQLField(){}

	CMySQLField& operator=( const CMySQLField& b );
	///由于MySQL接口取到的数据类型是c结构字符串类型,所有在所有的设置文本类型的数据时都使用了char*类型
	void SetFieldName( char *name )override{ field_name = name; }
	void SetFieldOrgName( char *name )override{ field_name = name; }
	void SetTableName( char* name )override{ table_name = name; }
	void SetTableOrgName( char* name )override{ table_org_name = name; }
	void SetDbName(char * name )override{ db_name = name; }
	void SetDbOrgName( char * name )override{ db_org_name = name; }
	void SetFieldType( unsigned long __type__ )override{ type = (enum_field_types)__type__; }
	void SetFlags( unsigned long type )override{ flags = type; }

	virtual std::string& GetFieldName() override{ return field_name; }
	std::string& GetFieldOrgName()override{ return field_org_name; }
	virtual std::string& GetDbName()override{ return db_name; }
	virtual std::string& GetDbOrgName()override{ return db_org_name; }
	virtual std::string& GetTableName()override{ return table_name; }
	virtual std::string& GetTableOrgName()override{ return table_org_name; }
	virtual unsigned long GetFieldType()override{ return type; }
	virtual unsigned long GetFlags()override{ return flags; }

	void EnableFlags( unsigned long __type__, bool enable = true )override{ flags = (enable == true ? flags | __type__ : flags ^ __type__);}
	void DisableFlags( unsigned long __type__ )override{ EnableFlags( __type__, false ); }
};

///
///数据库域数组定义
typedef std::vector< CMySQLField > ArrayField;
typedef std::vector< CMySQLTuple > ArrayRows;
///数据库数据行定义
typedef std::vector< CMySQLTuple > ArrayRows;
///
///
typedef  ArrayField  ArrayMySQLField;

class CltrpBase : public CTRP_CRM_DB
{
public:
	enum emErrCode {
		ERR_ALLOC_MEM = -1000,
		DB_QUERY_ERR,
		DB_RES_OUTOFNUM,
		DB_CONN_ERR,
		DB_LIB_INIT_ERR,
		OK = 0
	};
private:
	///服务器基本信息:
	std::string       server_info;
	std::string       client_info;
	unsigned long     server_version;
	unsigned long     client_version;
	std::string       host_info;
	std::string       staus;
	///连接数据库用的服务器地址等信息
	std::string       server_add;
	int               port;
	std::string       user_name;
	std::string       db_name;
	std::string       password;
	///数据库连接保持用数据
	MYSQL           * p_mysql_connect;
	MYSQL_RES       * p_result;  //操作返回结果
	MYSQL_FIELD     * p_fd;
	MYSQL_ROW         m_row;

	bool              m_conn_status; //当前连接状态
	/**
	 数据库查询结果数据
	*/
	ArrayRows         data_rows;
	ArrayField        m_field_info;
	std::string       err_msg;
	int               err_code;

	std::string       information;
public:
	  /**
	   * @brief   初始化服务器连接.自动连接MySQL服务器,同时完成服务器客户端等信息索取和存储.此外为了检查服务器是否保持正常连接
	   *  使用计时器定期检查服务器连接情况.
	   * @param    const std::string& server,  服务器地址
	   * @param std::string& user,      用户名
	   * @param  const std::string& pass,      用户密码
	   * @param  const std::string& db,         连接服务器的默认数据库
	   * @param  int _port,                          端口号,通常默认为3306
	   */
	CltrpBase(const std::string& server = "localhost",
		const std::string& user = "root",
		const std::string& password = "admin",
		const std::string& db = "samp_db",
		int port = 3306);
	virtual ~CltrpBase();
	/**
	 * @brief 枚举数据库
     * @param dbs[ O ] , 数据库名称
     * @return 成功操作返回0,否则返回错误代码
	*/
	virtual int CDB_EnumDBS( ArrayString& dbs );
	/**
	 * @brief 枚举当前数据库数据表
	 * @param tables
	 * @return
	 */
	virtual int CDB_EnumTables( ArrayString& tables);
	/**
	 * @brief 枚举指定数据库的数据表
	 * @param
	 * @return
	 */
	virtual int CDB_EnumTables( const std::string& db, ArrayString& tables );
	/**
	*/
	inline int GetCount(){ return data_rows.size(); }
	/**
	*/
	bool GetRow(int row, ArrayString& __row);
	/**
	*/
	unsigned long CDB_GetFieldType( int col ){ return m_field_info[ col ].GetFieldType(); }
	/**
	*/
	void CDB_GetFieldName( int col, std::string& name){ name = m_field_info[ col ].GetFieldName();}
	/**
	*/
	std::string& CDB_GetFieldName(int col ){ return m_field_info[ col ].GetFieldName(); }
	/**
	*/
	std::string& CDB_GetFieldOrgName( int col ){ return m_field_info[ col ].GetFieldOrgName();}
	/**
	*/
	unsigned long CDB_GetFieldTye( int col ){ return m_field_info[ col ].GetFieldType();}
	/**
	*/
	errCode CDB_Query(const std::string& sql ){ return MySQL_Query(sql);}
	/**
	*/
	int CDB_Connect(const std::string& ,//add,
			int ,//port,
			const std::string&,// user,
			const std::string&,// password,
			const std::string& /*db*/) {return 0;}
	/**
	*/
	int CDB_Disconnect();
	/**
	*/
	int CDB_NumCols();
	/**
	*/
	int  CDB_NumRows() ;
	/**
	*/
	int CDB_ChangePass(const std::string& old, const std::string& _new );
	/**
	*/
	int CDB_SetCharSet(const std::string& char_set){ return MySQL_set_char_code(char_set);}
	/**
	*/
	int CDB_EnumColName(const std::string& table, ArrayString& col_names );
	/**
	*/
	int  CDB_GetRow(int pos, ArrayString& row);
	/**
	*/
	std::string&  CDB_GetErrMsg(){ return err_msg; }
	/**
	*/
	unsigned long  CDB_Last_Insert_ID() { return MySQLLastInserId();}
	/**
	*/
	unsigned long  CDB_Last_Insert_ID(const std::string&) { return 0;}
	//服务器管理类函数
	/**
	*/
	bool CDB_GetServerInfo(std::string& info);
	/**
	*/
	bool CDB_GetClientInfo(std::string& infor) ;
	/**
	*/
	int CDB_GetServerType(){ return 0; }
	///枚举当前的服务器进程，用户通过GetRow函数取用
	/**
	*/
	bool CDB_ProcessList();
	/**
	*/
	bool CDB_Kill(unsigned long pid);
	/**
	*/
	unsigned long CDB_NumQuerys() ;
	/**
	*/
	unsigned long CDB_NetLoad() ;
	/**
	*/
	std::string CDB_GetDBName() { return db_name; }
	/**
	*/
	bool CDB_GetValue(int row, int col, uint8_t& value);
	bool CDB_GetValue(int row, int col, uint16_t& value);
	bool CDB_GetValue(int row, int col, unsigned int& value);

	bool CDB_GetValue(int row, int col, unsigned long& value );

	bool CDB_GetValue(int row, int col, int& value) ;
	bool CDB_GetValue(int row, int col, long& value) ;
	bool CDB_GetValue(int row, int col, bool& value) ;
	bool CDB_GetValue(int row, int col, double& value);
	bool CDB_GetValue( int row, int col,std::string& value);
    bool CDB_GetValue( int row , int col, long long& value );
    bool CDB_GetValue( int row , int col, uint64_t& value );
	///取大型数据
	bool CDB_GetValue(int row, int col, void *buffer);
	/**
	*/
	void CDB_AutoCommit( bool flag );
	/**
	*/
	bool CDB_Commit();
	/**
	*/
	inline bool  is_connect(){return m_conn_status;}
	/**
	*/
	int MySQL_CreatUser( const std::string& username);
	/**
	*/
	int Del_User(std::string& user);
	/**
	 *  目的：设置用户密码;
	 *  方法：
	 *  参数：wxString& user,要设置密码的用户名;
         * wxString& p,密码
         * 如果user = "",则设置当前用户密码
	 */
	void MySQL_SetPassword(const std::string& user, const std::string& p);
	/**
	*/
	bool Is_ResultOK();
	/**
	*/
	void FreeResult();
	/**
	*/
	MYSQL_ROW MySQL_Fetch_Row();
	/**
	*/
	unsigned long*  MySQL_fetch_length();
	/**
	*/
	unsigned long  MySQL_num_rows();
	/**
	*/
	MYSQL_FIELD* MySQL_fetch_fields();
	/**
	*/
	MYSQL_FIELD* MySQL_fetch_field();
	/**
	*/
	std::string MySQL_get_field_name();
	/**
	*/
	unsigned int MySQL_get_col_num();
	/**
	*/
	void  MySQL_row_seek(my_ulonglong offset);
	/**
	*/
	int MySQL_select_db(std::string& str);
	/**
	*/
	int MySQL_use_table(std::string& str);
	/**
	*/
	void MySQL_error_message(std::string& error);
	/**
	*/
	int MySQL_set_char_code(const std::string& str);
	/**
	*/
	errCode MySQL_Query(const std::string& sql_com);
	/**
	*/
	my_ulonglong MySQLLastInserId();
	/**
     * @brief 检查服务器是否正常连接。
     * @return 如果正常连接返回ture，否则返回false
	*/
    bool  CDB_Ping();
 private:
	/**
	*/
	void do_connect();

	/**
	 * 初始化连接信息, 如服务器名称,版本,客户端名称版本等
	*/
	void init_connect_info();
};

#endif // CLTRPBASE_H

