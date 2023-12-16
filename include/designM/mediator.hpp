/**
 * @brief 中介者模式
 * @version 1.1
 * @author 宋炜
 */

#pragma once

#include <type_traits>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <vector>

#if !defined( MEDIATOR_USE_TUPLE )
#    define MEDIATOR_USE_TUPLE (1)
#endif

#if MEDIATOR_USE_TUPLE == 1
#    include <tuple>
#    include <thread>
#    include <condition_variable>
#    include <functional>
#    include <mutex>
#    include <queue>
#    include <atomic>
#endif

namespace private__
{
	struct colleagueBase{};
}

namespace wheels {
namespace dm {

// 在不使用异步操作的时候使用直接传递消息参数的方式。这个时候发送和接收处理直接使用参数的方式进行
// 传递。当使用异步方式进行中介处理的时候接收处理的参数实际被封包成std::tuple。
// 因为异步传递的时候数据内容需要进行排队，这要求能够有容器临时存储数据内容。考虑到模块的通用性和
// 对消息结构的复杂性，所以使用std::tuple的方式。
#if MEDIATOR_USE_TUPLE
	template< typename MEDIATOR , typename ...PARAMS >
	struct colleagueItfc : public private__::colleagueBase
	{
	public:
		using mediator_t = typename std::remove_pointer< typename std::decay<MEDIATOR>::type >::type;
		using mediaItfc_t = typename mediator_t::itfc_t;
		using tuple_t = std::tuple< PARAMS... >;
	protected:
		std::shared_ptr< mediator_t >    pt_mediator__;
	public:
		colleagueItfc( std::shared_ptr< mediator_t > m ):pt_mediator__(m){}
		virtual ~colleagueItfc(){}
		// 发送消息接口，消息内容和数量是可以根据实际使用情况进行变化的
		template< typename ...Params>
		void send( Params&& ...args){
			pt_mediator__->dispatch( (mediaItfc_t*)this , std::forward<Params>(args)...);
		}
		template< typename ...Params>
		void sendTo( std::shared_ptr< mediaItfc_t > to , Params&& ...args ){
			pt_mediator__->dispatchTo( to , std::forward<Params>(args)...);
		};
		template< typename ...Params>
		void sendToDests( std::vector< std::shared_ptr< mediaItfc_t > > dests , Params&& ...args ){
			pt_mediator__->dispatchToDests( dests , std::forward<Params>(args)...);
		};
		// 接收处理接口
		virtual void recv( const std::tuple<PARAMS...>& tpl  ) = 0;
	};
#else
	template< typename MEDIATOR , typename ...PARAMS >
	struct colleagueItfc : public private__::colleagueBase
	{
	public:
		using mediator_t = typename std::remove_pointer< typename std::decay<MEDIATOR>::type >::type;
		using mediaItfc_t = typename mediator_t::itfc_t;
	protected:
		std::shared_ptr< mediator_t >    pt_mediator__;
	public:
		colleagueItfc( std::shared_ptr< mediator_t > m ):pt_mediator__(m){}
		virtual ~colleagueItfc(){}

		template< typename ...Params>
		void send( Params&& ...args){
			pt_mediator__->dispatch( (mediaItfc_t*)this , std::forward<Params>(args)...);
		}

		template< typename ...Params>
		void sendTo( std::shared_ptr< mediaItfc_t > to , Params&& ...args ){
			pt_mediator__->dispatchTo( to , std::forward<Params>(args)...);
		};

		template< typename ...Params>
		void sendToDests( std::vector< std::shared_ptr< mediaItfc_t > > dests , Params&& ...args ){
			pt_mediator__->dispatchToDests( dests , std::forward<Params>(args)...);
		};

		virtual void recv( const PARAMS& ...args ) = 0;
	};
#endif

	template< typename ITFC_TYPE 
#if MEDIATOR_USE_TUPLE
		  , typename ...Params
#endif		
		  >
	class mediator
	{
	public:
		using itfc_t = typename std::remove_pointer< typename std::decay<ITFC_TYPE>::type >::type;
		using data_t = std::unordered_set< std::shared_ptr< itfc_t > >;
#if MEDIATOR_USE_TUPLE
		using tuple_t = typename std::tuple<Params...>;
		/// 定义消息结构，消息结构中包含了接收者和消息数据内容
		struct stMsgs{
			std::shared_ptr< itfc_t >   pt_dst__;   // 接收者
			std::shared_ptr< tuple_t >  m_data__;   // 消息内容
		};
#endif
	protected:
		data_t                    m_colleague__;    // 同事表

#if  MEDIATOR_USE_TUPLE 
		std::queue< stMsgs >      m_msgs__;         // 消息队列
		std::condition_variable   m_cnd_var__;      // 采用异步处理需要考虑线程安全性
		std::atomic< bool >       m_is_running__;   // 线程运行标记
		std::mutex                m_mutex__;
	protected:
		/**
		 * @brief 后台处理线程。
		 */
		void backend__(){
			/// m_is_running__判断后台线程是否需要继续运行，逐个消息队列中读取数据消息内容，并调用目标的recv函数进行处理。如果消息表中没有
			/// 内容则等候消息。
			while( m_is_running__.load() ){
				std::unique_lock< std::mutex > lck( m_mutex__ );

				if( !m_msgs__.empty() ){
					auto item = m_msgs__.front();
					item.pt_dst__->recv( *item.m_data__ );
					m_msgs__.pop();
				}else{
					m_cnd_var__.wait( lck );
				}
			}
		}
#endif
	public:
		mediator():m_is_running__( false ){}
		virtual ~mediator(){}
		/**
		 * @brief 增加参与者
		 * @param colleague[ I ]， 参与者对象指针
		 */
		void add( std::shared_ptr< itfc_t > colleague) {
#if MEDIATOR_USE_TUPLE 
			std::unique_lock< std::mutex >  lck( m_mutex__ );
#endif
			m_colleague__.insert(colleague);
		}
		/**
		 * @brief 删除参与者
		 * @param colleague[ I ]， 参与者对象指针
		 */
		void erase( std::shared_ptr< itfc_t > colleague) {
#if MEDIATOR_USE_TUPLE 
			std::unique_lock< std::mutex >  lck( m_mutex__ );
#endif
			auto it = std::find( m_colleague__.begin() , m_colleague__.end() , colleague );
			if( it != m_colleague__.end() ){
				m_colleague__.erase( it );
			}
		}

#if  MEDIATOR_USE_TUPLE 
		/**
		 * @brief 启动线程或者关闭线程
		 * @param sw[ I ]
		 * @return 成功操作返回true
		 */
		bool run( bool sw ){
			if( m_is_running__.load() == sw ) return false;

			m_is_running__ = sw;
			if( sw ){
				std::thread thd( std::bind( &mediator::backend__ ,  this ) );
				thd.detach();
			}else{ // 结束后台线程的时候先把运行标志设置为false，然后清理掉所有已经在排队的
				// 消息；然后通知再等待的线程结束等待。
				std::unique_lock< std::mutex > lck( m_mutex__ );
				while( !m_msgs__.empty() ){
					m_msgs__.pop();
				}

				m_cnd_var__.notify_one();
			}

			return true;
		}

