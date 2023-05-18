/**
 * @brief 命令模式汇总模块.
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-19
 */

#pragma once

#include <string>

#include "designM/eventDetail/event.hpp"
#include "designM/eventDetail/dispatcher.hpp"
#include "designM/eventDetail/mainloop.hpp"

namespace wheels
{
	namespace dm
	{
		using commandUI = mainloop< uint32_t >;
		using commandStr = mainLoop< std::string >;
	}
}
