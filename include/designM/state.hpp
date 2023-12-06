/**
 * @brief 状态模式
 * @version 1.0
 * @author 宋炜
 */

#pragma once

#include <type_traits>
#include <functional>
#include <memory>
#include <map>
#include <vector>

#include "designM/mediator.hpp"
// 这是一个状态模式的实现，利用中介者模式和外部进行异步互动
// 也就是利用上次视屏的中介模式的异步消息传递方式，将状态变化传递出去
// 下来看具体实现
namespace private__
{
	// 这是状态变化事件类型定义
	enum class emEvent
	{
		EVT_ENT,	// 进入状态触发事件
		EVT_LEAVE,  // 离开状态触发事件
		EVT_READY,  // 模块就绪事件
		EVT_END     // 运行到结束节点的时间
	};
	// 状态节点定义，模板参数是状态标记类型
	// 运行自定义自己需要的类型，比如分析网络协议可以使用uint8_t, 比如定义路灯状态可以使用自己的枚举
	template< typename STATE_TYPE >
	struct stStat
	{
		STATE_TYPE   m_state;
		stStat( const STATE_TYPE& s ):m_state( s ){}
	};
	// 状态转换关系节点定义
	
	template< typename STATE_TYPE , typename CONDITON_DATA_TYPE >
	struct stArc
	{
		STATE_TYPE  m_from;      // 来源状态
		STATE_TYPE  m_to;        // 目标状态
		// 转换条件处理函数，如果转换条件满足则返回true
		std::function< bool ( const CONDITON_DATA_TYPE& ) >   m_condition;

		stArc( const STATE_TYPE& from , const STATE_TYPE& to ,std::function< bool ( const CONDITON_DATA_TYPE& ) > fun ):
			m_from( from ),
			m_to( to ),
			m_condition( fun ) {}
	};
}

namespace wheels{
	namespace dm{
// 这里是状态机的定义
// 第一个参数是状态类型，第二个是转换条件数据类型
		template< typename STATE_TYPE , typename CONDITION_DATA_TYPE >
		class state
		{
		public:
			// 检查转换条件是否符合要求，这里要求满足可以进行数学结算的类型
			// 或者是有默认构造的类
			static_assert( std::is_arithmetic<CONDITION_DATA_TYPE>::value ||
				       ( std::is_class<CONDITION_DATA_TYPE>::value && std::is_default_constructible<CONDITION_DATA_TYPE>::value ) , "" );

			using evtType_t = private__::emEvent;
			using stateData_t = typename std::remove_pointer< typename std::decay< STATE_TYPE >::type >::type;
			// 针对状态数据进行别名处理
			using state_t = private__::stStat< stateData_t >;
			using arc_t = private__::stArc< stateData_t , CONDITION_DATA_TYPE >;
			// 用map保存节点的转换关系
			// stateData_t 是源节点，作为检索的key
			// std::set< stateData_t >，使用set保存目标节点，因为一个源节点可能有多个目标节点状态
			using arcData_t = std::map< stateData_t , std::vector< arc_t > >;

			// 使用中介者模式传递消息。利用中介者模式，如果不了解这个模块的可以参考一下
			// 以前关于中介者模式的视频
			class colleague;
			using mediator_t = wheels::dm::mediator< colleague , stateData_t , private__::emEvent , CONDITION_DATA_TYPE>;
	
			// 定于消息接口
			class colleague : public colleagueItfc< mediator_t , stateData_t , private__::emEvent , CONDITION_DATA_TYPE >{
			public:
				std::function< void ( stateData_t , private__::emEvent , CONDITION_DATA_TYPE ) >  m_cb__;
			public :
				colleague( std::shared_ptr<mediator_t> m ):
					colleagueItfc< mediator_t , stateData_t , private__::emEvent , CONDITION_DATA_TYPE >(m){}
				virtual ~colleague(){}
				/**
				 * @brief 重载recv函数，并在其中针对tuple解包，调用回调函数通知状态变化
				 * @param tpl[ I ]，
				 */
				// 实现消息接收接口。消息传递使用了std::tuple，这里需要根据具体应用将tuple内容解出来
				virtual void recv( const std::tuple< stateData_t , private__::emEvent , CONDITION_DATA_TYPE >& tpl ) final{
					if( m_cb__ ){
						// 调用实际的处理函数
						m_cb__( std::get<0>( tpl ) , std::get<1>( tpl ) , std::get<2>( tpl ) );
					}
				}
			};

