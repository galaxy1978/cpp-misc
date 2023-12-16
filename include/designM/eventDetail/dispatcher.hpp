/**
 * @brief 命令模式命令派发器
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-18
 */

#pragma once

#include <memory>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <queue>
#include <chrono>

#include <mutex>
#if CMD_USE_SINGLETON
#    include "designM/singleton.hpp"
#endif

#include "designM/strategy.hpp"
#include "container/variant.hpp"
#include "event.hpp"
namespace wheels
{
	namespace dm
	{
		template< typename cmdType >
		class dispatcher
#if CMD_USE_SINGLETON
            : public singleton< dispatcher<cmdType> >
#endif
		{
		public:
			using clock = std::chrono::high_resolution_clock;
			using timepoint = std::chrono::time_point< clock >;
			
			using eventQueue = std::queue< variant >;

			using strategy = wheels::dm::strategy< cmdType , std::function< void (const variant& ) > >;
		protected:
			// 消息队列
			eventQueue                   m_store__;
			// 策略模式实现一个函数映射表
			strategy                     m_func_map__;
			// 数据互斥
			std::mutex                   m_mutex__;
			// 线程控制互斥对象
			std::mutex                   m_inform_mutex__;
			// 条件变量
			std::condition_variable      m_notify__;
		public:
			/**
			 * @brief ctor
			 */
			dispatcher(): m_store__(){}
			
			virtual ~dispatcher(){ 	clear(); }
			/**
			 * @brief 发送命令
			 * @tparam dataType 命令参数类型，要求支持拷贝构造，赋值拷贝
			 * @param evt[ I ]，命令
			 * @param data[ I ]，命令参数
			 * @param notify[ I ], 是否立即通知执行。如果不立即通知会在dispath的超时时间结束后
			 *    再开始执行。也可以后续调用update函数执行通知操作
			 */
			template< typename dataType >
			void send( const cmdType& cmd , dataType&& data , bool notify = true ){
				eventData< cmdType >  evt_data( cmd , data );
				wheels::variant param = wheels::variant::make( evt_data );
				{
					std::lock_guard< std::mutex > locker(m_mutex__);
					m_store__.push( param );
				}
				if( notify ){
					m_notify__.notify_one();
				}
			}
			
			template< typename dataType >
			void emit( const cmdType& cmd , dataType&& data , bool notify = true ){
				eventData< cmdType >  evt_data( cmd , data );
				wheels::variant param = wheels::variant::make( evt_data );
				{
					std::lock_guard< std::mutex > locker(m_mutex__);
					m_store__.push( param );
				}
				if( notify ){
					m_notify__.notify_one();
				}
			}

			void update(){  m_notify__.notify_one(); }
			/**
			 * @brief 清理队列，函数映射表
			 */
			void clear(){
				std::lock_guard< std::mutex > lok( m_mutex__ );
				//m_func_map__.clear();
				while( !m_store__.empty() ){
					m_store__.pop();
				}
				// 清理掉所有数据后通知dispatch，结束dispath的等待操作
				m_notify__.notify_all();
			}

			/**
			 * @brief 命令绑定，这个函数用来绑定类成员函数
			 * @tparam classType 类名称
			 * @tparam funcType，函数名称
			 * @param cmd 命令
			 * @param func 函数指针
			 * @param 类对象指针
                         */
			template<typename Func_t >
			bool connect( const cmdType& cmd , Func_t&& func ){
				
				std::lock_guard< std::mutex > lok( m_mutex__ );
				m_func_map__.add( cmd , std::forward<Func_t>( func ) );
				return true;
			}
			/**
			 * @brief 命令执行调度，执行命令队列的头一个
			 * @param ovtime[ I ], 在命令队列中没有命令的时候，函数的等待超时时间。
			 * @note 通过超时时间降低系统负载
			 */
			void dispatch( long ovtime ){
				if( m_store__.size() > 0 ){
					variant item;
				    {
						std::lock_guard< std::mutex > lock(m_mutex__);
						item = m_store__.front();
						m_store__.pop();
					}
					auto evt_data = item.get< eventData<cmdType> >();
                    m_func_map__.call( evt_data.m_id__ , evt_data.m_data__ );
				}else{  // 如果消息队列中没有数据，则阻塞线程等待新的消息
					// 这里要注意如果在销毁的时候或者清理数据内容的时候需要防止
					// dispatch无法退出的情况
					std::unique_lock<std::mutex> lok( m_inform_mutex__ );
					timepoint tp{ std::chrono::milliseconds( ovtime ) };
					m_notify__.wait_until( lok , tp );
				}
			}
		};
	}
}
