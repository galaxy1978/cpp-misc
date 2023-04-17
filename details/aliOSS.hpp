/**
 * @brief ali云OSS文件接口。在默认情况下使用开启断点续传功能。
 * @version 1.0
 * @author 宋炜
 * @date 2022-10-27
 */

// 在网关上节点名称不能使用https协议，但是在windows应用中是可以的。
#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

#include <alibabacloud/oss/OssClient.h>

#include "ary_str.hpp"
#include "details/fileSvrItfc.hpp"

namespace wheels {
	class aliOSS : public iFileService
	{
    public:
        using errCode = emErrCode;
	private:
		std::string __m_endpoint;     // 服务节点
		std::string __m_acckeyId;     // 访问服务身份ID
        std::string __m_acckeySec;    // 访问服务密码
		std::string __m_bktName;      // bucket 名称，类似于windows下的盘符

		std::atomic< size_t >   __m_buff_size;
		
		std::shared_ptr< AlibabaCloud::OSS::ClientConfiguration > __pt_conf;
        std::shared_ptr< AlibabaCloud::OSS::OssClient> __pt_client;
        std::function< bool ( emErrCode , size_t )>    __m_cb;
        std::function< void ( errCode , size_t , int , const std::string& )> __run_cb;
		// 如果要延迟下载，则将任务添加到这个表中。然后在run函数中执行实际下载动作
		std::vector<std::function<bool () > >          __m_missions;
	private:
		/**
		 * @brief 上传文件
		 * @param file[ I ], 本地文件
		 * @param rfile[ I ], 远程文件
		 */
		bool __real_upload_file( const std::string& file , const std::string& rfile );
		/**
		 * @brief 实际执行下载操作的函数。在默认情况下使用断点续传的方式进行下载操作。
		 * @param from[ I ], 来源文件
		 * @param dest[ I ], 目的文件
		 * @return 成功操作返回true，否则返回false
		 */
		bool __real_download_file( const std::string& from , const std::string& dest );
		/**
		 * @brief 将文件下载到内存中
		 * @param from[ I ], 源文件名称
		 * @param buffer[ O ], 目标内存指针
		 * @return 成功操作返回true，否则返回false
		 */
		bool __real_download_file( const std::string& from , char * buffer );
	public:
		/**
		 * @brief 这个函数会从配置文件中读取介个配置参数。其他相关参数采用SDK默认值
		 * @param 预留暂时没有用
		 * 	/fileService/aliOSS/endpoint
    	 * 	/fileService/aliOSS/secId
    	 * 	/fileService/aliOSS/secKey
		 * 	/fileService/aliOSS/bucketName
		*/
        aliOSS(const std::string&);
		aliOSS(const std::string& bucket , const std::string& endpoint , const std::string& acckeyId , const std::string& acckeySec );
        virtual ~aliOSS();

        inline virtual void bufferSize( size_t s )final
        {
            __m_buff_size = s;
        }
		/**
		 * @brief 下载文件操作
		 * @param from[ I ]源文件
		 * @param dest[ I ]目标文件
		 * @param autoRun[ I ]，自动开始执行操作
		 * @return 操作成功返回true，否则返回false
		 */
        virtual bool download( const std::string& from , const std::string& dest = "" , bool autoRun = true ) final;
		/**
		 * @brief 下载操作函数。操作完成后使用回调函数通知操作结果
		 * @param cb , 操作完成后的回调函数。这个函数用来通知操作结果，函数的原型如下：
         *             bool fun( errCode err , size_t dataSize , const std::string& fileStorePath );
         *        返回值主要用来控制在多个文件下载的时候如果一个文件下载后是否需要再执行下一个下载操作
		 */
        virtual void download( const std::string& from , const std::string& dest , std::function< bool ( errCode , size_t  ) > cb , bool autoRun = true ) final;
		/**
		 * @brief 将数据下载到内存中
		 * @param from[ I ]源文件
		 * @param buffer[ O ] 目标内存
		 * @param cb[ I ] 回调函数
		 * @param autoRun[ I ] 是否自动运行
		 */
        virtual void download( const std::string& from , char * buffer , std::function< bool ( errCode , size_t ) > cb , bool autoRun = true ) final;

		/**
		 * @brief 批量下载文件。
		 * @param from[ I ]
		 * @param dest[ I ]
		 * @param autoRun[ I ]
		 * @return 成功操作返回true, 否则返回false
		 */
		virtual bool download( const ArrayString& from , const ArrayString& dest , bool autoRun = true ) final;
		/**
		 * @brief 批量下载文件，使用回调函数通知操作结果和过程
		 */
        virtual void download( const ArrayString& from , const ArrayString& dest , std::function< bool ( errCode , size_t ) > cb, bool autoRun = true ) final;
		/**
		 * @brief 上传文件到服务器
		 * @param file[ I ], 本地文件
		 * @param remote_file[ I ], 远程文件
		 * @param autoRun[ I ]，调用后是否自动运行，如果不是自动运行任务将在后台挂起
		 * @param cb[ I ], 进度回调函数用来通知上传进度
		 * @return 成功操作返回true，否则返回false
		 * @note 目前仅仅支持自动运行
		 */
        virtual bool upload( const std::string& file , const std::string& remote_file ,  bool autoRun = true ) final;
        virtual void upload( const std::string& file , const std::string& remote_file, std::function< bool ( errCode , size_t )> cb ,bool autoRun = true ) final;
		/**
		 * @brief 批量上传操作
		 * @param from[ I ]
		 * @param dest[ I ]
		 * @param autoRun[ I ]
		 */
		virtual bool upload( const ArrayString& from , bool autoRun = true ) final;
        virtual void upload( const ArrayString& file , std::function< bool ( errCode , size_t )> cb , bool autoRun ) final;
		/**
		 * @brief 删除远程文件
		 * @param rem[ I ] 远程文件名
		 */
		virtual bool del( const std::string& rem ) final;
		/**
		 * @brief 设置用户名和密码
		 */
		virtual void setUser( const std::string& acc , const std::string& pswd ) final;

		/**
		 * @brief 指定URL
		 */
		virtual void addUrl( const std::string& url ) final;
		virtual void run() final;
        virtual void run( std::function< bool( emErrCode , size_t , int , const std::string& ) > cb ) final;

		/**
		 * @brief 列出远程目录内容
		 */
		virtual errCode ls( ArrayString& rst ) final;
		virtual void ls( std::function< void ( errCode , ArrayString& ) > cb ) final;
		/**
		 * @brief 和linux access函数一样，用来检查文件的可访问性
		 * @param url[ I ]， 要访问的文件的路径
		 * @param type[ I ]， 检查的类型
		 * @return 操作成功返回OK ，否则返回错误代码
		 */
		virtual errCode access( const std::string& url , int type ) final;
		/**
		 * @brief 切换远程目录路径
		 * @param path ， 路径字符串
		 */
		virtual errCode cd( const std::string& path ) final;
		/**
		 * @brief 读取远程文件的文件大小
		 * @param file[ I ] 文件路径
		 * @return 成功操作返回文件大小
		 */
		virtual inline size_t size( const std::string& file ) final;
	};
}
