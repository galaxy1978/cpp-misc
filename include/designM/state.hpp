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
#include <unordered_set>

#include "designM/mediator.hpp"

namespace private__
{
	enum class emEvent
	{
		EVT_ENT,
		EVT_LEAVE,
        EVT_READY,
        EVT_END
	};
	template< typename STATE_TYPE >
	struct stStat
	{
		STATE_TYPE   m_state;
        stStat( const STATE_TYPE& s ):m_state( s ){}
	};

	template< typename STATE_TYPE , typename CONDITON_DATA_TYPE >
	struct stArc
	{
		STATE_TYPE  m_from;
		STATE_TYPE  m_to;

		std::function< bool ( const CONDITON_DATA_TYPE& ) >   m_condition;

        stArc( const STATE_TYPE& from , const STATE_TYPE& to ,std::function< bool ( const CONDITON_DATA_TYPE& ) > fun ):
            m_from( from ),
            m_to( to ),
            m_condition( fun ) {}
	};
}

namespace wheels{namespace dm{
template< typename STATE_TYPE , typename CONDITION_DATA_TYPE >
class state
{
public:
    static_assert( std::is_arithmetic<CONDITION_DATA_TYPE>::value ||
                      ( std::is_class<CONDITION_DATA_TYPE>::value && std::is_default_constructible<CONDITION_DATA_TYPE>::value ) , "" );

    using stateData_t = typename std::remove_pointer< typename std::decay< STATE_TYPE >::type >::type;
    using state_t = private__::stStat< stateData_t >;
    using arc_t = private__::stArc< stateData_t , CONDITION_DATA_TYPE >;
    using arcData_t = std::map< stateData_t , std::unordered_set< stateData_t > >;

	// 使用中介者模式传递消息
    class colleague;
    using mediator_t = wheels::dm::mediator< colleague >;

    class colleague : public colleagueItfc< mediator_t , stateData_t , private__::emEvent , CONDITION_DATA_TYPE >{
	public:
        std::function< void ( stateData_t , private__::emEvent , CONDITION_DATA_TYPE ) >  m_cb__;
	public :
        colleague(){}
        virtual ~colleague(){}
        /**
         * @brief 重载recv函数，并在其中针对tuple解包，调用回调函数通知状态变化
         * @param tpl[ I ]，
         */
        virtual void recv( const std::tuple< STATE_TYPE , private__::emEvent , CONDITION_DATA_TYPE >& tpl ) final{
			if( m_cb__ ){
				m_cb__( std::get<0>( tpl ) , std::get<1>( tpl ) , std::get<2>( tpl ) );
			}
        }
    };

protected:
    std::map< stateData_t , private__::stStat<stateData_t> >    m_stats__;
	arcData_t                           m_arcs__;

	stateData_t                         m_current__;	
	stateData_t                         m_start__;	
	stateData_t                         m_end__;
	std::atomic<bool>                   m_is_running__;

	std::shared_ptr< mediator_t >       pt_mdt__;
	std::shared_ptr< colleague >        pt_producer__;  // 事件数据的生产者
	std::shared_ptr< colleague >        pt_consumer__;  // 事件数据的消费
protected:
	void call_leave__( const stateData_t& state , const  CONDITION_DATA_TYPE& data ){
        pt_producer__->send( pt_consumer__ , state , private__::emEvent::EVT_LEAVE , data );
	}
	
	void call_ent__( const stateData_t& state , const  CONDITION_DATA_TYPE& data ){
        pt_producer__->send( pt_consumer__ , state , private__::emEvent::EVT_ENT , data );
	}
public:

    state():m_is_running__( false ){
        // 准备中介者模式进行中间消息传递操作
        pt_mdt__ = std::make_shared<mediator_t>();

        pt_producer__ = std::make_shared<colleague>();
        pt_consumer__ = std::make_shared<colleague>();

        pt_mdt__->add( pt_producer__ );
        pt_mdt__->add( pt_consumer__ );
    }

    virtual ~state(){
        if( pt_mdt__ ){
            pt_mdt__->run( false );
        }
    }
    /**
     * @brief 登记状态转换动作函数
     * @param fun[ I ]，动作函数
     */
    void on( std::function< void (stateData_t , private__::emEvent , CONDITION_DATA_TYPE) > fun ){
		pt_consumer__->m_cb__ = fun;
    }
    /**
     * @brief 指定起始状态
     * @param data[ I ],状态
     */
	void setStart( const stateData_t& data ){
		m_start__ = data;
		if( m_is_running__ == false ){
			m_current__ = data;
		}
	}
    /**
     * @brief 指定结束状态
     * @param data[ I ],状态
     */
    void setEnd( const stateData_t& data ){
        m_end__ = data;
        if( m_is_running__ == false ){
            m_current__ = data;
        }
    }
    /**
     * @brief 启动
     * @param sw
     */
	void start( bool sw ){
		if( m_is_running__ == sw ){
			return;
        }

        m_is_running__ = sw;
        if( sw ){
            pt_mdt__->run( true );
            m_current__ = m_start__;
            pt_producer__->send( pt_consumer__ ,
                                m_end__ ,
                                private__::emEvent::EVT_READY ,
                                std::is_arithmetic<CONDITION_DATA_TYPE>::value?CONDITION_DATA_TYPE():(CONDITION_DATA_TYPE)0.0 );
        }else{
             pt_mdt__->run( false );
        }
	}
    /**
     * @brief 执行一次无参数的转换
     * @return 成功操作返回true;否则返回false
     */
    bool execute(){
        if(m_is_running__ == false ) return false;
        auto it = m_arcs__.find( m_current__ );
        if( it != m_arcs__.end() ){
            call_leave__( it->m_second[0], std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );
            call_ent__( it->m_second[0] , std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );

            m_current__ = it->m_second[ 0 ];
        }else{
            return false;
        }

        return true;
    }
    /**
     * @brief 执行一次状态转换操作
     * @param data[ I ]， 状态转换的条件数据
     * @return 成功操作返回true;否则返回false
     */
    bool execute( const stateData_t& data ){
        if(m_is_running__ == false ) return false;

		auto it = m_arcs__.find( m_current__ );
		if( it != m_arcs__.end() ){
			bool find_dst = false;
			for( auto item : it->second ){
				if( item.m_condition( data ) ){
					find_dst = true;
					call_leave__( item.m_from , data );
					call_ent__( item.m_to , data );

                    if( item.m_to == m_end__ ){
                        pt_producer__->send( pt_consumer__ ,
                                             m_end__ ,
                                             private__::emEvent::EVT_END ,
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
     * @brief 删除状态
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
        if( it1 ){
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
     * @brief 添加转换
     * @param from
     * @param to
     * @param cnd
     */
    void addArc( const stateData_t& from , const stateData_t& to , std::function< bool ( const CONDITION_DATA_TYPE& ) > cnd ){
        if(m_is_running__ == true ) return;
        auto it = m_arcs__.find( from );
        if( it ){
            private__::stArc<STATE_TYPE,CONDITION_DATA_TYPE> item( from , to , cnd );
            it->second.insert( item );
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
        if( it ){
            for( auto it1 = it->second.begin(); it != it->end(); it ++ ){
                if( it1->m_to == to ){
                    it->second.erase( it1 );
                }
            }
        }
    }
};
}}
