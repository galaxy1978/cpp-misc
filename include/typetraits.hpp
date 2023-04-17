/**
 * @brief 类型元函数
 * @version 1.1
 * @author 宋炜
 * @date 2020-8-7 ~ 2022-2-21
 */


/// 2022-2-21 宋炜 ADDED 添加bool类型判断
/// 2022-3-11 宋炜 ADDED 添加获取数组长度接口
#ifndef __TYPE_TRAITS_HPP__
#define __TYPE_TRAITS_HPP__

template< typename T , typename _Tp = T * , typename _Tr = T& > 
struct type_traits
{
        typedef T   value_type;
        typedef _Tp pointer;
        typedef _Tr refrence;
};

/**
 * @brief 判断类型是否是bool类型元函数。
 * @tparam T 数据类型
 * @return 如果数据数据类型是bool返回true，否则返回false
 */
template< typename T >
struct is_bool
{
	static const bool value = false;
};

template<>
struct is_bool<bool>
{
	static const bool value = true;
};

template< typename T , size_t N >
size_t arrayLen( T a[N] )
{
	return N;
}

#endif
