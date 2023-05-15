/**
 * @brief 函数类型萃取器
 * @version 1.0
 * @author 宋炜
 */

#pragma once
#include <type_traits>
#include <functional>

namespace wheels
{
	
	namespace __private{
		/// @brief 函数参数类型萃取器，采用递归的方式找到需要参数类型	
		/// @tparam IDX
		/// @tparam pArgs
		/// @treturn type

		template< size_t IDX , typename Args1 , typename ...pArgs >
		struct __arg_type: public __arg_type< IDX - 1 , pArgs... >{
			using type = typename __arg_type< IDX - 1 , pArgs... >::type;
		};
		
		template< typename Arg1 , typename ...pArgs >
		struct __arg_type<0, Arg1 , pArgs...>{
			using type = Arg1;
		};
		

		template<typename T > struct __imp_func_traitor{};
		/// @brief 函数类型萃取器
		/// @tparam retType
		/// @pram Args...
		template< typename retType , typename ...Args >
		struct __imp_func_traitor<std::function< retType ( Args... )> >{
			/// @brief 获取参数类型
			/// @tparam 
			/// @example
			///     using p1type = typename argType< 1 >::type;
			template< size_t N >
			struct argType{
				static_assert( N < sizeof...(Args)  , "" );					
				using type = typename __arg_type< N , Args... >::type;
			};
			
			/// 函数返回值类型
			using ret_type = retType;
		};
	}

	/// 实际实现，需要考虑三种情况。一是普通函数，而是成员函数，三是函数对象
	/// funcTrator< xxx >::ret_type
	/// using arg1_type = typename funcTrator< xxx >::argType::type
	template< typename typeFunctor > struct funcTraitor{};

	/// 普通函数
	template< typename retType , typename ...Args >
	struct funcTraitor< retType(Args...) >: public __private::__imp_func_traitor< std::function< retType (Args...) > > {};

	/// 类成员函数
	template< typename classType , typename funcType >
	struct funcTraitor< funcType classType::*> : public __private::__imp_func_traitor< std::function<funcType> > {};
	// 函数对象
	template< typename retType , typename ...Args >
	struct funcTraitor< std::function< retType(Args...) > > : public __private::__imp_func_traitor< std::function< retType(Args...) >>{};
}
