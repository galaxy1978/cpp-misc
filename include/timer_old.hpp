/**
 * @brief timer对象，在后台运行当计时触发条件成熟后触发运行指定函数。目前不支持带
 *        参数的通知.这个对象实现全部采用标准的库实现，可以有比较好的通用型。但是要求
 *        编译器支持c++11.
 *
 * @version 0.2
 * @date 2017-2-7
 * @author 宋炜
 */


#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

//#include "defines.hpp"
#include "misc.hpp"

class CTimer
{
private:
	std::atomic<bool>           m_is_one_shot;               // 计时器是否是单次计时器
	std::atomic<long>    	    m_tick;                      // 最大计时量
	std::atomic<bool>    	    m_is_run;                    // 计时线程是否正在运行
	std::atomic<bool>           m_reset;                     // 复位计时器

	std::mutex   	            m_mutex;

	std::function<void(  )>     m_cb;                        //要调用的事件发生的时候调用这个函数对象
public:
	/**
	 * @brief 计时器的默认初始化函数，这个初始化函数没有指定回调函数，能够响应计时时间，需要调用SetOwner
	 *   指定事件触发的动作。使用CTimer( ms , oneShot ),可以初始化计时时长，使用CTimer( fun , ms , oneShot )
	 *   可以指定事件响应动作
	 * @param ms ，时长，单位为ms
	 * @param oneShot ， 设置为true，完成一次时间响应之后，计时器停止计时，否则计时响应时间后重启计时
	 * @param fun，事件响应函数
	 */
	CTimer(  ):m_is_one_shot( false ),m_tick( 0 ), m_is_run( false ) , m_reset( false ) {  }

	CTimer( long ms , bool oneShot = false )
		:m_is_one_shot( oneShot ),m_tick( ms ) , m_is_run( false ) , m_reset( false ){}

	CTimer( std::function<void( )> fun , long ms , bool oneShot = false ):
		m_is_one_shot( oneShot ),m_tick( ms ) , m_is_run( false ) , m_reset( false ),m_cb( fun )
	{}

	virtual ~CTimer()
	{
		Stop();
		std::this_thread::yield();
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ));
	}
	/**
	 * @brief 设置和获取计时器计时长度
	*/
	inline long operator()()const
	{
		return m_tick;
	}
	inline void operator()( long t )
	{ 
		m_tick = t; 
	}
	/**
	 * @brief 启动或者重启计时器。
	 * @param ms ， 计时时长，单位ms，当ms小于0的时候，使用就计时时长启动计时器
	 * @param oneShot ， 是否单次计时，true单次计时，否则连续计时
	 */
	void Start( long ms = -1, bool oneShot = false );
	/**
	 * @brief 指定事件响应函数，
	 * @param fun ，事件响应的函数对象，使用std::bind完成或者直接指定函数对象
	 */
	void SetOwner( std::function<void(  )> fun );
	/**
	 * @brief 停止计时器
	 */
	void Stop(  );
	/**
	 * @brief 判断计时器是否正在运行
	 */
	bool IsRuning(  );
	/**
	 * @brief 复位计时，让计时器从零开始计时
	 */
	void Reset(){ m_reset = true ; }
private:
	/**
	 * @brief 后台计时线程
	*/
	void thd_fun(  );
};
#endif
