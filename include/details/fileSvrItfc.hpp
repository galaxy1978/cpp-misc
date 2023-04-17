/**
 * @brief 文件服务器接口类。给文件操作提供一个统一的接口定义
 * @version 1.0
 * @author 宋炜
 * @date 2022-10-27
 */
#include <functional>
#include "ary_str.hpp"
#pragma once
namespace wheels{
struct iFileService
{
	enum emErrCode{
        ERR_DOWNLOAD_OSS = -10000,                   // oss下载失败
        ERR_SYS_VAR,
		ERR_CANCEL_TRANSFER,
		ERR_UNSUPPORTED_PROTOCOL,           // CURL不支持指定协议
		ERR_INIT_CURL,                      // 初始化CURL错误
		ERR_CONNECT_SVR,                    // 连接服务器错误
		ERR_FILE_DOWNLOAD_TRANSFER,         // 下载传输过程错误
		ERR_FILE_UPLOAD_TRANSFER ,          // 上传文件传输过程错误
		ERR_CTX_NULL,                       // CURL 上下文对象空
		ERR_ALLOC_MEM,                      // 内存分配失败
		ERR_CREATE_DEST_FILE,               // 创建目的文件失败
		ERR_CURL_SET_OPT,                   // 配置CURL参数失败
		ERR_USER_OR_PSWD,                   // 用户名或者密码错误
		ERR_CURL_PERFORM,                   // 执行CURL失败
		ERR_OPEN_LOC_FILE,                  // 打开本地文件失败
		ERR_NO_SRC_FILE,
		ERR_DESC_NULL,
		ERR_EMPTY_URL,
		ERR_FILE_NOT_FOUND,                 // 远程文件不存在
		ERR_ACCESS_NOT_SUPPORT,             //
		STATUS_TRANS_FINISH,                // 数据传输完成
		STATUS_TRANSING,                    // 数据正在传输
		ERR_INIT_FTP,
		ERR_DELE_OSS_FILE,
		ERR_LS_OSS_FILE,
        ERR_UPLOAD_OSS,
        OK = 0
	};
	virtual void bufferSize( size_t s ) = 0;
	/**
	 * @brief 下载文件操作
	 * @param from[ I ]源文件
	 * @param dest[ I ]目标文件
	 * @param autoRun[ I ]，自动开始执行操作
	 * @return 操作成功返回true，否则返回false
	 */
	virtual bool download( const std::string& from , const std::string& dest = "" , bool autoRun = true ) = 0;
	/**
	 * @brief 下载操作函数。操作完成后使用回调函数通知操作结果
	 * @param cb , 操作完成后的回调函数。这个函数用来通知操作结果，函数的原型如下：
     *             bool fun( errCode err , size_t dataSize , const std::string& fileStorePath );
	 */
    virtual void download( const std::string& from , const std::string& dest , std::function< bool ( emErrCode , uint8_t * ,size_t  ) > cb , bool autoRun = true ) = 0;
	/**
	 * @brief 将数据下载到内存中
	 */
    virtual void download( const std::string& from , char * buffer , std::function< bool ( emErrCode , uint8_t * ,size_t ) > cb , bool autoRun = true ) = 0;

	/**
	 * @brief 批量下载文件。
	 * @param from
	 * @param dest
	 * @param autoRun
	 * @return 成功操作返回true, 否则返回false
	 */
	virtual bool download( const ArrayString& from , const ArrayString& dest , bool autoRun = true ) = 0;
	/**
	 * @brief 批量下载文件，使用回调函数通知操作结果和过程
	 */
    virtual void download( const ArrayString& from , const ArrayString& dest , std::function< bool ( emErrCode , uint8_t * ,size_t ) > cb, bool autoRun = true ) = 0;
	/**
	 * @brief 上传文件到服务器
	 * @param file[ I ], 本地文件
	 * @param remote_file[ I ], 远程文件
	 * @param autoRun[ I ]，调用后是否自动运行，如果不是自动运行任务将在后台挂起
	 * @param cb[ I ], 进度回调函数用来通知上传进度
	 * @return 成功操作返回true，否则返回false
	 * @note 目前仅仅支持自动运行
	 */
	virtual bool upload( const std::string& file ,  const std::string& remote_file ,  bool autoRun = true ) = 0;
    virtual void upload( const std::string& file ,  const std::string& remote_file, std::function< bool ( emErrCode , size_t )> cb ,bool autoRun = true ) = 0;
	/**
	 * @brief 批量上传操作
	 * @param from[ I ]
	 * @param dest[ I ]
	 * @param autoRun[ I ]
	 */
	virtual bool upload( const ArrayString& from , bool autoRun = true ) = 0;
    virtual void upload( const ArrayString& file , std::function< bool ( emErrCode , size_t )> cb , bool autoRun ) = 0;
	/**
	 * @brief 删除远程文件
	 * @param rem[ I ] 远程文件名
	 */
	virtual bool del( const std::string& rem ) = 0;
	/**
	 * @brief 设置用户名和密码
	 */
	virtual void setUser( const std::string& acc , const std::string& pswd ) = 0;

	/**
	 * @brief 指定URL
	 */
	virtual void addUrl( const std::string& url ) = 0;
	virtual void run() = 0;
    virtual void run( std::function< bool ( emErrCode , size_t , int , const std::string& ) > cb ) = 0;
	/**
	 * @brief 列出远程目录内容
	 */
	virtual emErrCode ls( ArrayString& rst ) = 0;
    virtual void ls( std::function< void ( emErrCode , ArrayString& ) > cb ) = 0;
	/**
	 * @brief 和linux access函数一样，用来检查文件的可访问性
	 * @param url[ I ]， 要访问的文件的路径
	 * @param type[ I ]， 检查的类型
	 * @return 操作成功返回OK ，否则返回错误代码
	 */
	virtual emErrCode access( const std::string& url , int type ) = 0;
	/**
	 * @brief 切换远程目录路径
	 * @param path ， 路径字符串
	 */
	virtual emErrCode cd( const std::string& path ) = 0;
	/**
	 * @brief 读取远程文件的文件大小
	 * @param file[ I ] 文件路径
	 * @return 成功操作返回文件大小
	 */
	virtual inline size_t size( const std::string& file ) = 0;

	static std::string errMsg( emErrCode e );
};
}

