/**
 * @brief 中介者模式
 * @version 1.1
 * @author 宋炜
 *   新增异步发送消息的功能
*/

#pragma once
#include <type_traits>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <vector>

//#define MEDIATOR_USE_TUPLE (1)

#if defined( MEDIATOR_USE_TUPLE )
#	include <tuple>
#   include <thread>
#	include <condition_variable>
#   include <functional>
#   include <mutex>
#   include <queue>
#   include <atomic>
#endif

namespace private__
{
	struct colleagueBase{};
}

namespace wheels { namespace dm {

#if !defined( MEDIATOR_USE_TUPLE )
    template< typename MEDIATOR , typename ...PARAMS >
    struct colleagueItfc : public private__::colleagueBase
    {
    public:
        using mediator_t = typename std::remove_pointer< typename std::decay<MEDIATOR>::type >::type;
		using tuple_t = std::tuple< PARAMS... >;
    protected:
        std::shared_ptr< mediator_t >    pt_mediator__;
    public:
        virtual void send( PARAMS&& ...args ) = 0;
        virtual void recv( PARAMS&& ...args ) = 0;
    };
#else
	template< typename MEDIATOR , typename ...PARAMS >
	struct colleagueItfc : public private__::colleagueBase
	{
	public:
		 using mediator_t = typename std::remove_pointer< typename std::decay<MEDIATOR>::type >::type;
	protected:
		 std::shared_ptr< mediator_t >    pt_mediator__;
    public:
        virtual void send( PARAMS&& ...args ) = 0;
        virtual void recv( const std::tuple<PARAMS...>& tpl  ) = 0;
    };
#endif

    template< typename ITFC_TYPE >
    class mediator
    {
    public:
        using itfc_t = typename std::remove_pointer< typename std::decay<ITFC_TYPE>::type >::type;
        using data_t = std::unordered_set< std::shared_ptr< itfc_t > >;
#if defined( MEDIATOR_USE_TUPLE )
        using tuple_t = typename itfc_t::tuple_t;
        struct stMsgs{
            std::shared_ptr< itfc_t >   pt_dst__;
            std::shared_ptr< tuple_t >  m_data__;
        };
#endif
        static_assert( std::is_base_of<itfc_t , private__::colleagueBase>::value , "" );

    protected:
        data_t      m_colleague__;

#if defined( MEDIATOR_USE_TUPLE )

        std::queue< stMsgs >      m_msgs__;
        std::condition_variable   m_cnd_var__;
        std::atomic< bool >       m_is_running__;
        std::mutex                m_mutex__;
	protected:
        void backend__(){
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
        mediator(){}
        virtual ~mediator(){}

        void add( std::shared_ptr< itfc_t > colleague) {
#if defined( MEDIATOR_USE_TUPLE )
            std::unique_lock< std::mutex >  lck( m_mutex__ );
#endif
            m_colleague__.insert(colleague);
        }

        void erase( std::shared_ptr< itfc_t > colleague) {
#if defined( MEDIATOR_USE_TUPLE )
            std::unique_lock< std::mutex >  lck( m_mutex__ );
#endif
            auto it = std::find( m_colleague__.begin() , m_colleague__.end() , colleague );
            if( it != m_colleague__.end() ){
                m_colleague__.erase( it );
            }
        }

#if defined( MEDIATOR_USE_TUPLE )
        bool run( bool sw ){
            if( sw ){
                m_is_running__ = true;
                std::thread thd( std::bind( &mediator<itfc_t>::backend__ ,  this ) );
                thd.detach();
            }else{
                m_is_running__ = false;
                m_cnd_var__.notify_one();
            }
        }

        template< typename ...PARAMS >
        void dispatch(std::shared_ptr< itfc_t > sender, PARAMS&& ...args ) {
            auto pt_tpl = std::make_shared< std::tuple<PARAMS...> >( std::forward<PARAMS>(args)... );
            std::unique_lock< std::mutex >  lck( m_mutex__ );

            for (auto colleague : m_colleague__) {
                if (colleague != sender) {
                    stMsgs msg = { colleague , pt_tpl };
                    m_msgs__.push( msg );
                }
            }

            m_cnd_var__.notify_one();
        }

        template< typename ...PARAMS >
        void dispatchTo(std::shared_ptr< itfc_t > to, PARAMS&& ...args ) {
            auto it = m_colleague__.find( to );
            if( it != m_colleague__.end() ){
               auto pt_tpl = std::make_shared< std::tuple<PARAMS...> >( std::forward<PARAMS>(args)... );
               stMsgs msg = { to , pt_tpl };
               m_msgs__.push( msg );
            }

            m_cnd_var__.notify_one();
        }

        template< typename ...PARAMS >
        void dispatchTo( std::vector< std::shared_ptr< itfc_t > > dests, PARAMS&& ...args ) {
            auto pt_tpl = std::make_shared< std::tuple<PARAMS...> >( std::forward<PARAMS>(args)... );

            for( auto to : dests ){
                auto it = m_colleague__.find( to );
                if( it != m_colleague__.end() ){
                    stMsgs msg = { to , pt_tpl };
                    m_msgs__.push( msg );
                }
            }

            m_cnd_var__.notify_one();
        }
#else
        template< typename ...PARAMS >
        void dispatch(std::shared_ptr< itfc_t > sender, PARAMS&& ...args ) {
            for (auto colleague : m_colleague__) {
                if (colleague != sender) {
                    colleague->recv( std::forward<PARAMS>( args )...);
                }
            }
        }

        template< typename ...PARAMS >
        void dispatchTo(std::shared_ptr< itfc_t > to, PARAMS&& ...args ) {
            auto it = m_colleague__.find( to );
            if( it != m_colleague__.end() ){
                to->recv( std::forward<PARAMS>( args )...);
            }
        }

        template< typename ...PARAMS >
        void dispatchTo( std::vector< std::shared_ptr< itfc_t > > dests, PARAMS&& ...args ) {
            for( auto to : dests ){
                auto it = m_colleague__.find( to );
                if( it != m_colleague__.end() ){
                    to->recv( std::forward<PARAMS>( args )...);
                }
            }
        }
#endif
    };
}}