		/**
		 * @brief 将消息数据添加到消息队列中，并触发后台线程。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param sender[ I ]， 发送方
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatch(itfc_t * sender, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			
			auto pt_tpl = std::make_shared< std::tuple< typename std::decay<PARAMS>::type...> >( std::forward<PARAMS>(args)... );
			std::unique_lock< std::mutex >  lck( m_mutex__ );

			for (auto colleague : m_colleague__) {
				if (colleague.get() != sender) {
					stMsgs msg = { colleague , pt_tpl };
					m_msgs__.push( msg );
				}
			}

			m_cnd_var__.notify_one();
		}
		/**
		 * @brief 将消息数据添加到消息队列中，并触发后台线程。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param to[ I ]， 接收者
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatchTo(std::shared_ptr< itfc_t > to, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			auto it = std::find_if( m_colleague__.begin() , m_colleague__.end(), [=]( std::shared_ptr<itfc_t> ptr )->bool{
				if( ptr.get() == to.get() ) return true;
				return false;
			} );
			
			if( it != m_colleague__.end() ){
				auto pt_tpl = std::make_shared< std::tuple< typename std::decay<PARAMS>::type...> >( std::forward<PARAMS>(args)... );
				stMsgs msg = { to , pt_tpl };
				m_msgs__.push( msg );
			}

			m_cnd_var__.notify_one();
		}
		/**
		 * @brief 将消息数据添加到消息队列中，并触发后台线程。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param dests[ I ]， 接收者数组
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatchToDests( std::vector< std::shared_ptr< itfc_t > > dests, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			// 1  构造std::tuple对象，tuple的类型需要使用PARAMS进行推导，但是由于PARAMS是万能应用可能导致推导内容不能满足实际传递过来的内容完全匹配
			// 来构造tuple，所以使用std::decay移除修饰内容后再构造shared_ptr。
			// 2  使用shared_ptr方便消息传递过程里面重复的对象构造和内存拷贝。
			auto pt_tpl = std::make_shared< std::tuple<typename std::decay<PARAMS>::type...> >( std::forward<PARAMS>(args)... );

			for( auto to : dests ){
				auto it = std::find( m_colleague__.begin() , m_colleague__.end(), to );
				if( it != m_colleague__.end() ){
					stMsgs msg = { to , pt_tpl };
					m_msgs__.push( msg );
				}
			}

			m_cnd_var__.notify_one();
		}
#else
		// 在不适用std::tuple的情况下没有做异步处理，定义这个方法以实现代码层的兼容。
		bool run( bool ){ return true; }
		/**
		 * @brief 转发消息。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param dests[ I ]， 接收者数组
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatch( itfc_t * sender, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			for (auto colleague : m_colleague__) {
				if (colleague.get() != sender) {
					colleague->recv( std::forward<PARAMS>( args )...);
				}
			}
		}
		/**
		 * @brief 转发消息。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param dests[ I ]， 接收者数组
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatchTo(std::shared_ptr< itfc_t > to, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			auto it = std::find( m_colleague__.begin() , m_colleague__.end(), to );
			if( it != m_colleague__.end() ){
				to->recv( std::forward<PARAMS>( args )...);
			}
		}

		/**
		 * @brief 转发消息。通常情况下dispatch函数在colleagueItfc的子类
		 *    中的send函数中调用
		 * @param dests[ I ]， 接收者数组
		 * @param args[ I ]， 消息内容
		 */
		template< typename ...PARAMS >
		void dispatchToDests( std::vector< std::shared_ptr< itfc_t > > dests, PARAMS&& ...args ) {
			static_assert( std::is_base_of<private__::colleagueBase , itfc_t >::value , "" );
			for( auto to : dests ){
				auto it = std::find( m_colleague__.begin() , m_colleague__.end(), to );
				if( it != m_colleague__.end() ){
					to->recv( std::forward<PARAMS>( args )...);
				}
			}
		}
#endif
	};
}}