		protected:
			std::map< stateData_t , private__::stStat< stateData_t > >    m_stats__;
			arcData_t                           m_arcs__;

			stateData_t                         m_current__;	
			stateData_t                         m_start__;	
			stateData_t                         m_end__;
			std::atomic<bool>                   m_is_running__;

			std::shared_ptr< mediator_t >       pt_mdt__;
			// 将中介者模式分成生产者和消费者两个实例，生产者处理状态转换并发送事件。消费者处理事件
			// 通过回调函数将消息发出给外部
			std::shared_ptr< colleague >        pt_producer__;  // 事件数据的生产者
			std::shared_ptr< colleague >        pt_consumer__;  // 事件数据的消费
		protected:
			// 通知的操作
			void call_leave__( const stateData_t& state , const  CONDITION_DATA_TYPE& data ){
				pt_producer__->sendTo( pt_consumer__ , state , private__::emEvent::EVT_LEAVE , data );
			}
			// 通知进入状态的操作
			void call_ent__( const stateData_t& state , const CONDITION_DATA_TYPE& data ){
				pt_producer__->sendTo( pt_consumer__ , state , private__::emEvent::EVT_ENT , data );
			}
		public:
			// 这个是构造函数，其中初始化了中介者模式
			state():m_is_running__( false ){
				// 准备中介者模式进行中间消息传递操作
				pt_mdt__ = std::make_shared<mediator_t>();

				pt_producer__ = std::make_shared<colleague>( pt_mdt__ );
				pt_consumer__ = std::make_shared<colleague>( pt_mdt__ );

				pt_mdt__->add( pt_producer__ );
				pt_mdt__->add( pt_consumer__ );
			}

