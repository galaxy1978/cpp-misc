/**
 * @brief libuv 事件循环控制器。所有的libuv事件循环都应该依赖于这个模块。如果硬件设备是多核的
 *   可以把这个模块设计成多个线程循环的模块。否则应该是一个循环线程
 * @version 1.0
 * @date 2018-5-1
 * @author 宋炜
*/

#ifndef __UVLPER_HPP__
#define __UVLPER_HPP__
#include <uv.h>
#include <atomic>
class looper
{
friend looper * CreateLooper();

public:
    static uv_loop_t   	    * pt_loop;
	std::atomic<bool>         m_is_run;
private:
	/**
	 * @brief
	*/
	looper();
public:
	looper( const looper& b ) = delete;
	looper& operator =( const looper& b ) = delete;
	virtual ~looper();
	/**
     * @brief 启动循环
	*/
	void run( uv_run_mode mode = UV_RUN_DEFAULT );
	/**
     * @brief 停止运行
	*/
	void stop();
	/**
     * @brief 获取循环指针
	*/
	static uv_loop_t * get()
	{
		return pt_loop;
	}
};
/**
 * @brief 事件循环器工厂函数。如果对象存在则会创建失败返回空。否则返回循环器对象
 */
extern looper * CreateLooper();
/**
 * @brief 获取循环器
 */
extern looper * GetLooper();	
#endif
