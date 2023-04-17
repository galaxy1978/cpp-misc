#include "details/itimer.hpp"
std::string iTimer :: errMsg( emErrCode e )
{
	std::string ret;
	switch( e ){
	case ERR_CREATE_TIMER:
		ret ="创建计时器错误";
		break;
	case ERR_MISSING_INFO_FUN:
		ret = "缺少了事件通知函数";
		break;
	case ERR_START_TIMER:
		ret = "启动计时器错误";
		break;
	case ERR_STOP_TIMER:
		ret ="停止计时器错误";
		break;
	case ERR_TICK:
		ret = "计时时间配置错误";
		break;
	default:break;
	}

	return ret;
}
