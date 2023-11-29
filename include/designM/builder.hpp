/**
 * @brief 建造者模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-4~2023-11-26
 *
 * 2023-11-26 增加std::tuple支持，改善名字冲突的问题
*/

#pragma once
#include <memory>
#include <functional>
#include <stdexcept>
#include <type_traits>
namespace wheels{ namespace dm {
#if BUILD_USE_TUPLE == 1
#include <tuple>
template< itfcType , typename ...Parts >
class product
{
protected:
    std::tuple< Parts... >   m_parts__;
public:
    virtual ~product(){}

    template< int N >
    auto get()->decltype(typename std::tuple_element< N , prdtTypes >::type){
        return std::get< N >( m_parts__ );
    }

    template< int idx , typename PART_TYPE >
    void set( PART_TYPE param ){
        std::get< idx >( m_parts__ ) = param;
    }
};
#else
template< typename itfcType , typename... Parts > class product{};

template< typename itfcType , typename Part1 , typename... Parts >
class product<itfcType , Part1 , Parts... > : public Part1 , public product< itfcType , Parts... >
{
public:
    virtual ~product(){}
};


template< typename itfcType >
class product<itfcType>: public itfcType{
public:
    virtual ~product(){}
};
#endif

// 如果要使用接口方式使用director，需要BUILD_USE_DIR_ITFC配置为1，并实现
// 相关的虚函数。调用build函数的时候，就
#if BUILD_USE_DIR_ITFC == 1
struct dir_base_itfc{};

template< typename pdtItfcType , typename ...Params >
struct directorItfc : public dir_base_itfc
{
    virtual std::shared_ptr<pdtItfcType> build( Params&& ... args ) = 0;
};
#endif
/**
 * @brief
 * @tparam Parts ，组件表
*/
template<
#if BUILD_USE_DIR_ITFC == 1
    typename DIR_T
#endif
    typename pdtItfcType , typename... Parts >
class director
#if BUILD_USE_DIR_ITFC == 1
    public DIR_T
#endif
{
public:
    using itfc_t = typename std::remove_pointer< typename std::decay< pdtItfcType >::type >::type;
    using product_t = product< itfc_t , Parts... >;
#if BUILD_USE_DIR_ITFC == 1
    static_assert( std::is_base_of< dir_base_itfc , DIR_T >::value , "" )
#endif
public:
    director(){}
    virtual ~director(){}
#if BUILD_USE_DIR_ITFC == 0
    template< typename... Params >
    static  std::shared_ptr< product_t >
    build( std::function< void ( std::shared_ptr< product_t > ) >fun , Params&&... args ){
        std::shared_ptr< product_t >   pt_product;
        try{
            pt_product = std::make_shared< product_t >( std::forward<Params>( args )...);
            if( fun ){
                fun( pt_product );
            }
        }catch( std::bad_alloc& e ){
            if( pt_product ){
                pt_product.reset(  );
            }
        }catch( std::runtime_error& e  ){
            if( pt_product ){
                pt_product.reset( );
            }
        }

        return pt_product;
    }
#endif
};
}}
