/**
 * @brief 配置文件模块。配置文件格式定义
 *   <CONF_FILE VER="1.0" author="">
 *       <var name="" type="" value=""/>
 *       <dir name="">
 *           <var name="" type="" value="" />
 *           ...
 *       </dir>
 *       ...
 *   </CONF_FILE>
 *   var:  变量
 *   dir:  目录
 *   name: 变量名称
 *   type: 变量类型，bool , int , long , float
 * @version 0.1
 * @date 2016-11-24
 * 2018-3-18  添加数据更改功能 
 * 2018-9-27  添加数据增加和删除功能
*/
#ifndef CCONFFILE_H
#define CCONFFILE_H

#include <string>
#include <vector>

#include <tinyxml.h>

#include "misc.hpp"
//#include "defines.hpp"

class CConfFile
{
	// 目录遍历模块，解析目录描述字符串，在XML文件节点中查找指定的节点。
	class CPathTravler
	{
	public:
		enum emErrCode{
			PATH_DATA_EMPTY = -1000,
			PATH_NOT_EXIST,
			ERR_GET_NAME,
			ERR_GET_ATTR,
			ERR_DATA_TYPE,
			OK = 0
		};
	public:
		typedef std::vector< std::string>   ArrayPath;
		/**
		 * @brief 构造函数
		 * @param const std::string& path , 目录描述字符串，分隔符号使用/
		 * @param CConfFile* conf , 配置xml文档指针。
		 * @param TiXmlElement* current , 当前所在的位置，NULL表示当前位置为根节点。
		 */
		CPathTravler( const std::string& path , CConfFile* conf , TiXmlElement* current = NULL );
		virtual ~CPathTravler(){}
		/**
		 * @brief 获取路径的XML节点
		 * @param path，路径字符串
		 * @retval 成功发挥节点指针，否则返回NULL
		 * @note 不支持UNICODE编码，如果需要使用UNICODE编码，需要将数据转换成UTF8
		*/
		TiXmlElement* operator()();
		TiXmlElement* operator()( const std::string& path );
		/**
		 * @brief
		 */
		TiXmlElement* Cd( const std::string& path );

		operator bool(){ return (p_file != NULL); }
	private:
		/**
		* @brief 解析路径。将路径解析成字符数组
		* @retval 成功返回0，否则返回错误代码
		*/
		int parse_path( const std::string& path );
		/**
		* @brief 按照路径结构，从起始地点到目标地点
		*/
		int run();

		/**
		 * @brief 向父亲节点移动
		*/
		int go_to_parent();
		/**
		* @brief 向根节点移动
		*/
		int go_to_root();
		/**
		* @brief 向子节点移动
		*/
		int go_to_child( const std::string& );
		/**
		* 向兄弟节点移动
		*/
		int go_to_brother( const std::string& );
	private:
		ArrayPath  m_path_data;
		TiXmlElement  *p_current;       //当前路径的指针
		TiXmlElement  *p_rst;           //运行的结果

		CConfFile     *p_file;
	};

    friend class CPathTravler;

public:
	enum err_code{
		CAN_NOT_LOAD_FILE = -2000,          //无法装入配置文件
		ERR_FILE_NOT_EXIST,		    //文件不存在
		CAN_NOT_FIND_XML_ROOT,              //无法找到XML根节点
		ERROR_FILE_VERSION,                 //文件版本错误
		ERROR_FILE_FORMAT,                  //文件格式错误
		ERROR_FILE_ROOT_NAME ,              //文件跟节点名称错误
		ERROR_VAR_NO_NAME,                  //变量没有名称
		ERROR_VAR_VALUE_TYPE,               //变量类型不匹配
		ERROR_FILE_DIR_FORMAT,
		ERROR_VAR_NOT_EXIST,                // 变量不存在
        ERR_GET_ATTR,
		ERROR_SAVE_FILE,
		ERR_PATH_GRRAMA,                    // 路径格式错误
		ERR_VAR_NAME_EMPTY,                 // 变量名称空
		ERR_PATH_NOT_EXIST,                 // 路径不存在
        ERR_GET_NAME,
		ERR_DEL_ROOT,                       // 执行了删除根节点的操作
		ERR_DEL_NODE,                       // 删除节点失败
		ERR_OBJ_NULL,
		OK = 0
	};

	typedef err_code e_error_conf_file;
	typedef err_code emErrCode;
public:
	CConfFile( const std::string& file );
	virtual ~CConfFile();
	/**
	 * @brief 获取变量数据
	 */
	void GetValue( const std::string& var_name , std::string& value );
	void GetValue( const std::string& var_name , int& value );
	void GetValue( const std::string& var_name , long& value );
	void GetValue( const std::string& var_name , bool& value );
	void GetValue( const std::string& var_name , float& value );
	void GetValue( const std::string& var_name , double& value );
	/**
	 * @brief 设置数据
	 */
	void SetValue( const std::string& var , const std::string& v );
	void SetValue( const std::string& var , int v );
	void SetValue( const std::string& var , long v );
	void SetValue( const std::string& var , bool v );
	void SetValue( const std::string& var , float v );
	void SetValue( const std::string& var , double v );
	/**
	 * @brief 添加数据
	 * @param var
	 * @param v
	 * @exception std::bad_alloc
	 * @exception err_code 
	 */
	void AddValue( const std::string& var , const std::string& v );
	void AddValue( const std::string& var , int v );
	void AddValue( const std::string& var , long v );
	void AddValue( const std::string& var , bool v );
	void AddValue( const std::string& var , float v );
	void AddValue( const std::string& var , double v );
	/**
	 * @brief 删除数据。
	 * @param var 
	 * @param del_children , true 标识如果变量名称给定的目录名称，则会删除目录内的所有变量；否则将放弃删除
	 * @exception err_code 
	 */
	void Del( const std::string& var , bool del_children = true );
	/**
	 * @brief 更改当前路径
	 */
	void Cd( const std::string& path );

	/**
	 * @brief 保存文件
	 */
	e_error_conf_file Save();
	e_error_conf_file Save( const std::string& file );

	static std::string errMsg( emErrCode e );
private:
	/**
		@brief 检查设置文件版本
		@retval 成功返回true,否则返回false
		@exception，ERROR_FILE_FORMAT， 文件格式不正确
	*/
	bool check_file();
	/**
		@brief 查找名称为name的变量。
		@param const std::string& name , 变量名称。
		@retval 成功返回节点的指针，否则返回NULL
		@exception
	*/
	TiXmlElement* find_var( const std::string& name );
	/**
	 * @brief 判断是否是变量。
	 * @param node , 节点指针
	 * @return 如果是变量节点则返回true， 否则返回false
	 * @exception err_code ， 当node返回的名称为空的时候抛出ERR_OBJ_NULL
	 * @note 这个函数不判断传入的指针是否为空，使用的使用应该首先判断指针是否有效
	 */
	bool is_var( TiXmlElement * node );
private:
	std::string         m_file;
	TiXmlDocument       m_doc;
	TiXmlElement*       p_root;
	TiXmlElement*       p_current;
};

#endif // CCONFFILE_H
