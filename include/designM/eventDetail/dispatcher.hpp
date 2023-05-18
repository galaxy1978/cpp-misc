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

#if CMD_USE_SINGLETON
#    include "designM/singleton.hpp
#endif

#include "designM/strategy.hpp"
#include "event.hpp"
namespace wheels
{
	namespace dm
	{
		template< typename cmdType >
		class dispatcher
#if CMD_USE_SINGLETON
			: singleton< dispatcher >
#endif
		{
		public:
			using evt_func_type = void(const wheels::variant& );
			using evt_func_type_p = evt_func_type_p *;
			
			using clock = std::chrono::high_resolution_clock;
			using timepoint = std::chrono::time_point< clock >;
			
			using eventQueue = std::queue< wheels::variant >;

			using strategy = wheels::dm::strategy< cmdType , evt_func_type >;
		protected:
			// 消息队列
			std::unique_ptr< eventQueue >    __pt_store;
			// 策略模式实现一个函数映射表
			strategy                     __m_func_map;
			// 数据互斥
			std::mutex                   __m_mutex;
			// 线程控制互斥对象
			std::mutex                   __m_inform_mutex;
			// 条件变量
			std::conditional_variable    __m_notify;
		public:
			/**
			 * @brief ctor
			 */
			dispatcher(): __pt_store( new eventQueue ){}
			
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
			void send( const cmdType& cmd , const dataType& data , bool notify = true ){
				eventData< cmdType >  evt_data( cmd , data );
				wheels::variant param = wheels::variant::make( evt_data );
				{
					std::lock_guard< std::mutex > locker;
					__pt_store->push( param );
				}
				if( notify ){
					__m_notify.notify_all();
				}
			}

			void update(){  __m_notify.notify_all(); }
			/**
			 * @brief 清理队列，函数映射表
			 */
			void clear(){
				std::lock_guard< std::mutex > lok( __m_mutex );
				__m_func_map.clear();
				__pt_store.erase( __pt_store->begin() , __pt_store->end() )h;
				// 清理掉所有数据后通知dispatch，结束dispath的等待操作
				__m_notify.notify_all();
			}
			/**
			 * @brief 连接命令和执行部分
			 * @param func[ I ] 函数对象
			 * @return 返回操作结果
			 */
			bool connect( const cmdType& cmd , std::function< evt_func_type> func ){
			        std::lock_guard< std::mutex > lok( __m_mutex );
				__m_func_map.add( cmd , func );
				return true;
			}
			/**
			 * @brief 连接命令和函数指针指向的处理函数
			 * @param func[ I ]， 函数指针
			 */
			bool connect( const cmdType& cmd , evt_func_type_p func ){
				std::lock_guard< std::mutex > lok( __m_mutex );
				__m_func_map.add( cmd , std::bind( func , std::placeholders::_1 ) );
				return true;
			}
			/**
                         * @brief 命令绑定，这个函数用来绑定类成员函数
			 * @tparam classType 类名称
			 * @tparam funcType，函数名称
			 * @param cmd 命令
			 * @param func 函数指针
			 * @param 类对象指针
                         */
			template<typename classType , typename funcType >
			bool connect( const cmdType& cmd , funcType classType::* func , classType * obj ){
				std::lock_guard< std::mutex > lok( __m_mutex );
				__m_func_map.add( cmd , std::bind( func , obj ) );
				return true;
			}
			/**
			 * @brief 命令执行调度，执行命令队列的头一个
			 * @param ovtime[ I ], 在命令队列中没有命令的时候，函数的等待超时时间。
			 * @note 通过超时时间降低系统负载
			 */
			void dispatch( long ovtime ){
				if( __pt_store->size() > 0 ){
					eventQueue<cmdType> item;
				        {
						std::lock_guard< std::mutex > lock;
						item = __pt_store->front();
						__pt_store->pop();
					}
					auto evt_data = item.get< eventData >();
					__m_func_map.call( evt_data.__m_id , evt_data.__m_data );
				}else{  // 如果消息队列中没有数据，则阻塞线程等待新的消息
					// 这里要注意如果在销毁的时候或者清理数据内容的时候需要防止
					// dispatch无法退出的情况
					std::unique_lock<std::mutex> lok( __m_inform_mutex );
					timepoint tp( std::chrono::milliseconds( ovtime ) );
					__m_notify.wait_until( lok , tp );
				}
			}
		};

	}
}
