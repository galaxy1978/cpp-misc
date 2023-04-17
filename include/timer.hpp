/**
 * @brief timer定义
 * @version 2.0
 * @author 宋炜
 * @date 2017-2-7 ~ 2021-11-2
 */
#pragma once

#if defined __TIMER_OLD__
#    include "timer_old.hpp"
#else
#include <functional>
#include <memory>

#include "details/itimer.hpp"
namespace wheels
{
class CTimer : public iTimer
{
public:

private:

private:
	std::unique_ptr< iTimer >   __pt_imp;
public:
	CTimer();
	CTimer( long ms , bool oneShot = false );
	CTimer( std::function< void () > fun , long ms , bool oneShot = false );

	~CTimer();

	/**
	 * @brief 启动或者重启计时器。
	 * @param ms ， 计时时长，单位ms，当ms小于0的时候，使用就计时时长启动计时器
	 * @param oneShot ， 是否单次计时，true单次计时，否则连续计时
	 */
	virtual emErrCode Start( long ms = -1, bool oneShot = false) final;
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

	virtual emErrCode Reset() final;

	void setTick( long tick ){ if( __pt_imp ) { __pt_imp->setTick(tick);} }
};
}
#endif
