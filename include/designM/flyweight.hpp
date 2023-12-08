/**
 * @brief 享元模式实现。支持多个不同的类型的享元同时管理；线程安全；支持任意参数的构造函数；
 * @version 1.1
 * @author 宋炜
 * @date 2023-6-12 ~ 2023-12-8
 *   2023-12-8 改变了内部实现方式，使用wheels::variant来保存数据内容。并增加了接口方式处理数据
 */

#pragma once

#include <mutex>
#include <map>

#if !defined( FLYWEIGHT_USE_VARIANT )
#    define FLYWEIGHT_USE_VARIANT   (1)
#endif 

#if FLYWEIGHT_USE_VARIANT == 1
#   include "container/variant.hpp"
#else 
namespace private__
{
	struct dataItfc__{
		virtual ~dataItfc__(){}
	};
}
#endif
namespace wheels{namespace dm {
#if !FLYWEIGHT_USE_VARIANT 
	template< typename dataType >
	struct dataItfc : public private__::dataItfc__
	{
		virtual void set( const dataType& d ) = 0;
		virtual const dataType& get() const = 0;
	};
#endif
    template< typename idType >
    class flyweight
    {
    protected:
	    // 考虑一个数据可能会被很多接口使用，这样需要考虑数据的线程安全
	    mutable std::mutex             			m_mutex__;
#if FLYWEIGHT_USE_VARIANT == 1
	    std::map< idType , wheels::variant >    m_itfcs__;
#else 
		std::map< idType , private__::dataItfc__ * >       m_itfcs__;
#endif
	public:
#if FLYWEIGHT_USE_VARIANT == 1
		using iterator = typename std::map< idType , wheels::variant >::iterator;
#else 
		using iterator = typename std::map< idType , private__::dataItfc__ * >::iterator;
#endif
    public:
	    flyweight(){}
	    virtual ~flyweight(){
#if !FLYWEIGHT_USE_VARIANT
			for( auto item : m_itfcs__ ){
				delete item.second;
			}
#endif
		}
	    /**
	     * @brief 判断是否存在指定的享元
	     */
	    bool has( const idType& id ){
		    std::lock_guard< std::mutex> lock( m_mutex__ );
		    auto it = m_itfcs__.find( id );
		    return (it != m_itfcs__.end() );
	    }
	    /**
	     * @brief 修改指定的内容
	     * @note 相对应的对象必须支持set函数
	     */
	    template< typename T , typename ...PARAMS >
	    bool set( const idType& id , PARAMS&&... args ){
		    bool ret = true;
#if FLYWEIGHT_USE_VARIANT == 1
		    auto var = wheels::variant::make( T( std::forward<PARAMS>(args)...) );
		    
		    std::lock_guard< std::mutex> lock( m_mutex__ );
				
		    auto it = m_itfcs__.find( id );
		    if( it != m_itfcs__.end() ){
			    it->second = std::move( var );
		    }else{
			    m_itfcs__.insert( std::make_pair( id , wheels::variant::make( T(std::forward<PARAMS>(args)... ) ) ) );
		    }
			
#else 
			static_assert( std::is_base_of< private__ :: dataItfc__ , T>::value , "" );
			static_assert( ( std::is_class< T >::value && std::is_default_constructible< T >::value ) || std::is_arithmetic<T>::value , "" );
			auto it = m_itfcs__.find( id );
			if( it != m_itfcs__.end() ){
				auto p = dynamic_cast< dataItfc<PARAMS...> * >( it->second );
				p->set( std::forward<PARAMS>(args)... );
			}else{
				try{
					auto * t = new T;
					m_itfcs__.insert( std::make_pair( id , t ) );
					t->set( std::forward<PARAMS>(args)... );
				}catch( std::bad_alloc& ){
					ret = false;
				}
			}
#endif
		    return ret;
	    }
	    /**
	     * @brief 获取指定的名称的数据指针
	     */
#if FLYWEIGHT_USE_VARIANT == 1
	    variant get( const idType& name ){
				
		    std::lock_guard< std::mutex> lock( m_mutex__ );
				
		    auto it = m_itfcs__.find( name );
		    if( it != m_itfcs__.end() ){
			    return it->second; 
		    }
				
		    return {};
	    }
		
		wheels::variant 
		operator[](const idType& name){
			auto it = m_itfcs__.find( name );
		    if( it != m_itfcs__.end() ){
			    return it->second;
		    }
			
			return {};
		}
#else
		template< typename T >
		T * get( const idType& name ){
			static_assert( std::is_base_of< private__ :: dataItfc__	, T >::value , "" );
		    std::lock_guard< std::mutex> lock( m_mutex__ );
				
		    auto it = m_itfcs__.find( name );
		    if( it != m_itfcs__.end() ){
			    return dynamic_cast<T*>(it->second); 
		    }
				
		    return nullptr;
	    }
#endif
		size_t count(){ return m_itfcs__.size(); }
	
		void erase( iterator it ){ m_itfcs__.erase( it ); }
		void erase( iterator b , iterator e ){ m_itfcs__.erase( b , e ); }
		
		iterator begin(){ return m_itfcs__.begin(); }
		iterator end(){ return m_itfcs__.end(); }
    };
}}
