/**
 * @brief 策略模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-11
 */

#pragma once

#include <type_traits>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <cstddef>

namespace wheels
{
	/**
	 * @brief 策略方式定义，
	 * @tparam keyType , 进行判断条件类型，条件在运行过程中必须是唯一的保证能够找到实际最终的策略
	 * @tparam Ret , 策略算法的返回值类型
	 * @tparam Args , 策略算法的参数类型
	 */
	namespace dm
	{
		template< typename keyType , typename funcType > struct strategy{};
		
		template< typename keyType , typename Ret , typename ...Args >
		class strategy< keyType , std::function< Ret (Args...) > >{
		public:
			using callee_type = std::function< Ret (Args...) >;
			using iterator    = typename std::unordered_map< keyType , callee_type >::iterator;
		protected:
			std::unordered_map< keyType , callee_type >    __m_strates;
		public:
			strategy(){}
			virtual ~strategy(){}
			/**
			 * @brief 添加算法操作
			 * @param key[ I ] , 条件判断具体条件，必须是唯一的
			 * @param callFn , 算法函数，这是一个函数对象，可以使用lambda可以采用bind方式绑定
			 */
			bool add( const keyType& key , callee_type callFn ){
				auto it = __m_strates.find( key );
				if( it == __m_strates.end() ){
					__m_strates.insert( std::make_pair( key , callFn ) );
					return true;
				}

				return false;
			}
			/**
			 * @brief 移除算法操作
			 */
			bool erase( const keyType& key ){
				auto it = __m_strates.find( key );
				if( it != __m_strates.end() ){
					__m_strates.erase( it );
					return true;
				}

				return false;
			}
			/**
			 * @brief 清理记录
			 */
			void clear(){ __m_strates.erase( __m_strates.begin() , __m_strates.end() ); }
			/**
			 * @brief 读取记录数量
			 */
			size_t count(){ return __m_strates.size(); }

			void call_each( iterator from , iterator to , Args... args ){
				for( auto it = from; it != to; ++it ){
					it->second( (0,args)... );
				}
			}
			/**
			 * @brief 针对所有的内容执行一次
			 */
			template< typename ...Params >
			void call_each( Params&&... args ){
				for( auto it = __m_strates.begin(); it != __m_strates.end(); ++it ){
					it->second( std::forward<Args>(args)... );
				}
			}
			/**
			 * @brief 执行算法操作
			 * @param key[ I ], 执行调节
			 * @param args[ IO ], 算法参数表
			 * @exception 找不到合适的策略会抛出-1
			 */
			template< typename ...Params >
			Ret call( const keyType& key , Params&&... args ){
				auto it = __m_strates.find( key );
				if( it != __m_strates.end() ){
					return it->second(std::forward<Args>(args)...);
				}
				std::cout << "Can not find strategy key" << std::endl;
				throw std::runtime_error( "branch is not exist" );
			}
		};
	}
}