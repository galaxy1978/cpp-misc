/**
 * @brief 策略模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-11
 */

#pragma once

#include <type_traits>
#include <map>
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
		template< typename keyType , typename Ret , typename ...Args >
		class strategy{
		public:
			using callee_type = std::function<Ret (Args... args) >;
			using iterator = typename std::map< keyType , callee_type >::iterator;
		protected:
			std::map< keyType , callee_type >    __m_strates;
		public:
			strategy(){};
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
				for( auto it = from; it != to; it ++ ){
					it->second( (0,args)... );
				}
			}
			/**
			 * @brief 针对所有的内容执行一次
			 */
			void call_each( Args... args ){
				for( auto it = __m_strates.begin(); it != __m_strates.end(); it ++ ){
					it->second( (0,args)... );
				}
			}
			/**
			 * @brief 执行算法操作
			 * @param key[ I ], 执行调节
			 * @param args[ IO ], 算法参数表
			 */
			Ret call( const keyType& key , Args... args ){
				auto it = __m_strates.find( key );
				if( it != __m_strates.end() ){
					return it->second((0,args)...);
				}

				throw -1;
			}
		};
	}
}
/**
 // 策略内容方式1 , 继承同一接口方式策略项目
 class stra_base{
 virtual void callee( int a ) = 0;
 };

 class stra_a : public stra_base{
 virtual void callee( int a ) final{
 std::cout << a << std::endl;
 }
 };

 class stra_b : public stra_base{
 virtual void callee( int a ) final{
 std::cout << a * 10 << std::endl;
 }
 };
 stra_a   a;
 stra_b   b;
   
 // 初始化
 strategy<int , void , int >  st;
 st.add( 0 , [&]( int param ){
 a.callee( param );
 });
 st.add( 1 , [&]( int param ){
 b.callee( param );
 });

 // 执行
 st.call( 0 , 14 );
 st.call( 1 , 27 );
 --------------------------------------------------------------------------------------------------------
 // 策略方式2 ， 非统一接口方式
 int stra_fun1( int a ){

 }

 int stra_fun2( int b ){

 }

 strategy<int , int , int >  st;
 st.add( 0 , [&]( int param )->int{
 stra_fun1( param );
 });
 st.add( 1 , [&]( int param )->int{
 stra_fun2( param );
 });

 // 执行
 int rst = st.call( 0 , 14 );
 rst = st.call( 1 , 27 );

*/
