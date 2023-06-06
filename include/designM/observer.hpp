#include <vector>
#include <functional>
#include <tuple>

#include "designM/variant.hpp"

namespace wheels
{
	namespace dm
	{
		/**
		 * @brief 观察数据变化的模块接口
		 */
		class observer {
		public:
			virtual void update( const std::vector< wheels::variant> & data ) = 0;
		};
		/**
		 * @brief 数据改变的模块
		 */
		class subject {
		protected:
			/**
			 * @brief 抽取tuple数据内容构造参数表的vector
			 */
			template< int N >
			struct __FOR{
				static void extract( std::vector< wheels::variant >& param , const std::tuple<>& t ){
					param[ N ] = wheels::variant::make( std::get< N >( t ) );
					__FOR< N - 1 >::extract( param , t );
				}
			};

			template<>
			struct __FOR<0>{
				static void extract( std::vector< wheels::variant > param , const std::tuple<>& t ){
					param[ 0 ] = wheels::variant::make( std::get< 0 >( t ) );
				}
			};
		public:
			using obsvFunc_t = std::function< void ( const std::vector<wheels::variant>& ) >;
		public:
			subject(){}
			virtual ~subject(){}
			/**
			 * @brief 添加观察者对象
			 */
			template< typename obsvType >
			void addObserver( obsvType* obsv ) {
				static_assert( std::is_base_of< observer , observer> :: value , "" );
				__m_observers.push_back( obsv );
			}

			/**
			 * @brief 添加观察者函数
			 */
			void addObserver( obsvFunc_t func ) {
				static_assert( std::is_base_of< observer , observer> :: value , "" );
				__m_func_obsv.push_back( func );
			}
			/**
			 * @brief 移除观察者对象
			 */
			void removeObserver( observer* obsv ) {
				auto it = std::find(__m_observers.begin(), __m_observers.end(), obsv );
				if (it != observers.end()) {
					__m_observers.erase(it);
				}
			}

			/**
			 * @brief 执行通知操作
			 */
			template< typename ...Args >
			void notifyObservers( Args&... args) {
				// 构造variant数组
				std::tuple<> t = std::make_tuple(args...);
				std::vector< wheels::variant >   param( sizeof...( args ) );
				// 从tuple抽取参数构造成 std::vector< wheels::variant >
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

