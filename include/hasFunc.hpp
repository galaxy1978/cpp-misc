/**
 * @brief 检查指定函数名字是否存在
 * @version 1.0
 * @author 宋炜
 * @date 2023-1-17
*/

#pragma once

#include <type_traits>

#define DECLARE_FUNC_CHK( funcName ) \
template <typename classType > \
struct has_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName())> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC( classType , funcName ) has_##funcName< classType >::value 

// 一个参数的实现
#define DECLARE_FUNC_CHK_1( funcName , paramType1 ) \
template <typename classType > \
struct has1_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName( *(paramType1*)nullptr))> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC_1( classType , funcName ) has1_##funcName< classType >::value 

// 2个参数的实现
#define DECLARE_FUNC_CHK_2( funcName , paramType1 , paramType2 ) \
template <typename classType > \
struct has2_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName( *(paramType1*)nullptr，*(paramType2*)nullptr))> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC_2( classType , funcName ) has2_##funcName< classType >::value 


// 3个参数的实现
#define DECLARE_FUNC_CHK_3( funcName , paramType1 , paramType2 , paramType3 ) \
template <typename classType > \
struct has3_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName( *(paramType1*)nullptr，*(paramType2*)nullptr，*(paramType3*)nullptr))> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC_3( classType , funcName ) has3_##funcName< classType >::value 


// 4个参数的实现
#define DECLARE_FUNC_CHK_4( funcName , paramType1 , paramType2 , paramType3 , paramType4) \
template <typename classType > \
struct has4_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName(  \ 
    	*(paramType1*)nullptr, \ 
    	*(paramType2*)nullptr, \ 
    	*(paramType3*)nullptr), \ 
    	*(paramType4*)nullptr))> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC_4( classType , funcName ) has4_##funcName< classType >::value 


// 4个参数的实现
#define DECLARE_FUNC_CHK_5( funcName , paramType1 , paramType2 , paramType3 , paramType4 , paramType4) \
template <typename classType > \
struct has5_ ## funcName{ \
    template <typename T, typename = decltype(std::declval<T>().funcName( \ 
    	*(paramType1*)nullptr, \ 
    	*(paramType2*)nullptr, \ 
    	*(paramType3*)nullptr), \ 
    	*(paramType4*)nullptr), \ 
    	*(paramType5*)nullptr))> \
    static std::true_type test(int); \
	\
    template <typename T>  \
    static std::false_type test(...);  \
	\
    static constexpr bool value = decltype(test< classType >(0) )::value; \
}

#define HAS_FUNC_5( classType , funcName ) has5_##funcName< classType >::value 
