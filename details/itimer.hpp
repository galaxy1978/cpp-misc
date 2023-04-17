/**
 * @brief 定时器定义接口
 * @version 2.0
 * @author 宋炜
 * @date 2017-2-7 ~ 2021-11-1
 */
#pragma once
#include <atomic>
#include <functional>
#include <string>
/// 2018-03-05 ADD 宋炜 添加计时器复位功能； 使用atomic作为数据操作互斥
/// 2021-11-01 RECODE 宋炜 原来采用sleep方式的计时方式不准确，并且对CPU的占用比较大，重新调整结构
///             针对不同的操作系统进行独立编写相关代码，以排除上述问题。
struct iTimer
{
	enum emErrCode
	{
		ERR_CREATE_TIMER = -1000,      // 创建计时器错误
		ERR_MISSING_INFO_FUN,          // 缺少了事件通知函数
		ERR_START_TIMER,               // 启动计时器错误
		ERR_STOP_TIMER,                // 停止计时器错误
		ERR_TICK,                      // 计时时间配置错误
		OK = 0 
	};
	/**
	 * @brief 启动或者重启计时器。
	 * @param ms ， 计时时长，单位ms，当ms小于0的时候，使用就计时时长启动计时器
	 * @param oneShot ， 是否单次计时，true单次计时，否则连续计时
	 */
	virtual emErrCode Start( long ms = -1, bool oneShot = false) = 0;
	/**
	 * @brief 指定事件响应函数，
	 * @param fun ，事件响应的函数对象，使用std::bind完成或者直接指定函数对象
	 */
	virtual void SetOwner( std::function<void(  )> fun ) = 0;
	/**
	 * @brief 停止计时器
	 */
	virtual emErrCode Stop(  ) = 0;
	/**
	 * @brief 判断计时器是否正在运行
	 */
	virtual bool IsRuning(  ) = 0;
	/**
	 * @brief 复位计时，让计时器从零开始计时
	 */
	virtual emErrCode Reset() = 0;

	virtual void setTick( long t ) = 0;
	
	static std::string errMsg( emErrCode e );
};
