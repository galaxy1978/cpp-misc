/**
 * @brief FTP接口，这个功能使用libcurl来实现
 * @version 1.0
 * @date 2019-7-21
 * @author 宋炜
 */
#ifndef __FTP_HPP__
#define __FTP_HPP__

#include <string>
#include <curl/curl.h>
#include <memory>
#include <functional>
#include <atomic>
#include <fstream>

#include "ary_str.hpp"
#include "misc.hpp"
#include "url.hpp"
#include "tribool.hpp"     // 三值布尔


namespace wheels {
class rcFtp 
{
        friend size_t ftp_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
        friend size_t ftp_read_callback(char *buffer, size_t size, size_t nitems, void *userdata);
        friend size_t on_ftp_size( char *ptr, size_t size, size_t nmemb, void *userdata );
        friend size_t on_nlist( char *ptr, size_t size, size_t nmemb, void *userdata );
        friend size_t onConnectCb( char * , size_t , size_t , void *);
        friend size_t onChkInst(char *buffer,   size_t size,   size_t nitems,   void *userdata);
        friend size_t ftp_write_cb_data(char *ptr, size_t size, size_t nmemb, void *userdata);
        friend int on_upld_progress(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
public:
        typedef size_t (*curlFun)( char * , size_t , size_t , void *  );
        static const size_t DEFAULT_BUF_SIZE;
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
			OK
		};
        using errCode = emErrCode;

        struct stParams{
                bool     m_passive;
                bool     bin_ascii;
                size_t   buf_size;
                stParams():m_passive( false ) , bin_ascii( true ),buf_size( DEFAULT_BUF_SIZE ){ }
                stParams( bool p , bool b , size_t s = DEFAULT_BUF_SIZE )
                {
                        m_passive = p;
                        bin_ascii = b;
                        buf_size = s;
                }

                stParams( const stParams& b )
                {
                        m_passive = b.m_passive;
                        bin_ascii = b.bin_ascii;
                        buf_size = b.buf_size;
                }

                stParams& operator=( const stParams& b )
                {
                        m_passive = b.m_passive;
                        bin_ascii = b.bin_ascii;
                        buf_size = b.buf_size;

                        return *this;
                }
        };
        /**
         * @brief 传输描述符。在数据传输的过程中描述数据的传送状况喝操作状况
         * @{
         */
        struct stTransmit{
                rcFtp         * p_ftp;              // FTP对象指针
                tribool_t   	m_type;             // 传输类型, triTrue下载，triFalse上传
                std::string 	m_from;             // 源文件URL
                std::string 	m_dest;             // 目标文件路径
                std::ofstream * p_os; 	            // 写入文件对象
                std::ifstream   m_is;   	    // 读取文件对象
                size_t      	m_buf_size;         // 缓冲区大小
                size_t      	m_data_size;        // 缓冲区中数据大小
                char 	      * p_buffer;           // 数据指针
		/// 对于只下载数据并保存在内存中的情况，p_buffer是不断在变化的
                /// 最后所有的数据都会保存在p_final_buffer所指出的内存中
                char          * p_final_buffer;     // 最终数据指针
		size_t          m_final_off;        // 当前数据写入位置

                stTransmit() : p_ftp( nullptr ) , p_os( nullptr ) ,m_buf_size( 0 ),m_data_size(0),p_buffer( nullptr ), p_final_buffer( nullptr ),m_final_off( 0 ){}
                ~stTransmit(){ if( p_buffer ) free( p_buffer); }
        };
        /// @}
#if defined( __TEST__ )
public:
#else
private:
#endif
        CURL                   * p_ctx;		    // 这是一个一直存在CURL句柄，用来处理和侦测服务
        std::string              m_remote_add;      // 远程服务器地址，不包含文件路径
        int                      m_port;            // 远程服务器监听地址
        std::string              m_remote_path;
        std::string              m_local_path;
        std::string              m_usr;             // 用户账户
        std::string              m_pswd;            // 用户密码
        stParams                 m_params;

        ArrayString              m_files;           // 远程文件列表
        ArrayString              m_dests;           // 本地文件列表
        size_t                   m_file_size;       // 当前文件大小
        std::atomic< bool >      m_d_u;             // 分别上传或者是下载 , true上传，false下载
        std::atomic< tribool_t >      m_pause;      // 传输暂停操作记录，triTrue 暂停，triReady 传输 ，triFalse, 取消传输
        std::atomic< bool >      m_is_running;      // 上传或者是下载是否正在运行

