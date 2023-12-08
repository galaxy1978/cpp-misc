/**
 * @brief 享元模式实现。支持多个不同的类型的享元同时管理；线程安全；支持任意参数的构造函数；
 * @version 1.0
 * @author 宋炜
 * @date 2023-6-12 ~ 2023-12-8
 *   2023-12-8 改变了内部实现方式，使用wheels::variant来保存数据内容。
 */

#pragma once

#include <mutex>
#include <map>

#include "container/variant.hpp"
namespace wheels{namespace dm {
    template< typename idType >
    class flyweight
    {
    protected:
	    // 考虑一个数据可能会被很多接口使用，这样需要考虑数据的线程安全
	    mutable std::mutex             			m_mutex__;
	    std::map< idType , wheels::variant >    m_itfcs__;
	public:
		using iterator = typename std::map< idType , wheels::variant >::iterator;
    public:
	    flyweight(){}
	    virtual ~flyweight(){}
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
		    bool ret = false;
		    auto var = wheels::variant::make( T( std::forward<PARAMS>(args)...) );
		    
		    std::lock_guard< std::mutex> lock( m_mutex__ );
				
		    auto it = m_itfcs__.find( id );
		    if( it != m_itfcs__.end() ){
			    it->second = std::move( var );
		    }else{
			    m_itfcs__.insert( std::make_pair( id , wheels::variant::make( T(std::forward<PARAMS>(args)... ) ) ) );
		    }
				
		    return ret;
	    }
	    /**
	     * @brief 获取指定的名称的数据指针
	     */
	    variant get( const idType& name ){
				
		    std::lock_guard< std::mutex> lock( m_mutex__ );
				
		    auto it = m_itfcs__.find( name );
		    if( it != m_itfcs__.end() ){
			    return it->second; 
		    }
				
		    return {};
	    }
		
		size_t count(){ return m_itfcs__.size(); }
	    /**
	     * @brief 获取享元对象引用
	     */
	    wheels::variant 
		get( const idType& name ) const{
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
		
		void erase( iterator it ){ m_itfcs__.erase( it ); }
		void erase( iterator b , iterator e ){ m_itfcs__.erase( b , e ); }
		
		iterator begin(){ return m_itfcs__.begin(); }
		iterator end(){ return m_itfcs__.end(); }
    };
}}
