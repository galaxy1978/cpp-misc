/**
 * @brief 桥接模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once
#include <type_traits>
#include <memory>
#include <functional>
#include <stdexcept>

namespace wheels{namespace dm{
template< typename itfcType , typename implType >
class bridge
{
public:
    using itfc_t = typename std::remove_pointer< typename std::decay< itfcType >::type >::type;
    using impl_t = typename std::remove_pointer< typename std::decay< implType >::type >::type;
    // 实际实现类必须是接口类的子类
    static_assert( std::is_base_of<itfcType , implType >::value, "Interface type must be base class of implemention class." );

private:
    std::shared_ptr< impl_t > pt_imp__;
public:
    bridge(){}
    template< typename ...Args >
    bridge(Args&&... args ){
        pt_imp__ = std::make_shared< impl_t >( std::forward< Args >(args)...);
    }

    virtual ~bridge(){}

    template< typename ...Args >
    static bridge * create( Args&&... args ){

        bridge * ret = new bridge( std::forward<Args>(args)... );
        return ret;
    }

    template< typename ...Args >
    static bridge * create( std::function< void ( bridge * b , Args&&...) > func,  Args&&... args ){

        bridge * ret = new bridge( std::forward<Args>(args)... );

        if( func ){
            func( ret , std::forward<Args>(args)... );
        }

        return ret;
    }

    /**
     * @brief 重载方便访问具体实现的接口。
     * @exception 如果实现对象指针为空，则抛出异常
     */
    impl_t * operator->(){
        if( pt_imp__ == nullptr ) throw std::runtime_error( "object empty" );
        return pt_imp__.get();
    }

    impl_t& operator*(){
        if( pt_imp__ == nullptr ) throw std::runtime_error( "object empty" );;
        return *pt_imp__.get();
    }

};
}}
