/**
 * @brief jemalloc内存管理。
 * @version 1.0 
 * @author 宋炜
 * @date 2022-12-26
*/

#pragma once

/// 要使用jemalloc替代malloc必须在编译前定义USE_JEMALLOC
#if USE_JEMALLOC == 1

#	define JEMALLOC_MANGLE
#	include <jemalloc/jemalloc.h>

#else
#	warning "walloc.hpp is included without enable jemalloc, pls define USE_JEMALLOC enable it."
#endif
