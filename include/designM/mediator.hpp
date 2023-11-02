/**
 * @brief 中介者模式
 * @version 1.0
 * @author 宋炜
*/

#pragma once
#include <type_traits>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <vector>

namespace private__
{
	struct colleagueBase{};
}

namespace wheels { namespace dm {

    template< typename MEDIATOR , typename ...PARAMS >
    struct colleagueItfc : public private__::colleagueBase
    {
    public:
        using mediator_t = typename std::remove_pointer< typename std::decay<MEDIATOR>::type >::type;
    protected:
        std::shared_ptr< mediator_t >    pt_mediator__;
    public:
        virtual void send( PARAMS&& ...args ) = 0;
        virtual void recv( PARAMS&& ...args ) = 0;
    };


    template< typename ITFC_TYPE >
    class mediator
    {
    public:
        using itfc_t = typename std::remove_pointer< typename std::decay<ITFC_TYPE>::type >::type;
        using data_t = std::unordered_set< std::shared_ptr< itfc_t > >;

        static_assert( std::is_base_of<itfc_t , private__::colleagueBase>::value , "" );

    protected:
        data_t      m_colleague__;
    public:
        mediator(){}
        virtual ~mediator(){}

        void add( std::shared_ptr< itfc_t > colleague) {
            m_colleague__.insert(colleague);
        }

        void erase( std::shared_ptr< itfc_t > colleague) {
            auto it = std::find( m_colleague__.begin() , m_colleague__.end() , colleague );
            if( it != m_colleague__.end() ){
                m_colleague__.erase( it );
            }
        }

        template< typename ...PARAMS >
        void dispatch(std::shared_ptr< itfc_t > sender, PARAMS&& ...args ) {
            for (auto colleague : m_colleague__) {
                if (colleague != sender) {
                    colleague->receive( std::forward<PARAMS>( args )...);
                }
            }
        }

        template< typename ...PARAMS >
        void dispatchTo(std::shared_ptr< itfc_t > to, PARAMS&& ...args ) {
            auto it = m_colleague__.find( to );
            if( it != m_colleague__.end() ){
                to->receive( std::forward<PARAMS>( args )...);
            }
        }

        template< typename ...PARAMS >
        void dispatchTo( std::vector< std::shared_ptr< itfc_t > > dests, PARAMS&& ...args ) {
            for( auto to : dests ){
                auto it = m_colleague__.find( to );
                if( it != m_colleague__.end() ){
                    to->receive( std::forward<PARAMS>( args )...);
                }
            }
        }
    };
}}
