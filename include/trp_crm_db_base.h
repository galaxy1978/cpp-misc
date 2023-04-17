/*
-------------------------------------------------------------------------------------------------------------------
Block code:         0-0-
Block name:         Data base general socket;
Block type:         0
Block description:  由于在TRP_CRM 中使用两种不同的数据库MySQL和Sqlite3分别管理服务器数据和本地数据。两种数据库都是
                    采用c API，而两者的API是不同通用的，接口名称也完全不一样。为了程序编写方便，对两个数据库进行
                    抽象处理，安排出共同需要的数据接口。而这些接口的名字都采用纯虚函数处理，说明程序之间的接口，而
                    不管其具体的实现。然后再程序的所有地方都使用这个虚拟接口处理，保证了程序编写的一致性和高效性。
Date:               2010-12-8 20:04
License:            TRP_CRM license
Author:             galaxy song
--------------------------------------------------------------------------------------------------------------------
*/

#ifndef             __TRP_CRM_DB_BASE_H__
#define             __TRP_CRM_DB_BASE_H__

#include <string>
#include "ary_str.hpp"
using namespace wheels;
/*
数据元组定义
*/
class CDBTuple
{
public:
    virtual bool GetValue( int col, bool& value) = 0;
    virtual bool GetValue( int col, uint8_t& value )= 0;
    virtual bool GetValue( int col, uint16_t& value )  = 0;
    virtual bool GetValue( int col, int& value ) = 0;
    virtual bool GetValue( int col, unsigned int& value) = 0;
    virtual bool GetValue( int col, long& value ) = 0;
    virtual bool GetValue( int col, unsigned long& value ) = 0;
    virtual bool GetValue( int col, float& value) = 0;
    virtual bool GetValue( int col, double& value ) = 0;
    virtual bool GetValue( int col, std::string& value ) = 0;
/*
为了去用如BLOB类型的数据,需要分配大块的存储则使用这个函数.
*/
    virtual bool GetValue( int col, void *buffer , size_t len ) = 0;
};

class CDBField
{
public:
    virtual void SetFieldName( char* name ) = 0;
    virtual void SetFieldOrgName( char* name) = 0;
    virtual void SetDbName( char* name ) = 0;
    virtual void SetDbOrgName( char * name ) = 0;
    virtual void SetTableName( char* name ) = 0;
    virtual void SetTableOrgName( char* name ) = 0;
    virtual void SetFieldType( unsigned long type ) = 0;
    virtual void SetFlags( unsigned long type ) = 0;
    virtual void EnableFlags( unsigned long type, bool enable = true ) = 0;             ///指定或者取消属性
    virtual void DisableFlags( unsigned long type ) = 0;

    virtual std::string& GetFieldName() = 0;
    virtual std::string& GetFieldOrgName() = 0;
    virtual std::string& GetDbName() = 0;
    virtual std::string& GetDbOrgName() = 0;
    virtual std::string& GetTableName() = 0;
    virtual std::string& GetTableOrgName() = 0;
    virtual unsigned long GetFieldType() = 0;
    virtual unsigned long GetFlags() = 0;
};


class CTRP_CRM_DB
{
public:
	/// @brief 错误代码定义
	enum errCode{
		ERR_NULL_ROWS = -1000,		// 结果集二维数组指针空
		ERR_NULL_FIELD_RESULT,  // 结果集列信息集合不存在
		ERR_EMPTY_USER,		// 用户名称空
		ERR_MYSQL_NOT_CONNECTED, // 数据库没有连接
		ERR_TABLE_NAME_EMPTY,	// 数据表名称空
		OK = 0
	};

public:
    virtual errCode CDB_Query(const std::string& sql ) = 0;
    virtual int CDB_Connect(const std::string& add, int port,const std::string& user,const std::string& password, const std::string& db) = 0;
    virtual int CDB_Disconnect() = 0;
//enum tables if successed return count of tables ,tables contain table names, otherwise return error code less than 0
    virtual int CDB_EnumDBS( ArrayString& dbs ) = 0;
    virtual int CDB_EnumTables(ArrayString& tables) = 0;
    virtual int CDB_EnumTables( const std::string& db, ArrayString& tables ) = 0;

    virtual int CDB_NumCols() = 0;
    virtual int CDB_NumRows() = 0;
    virtual int CDB_ChangePass(const std::string& old, const std::string& _new ) = 0;
    virtual int CDB_SetCharSet(const std::string& char_set) = 0;
    virtual int CDB_EnumColName(const std::string& table, ArrayString& col_names ) = 0;
    virtual int CDB_GetRow(int pos, ArrayString& row) = 0;
    virtual std::string& CDB_GetErrMsg() = 0;
    virtual unsigned long CDB_Last_Insert_ID() = 0;
    virtual unsigned long CDB_Last_Insert_ID(const std::string& table)  = 0;
    /**
        \brief 确定是否需要将sql命令试用自动事务提交方式进行处理。
        \param bool flag, 当flag=true的时候自动提交事务，否则试用CDB_Commit提交事务
    */
    virtual void CDB_AutoCommit( bool flag ) = 0;
    virtual bool CDB_Commit() = 0;
//服务器管理类函数
    virtual bool CDB_GetServerInfo(std::string& info) = 0;
    virtual bool CDB_GetClientInfo(std::string& infor) = 0;
    virtual bool CDB_ProcessList() = 0;
    virtual bool CDB_Kill(unsigned long pid) = 0;
    virtual unsigned long CDB_NumQuerys() = 0;
    virtual unsigned long CDB_NetLoad() = 0;
    virtual std::string CDB_GetDBName() = 0;

    virtual bool CDB_GetValue(int row, int col, uint8_t& value) = 0;
    virtual bool CDB_GetValue(int row, int col, uint16_t& value) = 0;
    virtual bool CDB_GetValue(int row, int col, unsigned int& value) = 0;
    virtual bool CDB_GetValue( int row, int col, unsigned long& value ) = 0;
    virtual bool CDB_GetValue(int row, int col, int& value) = 0;

    virtual bool CDB_GetValue(int row, int col, long& value) = 0;

    virtual bool CDB_GetValue(int row, int col, bool& value) = 0;

    virtual bool CDB_GetValue(int row, int col, double& value) = 0;

    virtual bool CDB_GetValue( int row, int col,std::string& value) = 0;
    virtual bool CDB_GetValue(int row, int col, void *buffer) = 0;

    virtual void CDB_GetFieldName( int col, std::string& name) = 0;
    virtual std::string& CDB_GetFieldName(int col ) = 0;
    virtual std::string& CDB_GetFieldOrgName( int col ) = 0;
    virtual unsigned long CDB_GetFieldType( int col ) = 0;
    virtual bool is_connect() = 0;
    virtual int CDB_GetServerType() = 0;
};
#endif
