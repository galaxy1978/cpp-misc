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
         * @param tpl
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
        pt_producer__ = std::make_shared<colleague>();
    }
	virtual ~state(){}

	void on( std::function< void () > fun ){
		pt_consumer__->m_cb__ = fun;
	}
	
	void setStart( const stateData_t& data ){
		m_start__ = data;
		if( m_is_running__ == false ){
			m_current__ = data;
		}
	}

    void setEnd( const stateData_t& data ){
        m_end__ = data;
        if( m_is_running__ == false ){
            m_current__ = data;
        }
    }

	void start( bool sw ){
		if( m_is_running__ == sw ){
			return;
        }

        m_is_running__ = sw;
        if( sw ){
            m_current__ = m_start__;
            pt_producer__->send( pt_consumer__ ,
                                m_end__ ,
                                private__::emEvent::EVT_READY ,
                                std::is_arithmetic<CONDITION_DATA_TYPE>::value?CONDITION_DATA_TYPE():(CONDITION_DATA_TYPE)0.0 );
        }
	}

    bool execute(){
        auto it = m_arcs__.find( m_current__ );
        if( it != m_arcs__.end() ){
            call_leave__( it->m_second[0], std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );
            call_ent__( it->m_second[0] , std::is_arithmetic<CONDITION_DATA_TYPE>::value ? CONDITION_DATA_TYPE() : (CONDITION_DATA_TYPE)0.0 );
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

    bool addState( const stateData_t& state ){
        private__::stStat<stateData_t> s( state );
        auto rst = m_stats__.insert( std::make_pair(state , s ));
        return rst.second;
	}

	void removeState( const stateData_t& state ){
        auto it = std::find( m_stats__.begin() , m_stats__.end() , state );
        m_stats__.erase( it );

        // 清理状态起始ARM
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

    void addArc( const stateData_t& from , const stateData_t& to , std::function< bool ( const CONDITION_DATA_TYPE& ) > cnd ){
        auto it = m_arcs__.find( from );
        if( it ){
            private__::stArc<STATE_TYPE,CONDITION_DATA_TYPE> item( from , to , cnd );
            it->second.insert( item );
        }
	}

    void removeArc( const stateData_t& from , const stateData_t& to ){
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
