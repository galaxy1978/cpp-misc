#include <iostream>
#include <vector>
#include <algorithm>
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
		proteted:
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
				__m_func_obsv.push_back( obsv );
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
/**
 * 这是一个观察者模式的实现，其中subject类是数据改变的模块，observer类是观察者。
 * 观察者模式是一种行为设计模式，它允许对象在状态发生改变时通知一组依赖于此对象的观察者。观察者模式的核心是主题（subject）和观察者（observer）之间的松耦合关系。主题维护一个观察者列表，当主题的状态发生改变时，它会通知所有观察者，让它们能够及时更新自己的状态。
 * 在这个实现中，我们使用了一些C++11的高级特性，例如变长模板参数、递归模板、tuple和可变参数模板。
 * 在subject类中，我们定义了一个模板结构体__FOR，它有一个静态extract函数。在extract函数中，我们首先将tuple中的参数抽取出来，然后将其构造成std::vectorwheels::variant类型的数组。__FOR结构体是一个递归结构体，它的模板参数N表示参数的数量。当N大于0时，我们将第N个参数
 * 构造成wheels::variant类型，然后递归调用__FOR<N-1>::extract函数。当N等于0时，我们只需要将第0个参数构造成wheels::variant类型即可。
 * 这个实现的关键是使用了wheels::variant类型。wheels::variant是一个模板类，它可以存储任意类型的值。在extract函数中，我们将tuple中的参数构造成wheels::variant类型，这样我们就可以将它们存储在std::vectorwheels::variant类型的数组中了。这样做的好处是，我们可以将任
 * 意数量、任意类型的参数传递给notifyObservers函数，而不需要为每种参数类型都定义一个重载版本。
 * 在subject类中，我们还定义了一个addObserver函数，它可以将观察者添加到观察者列表中。removeObserver函数可以从观察者列表中删除观察者。
 * 在subject类中，我们还定义了一个notifyObservers函数，它可以通知所有观察者主题的状态发生了改变。该函数使用可变参数模板来接受任意数量的参数，然后使用tuple将这些参数打包起来。接着，我们使用__FOR结构体将tuple中的参数抽取出来，构造成std::vectorwheels::variant类型的
 * 数组。最后，我们遍历观察者列表，调用它们的update函数，并将参数数组传递给它们。
 * 总体来说，这个实现比较复杂，但使用了一些高级的C++11特性，可以灵活地处理任意数量、任意类型的参数。
*/
