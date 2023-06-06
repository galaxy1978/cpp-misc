#include <vector>
#include <functional>
#include <tuple>

#include "designM/variant.hpp"

namespace wheels
{
	namespace dm
	{
		/**
		 * @brief 观察数据变化的模块接口。
		 */
		class observer {
		public:
			virtual void update( const std::vector< wheels::variant> & data ) = 0;
		};
		/**
		 * @brief 数据改变的模块。这个是通知发出的模块，实际使用的时候可以采用组合的方式也可以采用继承的方式
		 */
		class subject {
		protected:
			/**
			 * @brief 抽取tuple数据内容构造参数表的vector
			 */
			template< int N > struct __FOR{
				static void extract( std::vector< wheels::variant >& param , const std::tuple<>& t ){
					param[ N ] = wheels::variant::make( std::get< N >( t ) );
					__FOR< N - 1 >::extract( param , t );
				}
			};
			template<> struct __FOR<0>{
				static void extract( std::vector< wheels::variant > param , const std::tuple<>& t ){
					param[ 0 ] = wheels::variant::make( std::get< 0 >( t ) );
				}
			};
		public:
			// 这个是为了std::function方式的接口所引入的定义
			using obsvFunc_t = std::function< void ( const std::vector<wheels::variant>& ) >;
		public:
			subject(){}
			virtual ~subject(){}
			/**
			 * @brief 添加观察者对象。
			 */
			template< typename obsvType , typename midType = typename std::decay< obsvType >::type ,
										  typename realType = std::coniditional< std::is_pointer< midType >::value , 
																	typename std::remove_pointer< midType >::type,
																	midType >
													>
			void addObserver( realType* obsv ) {
				static_assert( std::is_base_of< observer , realType> :: value , "" );
				__m_observers.push_back( obsv );
			}

			/**
			 * @brief 添加观察者函数。这个是添加函数对象类型的观察者
			 */
			void addObserver( obsvFunc_t func ) {
				__m_func_obsv.push_back( func );
			}
			/**
			 * @brief 移除观察者对象。
			 */
			void removeObserver( observer* obsv ) {
				auto it = std::find(__m_observers.begin(), __m_observers.end(), obsv );
				if (it != observers.end()) {
					__m_observers.erase(it);
				}
			}

			template< typename ...Args >
			void notifyObservers( Args&... args) {

				std::tuple<> t = std::make_tuple(args...);

				std::vector< wheels::variant >   param( sizeof...( args ) );

				__FOR< sizeof...(args) >::extract( param, t );

				for (auto obsv : __m_observers) {
					obsv->update( param );
				}
				
				for (auto obsv : __m_func_obsv) {
					obsv( param );
				}
			}

		private:
			std::vector< observer* > __m_observers;
			std::vector< obsvFunc_t >  __m_func_obsv;
		};
	}
}