        std::function< void ( errCode , size_t ) >  dwld_cb;
        std::function< void ( errCode , size_t , int , const std::string& ) > dwld_cb_ary;
        std::function< void ( errCode , size_t , int , const std::string& )> run_cb;
        std::function< void ( errCode , size_t ) > upld_cb;

#if defined( __TEST__ )
public:
#else
private:
#endif
        /**
         * @brief 初始化状态机
         */
        void __init_fsm();
        /**
         * @brief 连接服务器
         * @return 成功操作返回true，否则返回false
        */
        bool __connect();
        /**
         * @brief 检查服务器属性和状态。特别是检查服务器支持的指令集，方便后续采用合适的指令对
         * 服务器进行初始化或者选择合适的指令进行操作
         * @return 成功操作返回true, 否则返回false
        */
        bool __chk_svr();

        /**
         * @brief 处理错误
         * @param e[ I ], 错误编码
         */
        void do_process_error( errCode e );
        /**
         * @brief 真正执行下载操作的函数
         * @param from[ I ], 远程文件路径
         * @param to[ I ], 要保存的目标文件完整路径
         */
        errCode real_download( const std::string& from , const std::string& to );
        /**
         * @brief 执行下载操作，但是下载后并不保存文件。而是保存在给定数据缓冲区中
        */
        errCode __real_download( const std::string& from , char * data  );

        /**
         * @brief 等候操作结束
         * @param forceFinish ， 如果操作没有完成，是否强制结束传输

         */
        void wait_finish( bool forceFinish = true );

        /**
         * @brief 真正的保存文件的操作
         * @param 传输对象
         * @return 成功操作返回保存的数据长度，否则返回0
         */
        int do_process_save_file( stTransmit * desc );

        /**
         * @brief 修正源文件地址，和目标文件地址. 如果目标文件地址为空，则抽出源文件名称默认的路径组成保存路径存储在目标路径表中
         * @param from，源文件地址
         * @param to， 目标文件地址
         */
        void remody_url( const std::string& from , const std::string& dest );
        /**
         * @brief 响应CURL读取本地数据操作
         * @param buff , 要保存数据的内存指针
         * @param reqLen , 请求读取的数据长度
         * @return 实际读取到的数据长度
         */
        size_t on_read_data( char * buff , size_t reqLen );
        /**
         * @brief 检查是否以匿名方式登录
         * @return 以匿名方式返回true，否则返回false
         */
        bool is_anonymous();

        /**
         * @brief 处理FTP指令操作返回结果
         * @param 传输描述符
         */
        void do_process_cmd( stTransmit * desc );
        /**
         * @brief 读取文件大小
         * @param file[ I ] 文件名称
         * @return 成功操作返回文件大小，否则返回0
         */
        size_t __size( const std::string& file );


        /**
         * @brief 允许命令通道数据
         * @param fun 接收命令的回调函数
         * @return 成功操作返回OK, 否则返回错误代码
        */
        errCode __enCmdCh( curlFun fun );
        /**
         * @brief 关闭命令通道数据
        */
        errCode __disEnCmdCh();
        /**
         * @brief 真正执行上传操作的接口。
         * @param local[ I ],
         * @param remote[ I ],
         * @return 成功操作返回OK，否则返回错误代码
         */
        errCode __real_upload( const std::string& local , const std::string& remote);
        /**
         * @brief 上传文件的时候初始化本地文件读取接口
         * @brief file[ I ] , 本地文件路径
         * @return 成功操作返回文件输入流
         * @exceptions 如果操作错误抛出响应的错误代码
         */
        std::ifstream __init_local_file( const std::string& file );
        /**
         * @brief 初始化上传文件传输控制信息结构
         * @param local[ I ], 本地文件
         * @param remote[ I ], 远程文件路径
         * @param info[ I ], 传输控制信息
         * @exceptions 操作错误抛出相对应的错误代码
         */
        void __init_upload_trans_info( const std::string& local , const std::string& remote , stTransmit& info );
        /**
         * @brief 准备上传CURL环境,配置相关的参数
         * @param ctx[ I ],
         * @param trans[ I ],
         * @return 成功操作返回CURL_OK，否则返回curl相对应的错误代码
         */
        CURLcode __init_upload_curl( CURL * ctx , stTransmit& trans );

        void __do_process_err( errCode e );

        /**
         * @brief
         */
        errCode __access( const std::string& url );
public:
        /**
         * @brief
         * @param url[ IN ] url应该是完整的URL格式
         */
        rcFtp();
        rcFtp( const std::string& url );
        rcFtp( const stParams& params );
        rcFtp( const std::string& remote , const stParams& params );

        virtual ~rcFtp();
        /**
         * @brief 读取文件大小响应函数
         * @param data[ I ], 操作返回的字符串
         * @param size[ I ], 返回字符串的长度
         */
        void onSize( const char * data , size_t size );
        /**
         * @brief 响应暂停操作
         */
        int on_pause( tribool_t status );
        void __on_progress_dwld( size_t now );
        /**
         * @brief 传输过程中操作CURL回调函数
         * @param now当前传输的字节数
        */
        void __on_progress( size_t now );