			virtual ~state(){
				// 结束中介者模式
				if( pt_mdt__ ){
					pt_mdt__->run( false );
				}
			}
			/**
			 * @brief 登记状态转换动作函数。这个就是登记回调函数的操作
			 * @param fun[ I ]，动作函数
			 */
			void on( std::function< void (stateData_t , private__::emEvent , CONDITION_DATA_TYPE) > fun ){
				pt_consumer__->m_cb__ = fun;
			}
			/**
			 * @brief 指定起始状态。状态机就绪后从指定的节点开始
			 * @param data[ I ],状态
			 */
			void setStart( const stateData_t& data ){
				m_start__ = data;
				if( m_is_running__ == false ){
					m_current__ = data;
				}
			}
			/**
			 * @brief 指定结束状态。状态机完成后结束在这个节点
			 * @param data[ I ],状态
			 */
			void setEnd( const stateData_t& data ){
				m_end__ = data;
				if( m_is_running__ == false ){
					m_current__ = data;
				}
			}
			/**
			 * @brief 启动。启动状态机
			 * @param sw
			 */
			void start( bool sw ){
				if( m_is_running__ == sw ){ return; }

				m_is_running__ = sw;
				if( sw ){
					// 启动中介者模式后台
					pt_mdt__->run( true );
					m_current__ = m_start__;
					// 发送就绪通知
					pt_producer__->sendTo( pt_consumer__ , m_start__,  private__::emEvent::EVT_READY ,
							       std::is_arithmetic<CONDITION_DATA_TYPE>::value?CONDITION_DATA_TYPE():(CONDITION_DATA_TYPE)0.0 );
				}else{ // 结束运行
					pt_mdt__->run( false );
				}
			}
			/**
			 * @brief 执行一次无参数的转换。
			 * @return 成功操作返回true;否则返回false
			 */
			bool execute(){
				if(m_is_running__ == false ) return false;
				auto it = m_arcs__.find( m_current__ );
				if( it != m_arcs__.end() ){
					// 这个函数是没有参数条件的转换，主要针对自动转换的情况
					// 先调用离开
					call_leave__( it->m_second[0], std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );
					// 再调用进入
					call_ent__( it->m_second[0] , std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );

					m_current__ = it->m_second[ 0 ];
				}else{
					return false;
				}

				return true;
			}
			/**
			 * @brief 执行一次状态转换操作。这个函数是根据转换条件进行的转换操作
			 * @param data[ I ]， 状态转换的条件数据
			 * @return 成功操作返回true;否则返回false
			 */
			bool execute( const CONDITION_DATA_TYPE& data ){
				if(m_is_running__ == false ) return false;

				auto it = m_arcs__.find( m_current__ );
				if( it != m_arcs__.end() ){
					bool find_dst = false;
					for( auto item : it->second ){
						if( item.m_condition( data ) ){
							find_dst = true;
							call_leave__( item.m_from , data );
							call_ent__( item.m_to , data );
							// 执行结束通知
							if( item.m_to == m_end__ ){
								pt_producer__->sendTo( pt_consumer__ , m_end__,  private__::emEvent::EVT_END ,
										       std::is_arithmetic<CONDITION_DATA_TYPE>::value?CONDITION_DATA_TYPE():(CONDITION_DATA_TYPE)0.0 );
							}

							m_current__ = item.m_to;
							break;
						}
					}

					if( find_dst == false ){
						return false;
					}
				}else{
					return false;
				}

				return true;
			}
			/**
			 * @brief 删除状态。接下来的是添加和删除节点和状态连接的操作
			 * @param state
			 * @return
			 */
			bool addState( const stateData_t& state ){
				if(m_is_running__ == true ) return false;
				private__::stStat<stateData_t> s( state );
				auto rst = m_stats__.insert( std::make_pair(state , s ));
				return rst.second;
			}
			/**
			 * @brief 添加状态
			 * @param state
			 */
			void removeState( const stateData_t& state ){
				if(m_is_running__ == true ) return;
				auto it = std::find( m_stats__.begin() , m_stats__.end() , state );
				m_stats__.erase( it );

				// 清理状态起始ARC
				auto it1 = m_arcs__.find( state );
				if( it1.m_arcs__.end() ){
					m_arcs__.erase( it1 );
				}

				// 请里状态到达的ARC
				for( auto it2 = m_arcs__.begin(); it2 != m_arcs__.end(); it2 ++ ){
					for( auto it3 = it2->second.begin(); it3 != it2->second.end(); it3 ++ ){
						if( it3->m_to == state ){
							it2->second.erase( it3 );
						}
					}
				}
			}
			/**
			 * @brief 添加转换。添加转换的时候增加转换条件
			 * @param from
			 * @param to
			 * @param cnd
			 */
			void addArc( const stateData_t& from , const stateData_t& to , std::function< bool ( const CONDITION_DATA_TYPE& ) > cnd ){
				if(m_is_running__ == true ) return;
				auto it = m_arcs__.find( from );
		
				if( it != m_arcs__.end() ){
					private__::stArc<STATE_TYPE,CONDITION_DATA_TYPE> item( from , to , cnd );
					it->second.push_back( item );
				}else{
					private__::stArc<STATE_TYPE,CONDITION_DATA_TYPE> item( from , to , cnd );
					std::vector< private__::stArc<STATE_TYPE,CONDITION_DATA_TYPE> >  set({item});
			
					m_arcs__.insert( std::make_pair( from , set ));
				}
			}
			/**
			 * @brief 移除转换
			 * @param from
			 * @param to
			 */
			void removeArc( const stateData_t& from , const stateData_t& to ){
				if(m_is_running__ == true ) return;
				auto it = m_arcs__.find( from );
				if( it != m_arcs__.end() ){
					for( auto it1 = it->second.begin(); it != it->end(); it ++ ){
						if( it1->m_to == to ){
							it->second.erase( it1 );
						}
					}
				}
			}
		};
}}
