/**
 * @brief 享元模式实现。支持多个不同的类型的享元同时管理；线程安全；支持任意参数的构造函数；
 * @version 1.0
 * @author 宋炜
 * @date 
*/

#pragma once

#include <mutex>
#include <map>
#include <vector>
#include <iostream>

namespace wheels
{
	namespace dm 
	{
		namespace private__
		{
			
            /// 迭代检查每一个接口是否满足考虑函数的要求
            template<typename ...Args >
            class checkItfc__{};

            template< typename T >
            class checkItfc__<T>{
				static_assert( std::is_copy_assignable< T > :: value || std::is_copy_constructible< T >::value , "interface must be copiable." );
			};

            template< typename T1 , typename ...Args >
            class checkItfc__<T1, Args... > {
                static_assert( std::is_copy_assignable< T1 > :: value || std::is_copy_constructible< T1 >::value , "interface must be copiable." );
            };

			/// 删除器，用来迭代删除实际对象
            template< int N , typename... Args >
            struct deleter__{};
			
            template< int N , typename T1 , typename ...Args >
            struct deleter__< N, T1, Args... >{
                void deleteIt( std::vector< void *>& p ){
					delete (T1*)p[N];
				}
            };
            template<> struct deleter__<0>{};
		
		}
		
		template< typename idType , typename ...Args >
		class flyweight : public private__::checkItfc__<Args... >
		{
		protected:
			// 考虑一个数据可能会被很多接口使用，这样需要考虑数据的线程安全
			mutable std::mutex             __m_mutex;
			std::map< idType , void * >    __m_itfcs;
			std::vector< void * >          __m_v_itfcs;
		public:
			flyweight(){}
			virtual ~flyweight(){
                private__::deleter__< sizeof...(Args) ,  Args... >::deleteIt( __m_v_itfcs );
			}
			/**
			 * @brief 判断是否存在指定的享元
			*/
			bool has( const idType& id ){
				std::lock_guard< std::mutex> lock( __m_mutex );
				auto it = __m_itfcs.find( id );
				return (it != __m_itfcs.end() );
			}
			/**
			* @brief 修改指定的内容
			* @note 相对应的对象必须支持set函数
			*/
            template< typename T , typename ...PARAMS >
            bool set( const idType& id , PARAMS&&... args ){
				static_assert( std::is_class< T >::value , "flyweight data must be a class." );
				
				bool ret = false;
				std::lock_guard< std::mutex> lock( __m_mutex );
				
				auto it = __m_itfcs.find( id );
				if( it != __m_itfcs.end() ){
					T * itfc = static_cast<T*>(it->second );
					ret = itfc->set( (0,args)... );
				}
				
				return ret;
			}
			/**
			 * @brief 获取指定的名称的数据指针
			*/
            template< typename rawT ,
                     typename midT = typename std::decay< rawT >::type ,
                     typename T = typename std::conditional< std::is_pointer< midT >::value ,
                                  typename std::remove_pointer< midT >::type , midT > >
			T * get( const idType& name ){
				
				std::lock_guard< std::mutex> lock( __m_mutex );
				
				auto it = __m_itfcs.find( name );
				if( it != __m_itfcs.end() ){
					return (T*)it->second;
				}
				
				return nullptr;
			}
			/**
			 * @brief 获取享元对象引用
			 * @tparam rawT
			 * @param name
			 * @return 
			*/
			template< typename T >
			const T& get( const idType& name ) const{
				std::lock_guard< std::mutex> lock( __m_mutex );
				
				auto it = __m_itfcs.find( name );
				if( it != __m_itfcs.end() ){
					return *(T*)it->second;
				}
				
				return nullptr;
			}
			
			/**
			 * @brief 根据提供的类型创建享元对象
			 * @tparam T 享元对象的类型
			 * @tparam Params 享元对象构造的时候需要传递的参数
			 * @param name[ I ], 享元名称
			 * @param args[ I ], 调用享元类型构造函数的参数表
			 * @return 成功操作返回享元指针，否则返回nullptr
			*/
			template<typename T , typename ...Params >
			T * get(const idType& name, Params... args ){
				static_assert( std::is_class< T >::value , "flyweight data must be a class." );

				T * ret = nullptr;
				// 如果数据已经存在则返回数据，并且在返回数据前按照新的内容更改数据
				if( has( name ) == true ){
					auto it = __m_itfcs.find();
					
					ret = reinterpret_cast< T *>( it->second );
					ret->set( (0,args)... );
					return ret;
				}
				
				try{
					std::lock_guard< std::mutex> lock( __m_mutex );
					ret = new T( (0,args)... );
					
					__m_itfcs.insert( std::make_pair( name, ret ) );
                    __m_v_itfcs.push_back( (void*)ret );
				}catch( std::bad_alloc& e ){
					std::cout << e.what() << std::endl;
					ret = nullptr;
				}
				
				return ret;
			}
		};
		
	}
}