        /**
         * @brief 响应检查FTP支持指令的情况
         * @param data[ IN ]
         * @param len [ IN ]
         */
        void __onChkInst( const char * data , size_t len );
        /**
         * @brief 响应连接操作的过程中的数据回调
         * @param data[ IN ] ,
         * @param len[ IN ]
         */
        void __onConnect( const char * data , size_t len );
        /**
         * @brief ftp对象数据给出错误。虽然这种情况的可能性不大。
         */
        void __onConnectErr();
        /**
         * @brief 接收到数据的回调函数
         * @param desc ，传输描述符
         */
        int on_recv( stTransmit * desc );
        int on_recv_data( stTransmit * desc );

        /**
         * @brief 执行下载操作，这个函数会阻塞程序运行，直到完成操作
         * @param from , 远程文件路径，应该是完整的FTP路径
         * @param dest , 目标文件路径，应该是完整的文件路径，应该包含路径名称和文件名称
         * @param autoRun , 如果设置为true,则函数调用后自动启动下载，否则应该调用run开始执行下载操作
         * @return 成功操作返回true，否则返回false
         */
        bool download( const std::string& from , const std::string& dest = "" , bool autoRun = true );
        /**
         * @brief 下载操作函数。操作完成后使用回调函数通知操作结果
         * @param cb , 操作完成后的回调函数。这个函数用来通知操作结果，函数的原型如下：
         *             void fun( errCode err , size_t dataSize , const std::string& fileStorePath );
         */
        void download( const std::string& from , const std::string& dest , std::function< void ( errCode , size_t  ) > cb , bool autoRun = true );
        /**
         * @brief 将数据下载到内存中
        */
        void download( const std::string& from , char * buffer , std::function< void ( errCode , size_t ) > cb , bool autoRun = true ) ;
        /**
         * @brief 批量下载文件。
         * @param from
         * @param dest
         * @param autoRun
         * @return 成功操作返回true, 否则返回false
         */
        bool download( const ArrayString& from , const ArrayString& dest , bool autoRun = true ) ;
        /**
         * @brief 批量下载文件，使用回调函数通知操作结果和过程
         */
        void download( const ArrayString& from , const ArrayString& dest , std::function< void ( errCode , size_t ) > cb, bool autoRun = true ) ;
        /**
         * @brief 上传文件到服务器
         * @param file[ I ], 本地文件
         * @param remote_file[ I ], 远程文件
         * @param autoRun[ I ]，调用后是否自动运行，如果不是自动运行任务将在后台挂起
         * @param cb[ I ], 进度回调函数用来通知上传进度
         * @return 成功操作返回true，否则返回false
         * @note 目前仅仅支持自动运行
         */
        bool upload( const std::string& file ,  const std::string& remote_file ,  bool autoRun = true ) ;
        void upload( const std::string& file ,  const std::string& remote_file, std::function< void ( errCode , size_t )> cb ,bool autoRun = true ) ;
        /**
         * @brief 批量上传操作
         * @param from[ I ]
         * @param dest[ I ]
         * @param autoRun[ I ]
         */
        bool upload( const ArrayString& from , bool autoRun = true ) ;
        void upload( const ArrayString& file , std::function< void ( errCode , size_t )> cb , bool autoRun ) ;
        /**
         * @brief 删除远程文件
         * @param file[ I ] 要删除的文件路径
         */
        bool del( const std::string& file ) ;
        /**
         * @brief 设置用户名和密码
         */
        void setUser( const std::string& acc , const std::string& pswd ) ;
        /**
         * @brief 指定URL
         */
        void addUrl( const std::string& url ){(void)url;}
        void run() ;
        void run( std::function< void ( errCode , size_t , int , const std::string& ) > cb ) ;
        /**
         * @brief 列出远程目录内容
         */
        errCode ls( ArrayString& rst ) ;
        void ls( std::function< void ( errCode , ArrayString& ) > cb ) ;
        /**
         * @brief 和linux access函数一样，用来检查文件的可访问性
         * @param url[ I ]， 要访问的文件的路径
         * @param type[ I ]， 检查的类型
         * @return 操作成功返回OK ，否则返回错误代码
         */
        errCode access( const std::string& url , int type ) ;
        /**
         * @brief 切换远程目录路径
         * @param path ， 路径字符串
         */
        errCode cd( const std::string& path ) ;
        /**
         * @brief 读取远程文件的文件大小
         * @param file[ I ] 文件路径
         * @return 成功操作返回文件大小
         */
        inline size_t size( const std::string& file ) { return __size( file ); }

        static std::string errMsg( errCode e );
};
}
#endif
