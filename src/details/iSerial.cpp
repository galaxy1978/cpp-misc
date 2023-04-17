#include "details/iSerial.hpp"
using namespace wheels;
std::string
iSerial::errMsg( emErrCode& e )
{
	std::string ret;

	switch( e ){
	case ERR_ALLOC_MEM:
		ret = "分配内存失败";
		break;
	case ERR_DEV_OCCUPIED:
		ret = "串口被占用.";
		break;
	case ERR_DEV_NOT_EXIST:
		ret = "串口不存在.";
		break;
	case ERR_SEND_DATA_EMPTY:
		ret = "发送数据空";
		break;
	case ERR_SET_PARAMS:
		ret = "设置串口参数失败";
		break;
	case ERR_BAD_DEV:
		ret = "串口号错误";
		break;
	case ERR_BAD_FD:
		ret = "系统错误文件描述符";
		break;
	case ERR_OPEN_SERIAL:
		ret = "打开串口错误";
		break;
	case ERR_CLOSE_UART:
		ret = "关闭串口出错";
		break;
	case ERR_SEND_FAIL:
		ret = "发送数据错误";
		break;
	case ERR_ALREADY_OPENED:
		ret = "串口已经被打开或者被其他程序占用";
		break;
	case SET_COM_FAIL:
		ret = "配置串口失败";
		break;
	case ERR_DATA_NULL:
		ret = "数据指针为空";
		break;
	case ERR_STOP_READ_THD:
		ret = "结束数据读取线程失败";
	break;
	case ERR_READ_DATA:
		ret = "读取数据操作错误";
	default:break;
	}
	return ret;
}
