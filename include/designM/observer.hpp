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
			// 内部使用
			virtual bool needRelease(){ return false; }
		};
		/**
		 * @brief 数据改变的模块
		 */
		class subject {
		public:
			using obsvFunc_t = std::function< void ( const std::vector<wheels::variant>& ) >;
		protected:
			/**
			 * @brief 抽取tuple数据内容构造参数表的vector
			 */
			template< int N >
			struct FOR__{
				static void extract( std::vector< wheels::variant >& param , const std::tuple<>& t ){
					param[ N ] = wheels::variant::make( std::get< N >( t ) );
					FOR__< N - 1 >::extract( param , t );
				}
			};

			template<>
			struct FOR__<0>{
				static void extract( std::vector< wheels::variant > param , const std::tuple<>& t ){
					param[ 0 ] = wheels::variant::make( std::get< 0 >( t ) );
				}
			};

			class observer__ : public observer__{
			private:
				obsvFunc_t  m_func__;
			public:
				observer__( obsvFunc_t func ):m_func__( func ){}
				virtual ~observer__(){}
				
				virtual void update( const std::vector< wheels::variant > & data ) final{ m_func__( data ); }
				virtual bool needRelease() final{ return true; }
			};
		
		public:
			subject(){}
			virtual ~subject(){}
			/**
			 * @brief 添加观察者对象
			 */
			template<
				typename obsvType ,
				typename midType = typename std::enable_if< std::is_base_of< observer , obsvType > :: value ,obsvType >::type ,
				typename mid2Type = typename std::decay< midType >::type,
				typename realType = std::conditional<
				                std::is_pointer< mid2Type >::value ,
				                typename std::remove_pointer< mid2Type >::type,
				                mid2Type
					>
				>
			void addObserver( realType* obsv ) {
				std::unique_lock< std::mutex > lck( m_mutex__ );
				m_observers__.push_back( obsv );
			}

			/**
			 * @brief 添加观察者函数
			 */
			observer* addObserver( obsvFunc_t func ) {
				observer * ret = nullptr;
				try{
					ret = new observer__(  func );
					{
						std::unique_lock< std::mutex > lck( m_mutex__ );
						m_observers__.push_back( ret );
					}
				}catch( std::bad_alloc& e ){
					std::cout << e.what() <<std::endl;
				}

				return ret;
			}
			/**
			 * @brief 移除观察者对象
			 */
			void removeObserver( observer* obsv ) {
				std::unique_lock< std::mutex > lck_( m_mutex__ );
				if( m_observers__.size() == 0 ) return;
				
				auto it = std::find(m_observers__.begin(), m_observers__.end(), obsv );
				if (it != m_observers__.end()) {
					if( (*it)->needRelease() ){
						delete ( *it );
					}
					m_observers__.erase(it);
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
				FOR__< sizeof...(args) >::extract( param, t );
				std::lock_guard< std::mutex > lck( m_mutex__ );
				for (auto obsv : m_observers__) {
					
					obsv->update( param );
				}
			}

		private:
			std::mutex                 m_mutex__;
			std::vector< observer* >   m_observers__;
		};
	}
}

