/**
 * @brief 遵循POSIX标准的TIMER
 * @version 1.0
 * @author 宋炜
 * @date 2021-11-2
 */

#pragma once

#include <signal.h>
#include <time.h>

#include <atomic>
#include <mutex>
#include <functional>

#include "itimer.hpp"

class linuxTimer: public iTimer
{
	friend void linux_on_timer( sigval info );
public:
	
private:
	std::atomic<bool>           __m_is_one_shot;               // 计时器是否是单次计时器
	std::atomic<long>    	    __m_tick;                      // 最大计时量
	std::atomic<bool>    	    __m_is_run;                    // 计时线程是否正在运行
	std::atomic<bool>           __m_reset;                     // 复位计时器

	struct sigevent             __m_evt;                       // 事件通讯
	std::atomic<timer_t>        __m_id;                        // timerID
	struct itimerspec           __m_new;                       // 计时参数
	std::mutex   	            __m_mutex;

	std::function<void()>     __m_cb;                        //要调用的事件发生的时候调用这个函数对象
private:
	/**
	 * @brief 初始化计时器
	 */
	emErrCode __init_timer();
	void __on_sys_timer();
public:
	linuxTimer();
	linuxTimer( long ms , bool oneShot = false );
	linuxTimer( std::function<void()> fun , long ms , bool onShot = false );
	~linuxTimer(){ if( __m_is_run == true ) Stop(); }
	/**
	 * @brief 设置和获取计时器计时长度
	*/
	inline long operator()()const
	{
		return __m_tick;
	}
	inline void operator()( long t )
	{ 
		__m_tick = t; 
	}
	/**
	 * @brief 启动或者重启计时器。
	 * @param ms ， 计时时长，单位ms，当ms小于0的时候，使用就计时时长启动计时器
	 * @param oneShot ， 是否单次计时，true单次计时，否则连续计时
	 */
	virtual emErrCode Start( long ms = -1, bool oneShot = false ) final;
	/**
	 * @brief 指定事件响应函数，
	 * @param fun ，事件响应的函数对象，使用std::bind完成或者直接指定函数对象
	 */
	virtual void SetOwner( std::function<void(  )> fun ) final;
	/**
	 * @brief 停止计时器
	 */
	virtual emErrCode Stop(  ) final;
	/**
	 * @brief 判断计时器是否正在运行
	 */
	virtual bool IsRuning(  ) final;
	virtual void setTick(long t) final{ __m_tick = t ;}
	/**
	 * @brief 复位计时，让计时器从零开始计时
	 */
	virtual emErrCode Reset() final;
};
