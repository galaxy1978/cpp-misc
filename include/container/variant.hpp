/**
 * @brief 任意类型数据的容器
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-8
 */
#pragma once

#include <assert.h>

#include <string>
#include <type_traits>
#include <typeinfo>
#include <mutex>
#include <stdexcept>

// 默认开启类型检查。
// 如果要关闭可以在工程文件中添加自己的定义, 比如在Makefile中增加 -DVARIANT_USE_TYPE_CHECK=0

#if !defined( VARIANT_USE_TYPE_CHECK )
#    define VARIANT_USE_TYPE_CHECK  (1)
#endif

namespace wheels
{
	/// 这个名字空间中的内容是底层实现部分。
	namespace private__{
		class variant_base__{
		public:
			virtual ~variant_base__(){}
			virtual variant_base__ * clone() = 0;
			// 如果需要检查操作的时候需要检查数据，则在编译的时候要定义宏
			// 并将宏的值配置成1
#if VARIANT_USE_TYPE_CHECK
			virtual const std::string typeInfo() const= 0;
#endif
		};

		// *********************************************************************************************************************
		template< typename T >
		class variant__ : public variant_base__{
		private:
			T    __m_data;    // 实际数据内容
		public:
			variant__(){}
			variant__( const T& value ) : __m_data(value){}
			virtual ~variant__(){}
			
			T get(){ return __m_data; }

			void set( const T& b ){
				__m_data = b;
			}
			/**
			 * @brief 拷贝操作。主要用于外部的拷贝构造、赋值拷贝。
			 * @return 成功操作返回
			 */
			virtual variant_base__ *clone() final{
				variant__< T > * ret = nullptr;
				
				try{
					ret = new variant__< T >();
                    ret->__m_data = std::move( __m_data );
				}catch( std::bad_alloc& e ){
					ret = nullptr;
				}

				return ret;
			}
#if VARIANT_USE_TYPE_CHECK
			virtual const std::string typeInfo() const final{
				return std::string( typeid( T ).name() );
			}
#endif
		};
	}
	// ***************************************************************************************************************************
	/// 实际暴露出来的接口类。这个类具有明确的类型，可以在其他容器中，比如vector中，使用
	class variant{
            
	private:
		private__::variant_base__    * __p_data;
#if VARIANT_USE_TYPE_CHECK
		std::string                    __m_typeinfo;    // 保存数据类型描述
#endif
		
		mutable std::mutex             __m_mutex;     //
	private:
		
	public:
		variant():__p_data( nullptr ){}		
		virtual ~variant(){
			if( __p_data ){
				delete __p_data;
			}
		}

		variant( const variant& b ) : __p_data( nullptr )
		{
			__p_data = b.__p_data->clone();
#if VARIANT_USE_TYPE_CHECK
			__m_typeinfo = b.__p_data->typeInfo();
#endif
		}
		variant( variant&& b ): __p_data( nullptr )
		{
			auto * __p_temp = __p_data;
			__p_data = b.__p_data;
			b.__p_data = __p_temp;
#if VARIANT_USE_TYPE_CHECK
			auto __p_t = __m_typeinfo;
			__m_typeinfo = b.__m_typeinfo;
			b.__m_typeinfo = __p_t;
#endif
		}

		variant& operator=( const variant& b ){
			__p_data = b.__p_data->clone();
#if VARIANT_USE_TYPE_CHECK
			__m_typeinfo = b.__p_data->typeInfo();
#endif
			return *this;
		}

		variant& operator=( variant&& b ){
			auto * __p_temp = __p_data;
			__p_data = b.__p_data;
			b.__p_data = __p_temp;
#if VARIANT_USE_TYPE_CHECK
			auto __p_t = __m_typeinfo;
			__m_typeinfo = b.__m_typeinfo;
			b.__m_typeinfo = __p_t;
#endif
			return *this;
		}
		/// 针对=进行重载方便赋值
		inline variant& operator=( bool data ){ set( data ); return *this; }
		inline variant& operator=( char data ){ set(data); return *this; }
		inline variant& operator=( short data ){ set( data ); return *this; }
		inline variant& operator=( int data ){ set( data ); return *this; }
		inline variant& operator=( int64_t data ){ set( data ); return *this; }
		inline variant& operator=( uint8_t data ){ set(data); return *this; }
		inline variant& operator=( uint16_t data ){ set( data ); return *this; }
		inline variant& operator=( uint32_t data ){ set( data ); return *this; }
		inline variant& operator=( uint64_t data ){ set( data ); return *this; }
		inline variant& operator=( float data ){ set( data ); return *this; }
		inline variant& operator=( double data ){ set( data ); return *this; }

		inline variant& operator=( const std::string& data ){ set( data ); return *this; }
		
		/**
		 * @brief 构建数据对象
		 * @tparam 数据的原始类型
		 * @param data[ I ], 初始化值
		 * @return 返回构造后的右值引用
		 * @exceptions 这个函数会在内部分配内存
		 */
		template< typename dataType >
		static variant make( const dataType& data ){
			variant  ret;
#if VARIANT_USE_TYPE_CHECK
			ret.__m_typeinfo = typeid( dataType ).name();
#endif			
			ret.__p_data = new private__::variant__< dataType >( data );
			return ret ;
		}

		template<typename dataType >
		dataType get( )const{
			std::lock_guard< std::mutex > lock( __m_mutex );
#if VARIANT_USE_TYPE_CHECK
			assert( __m_typeinfo == typeid( dataType ).name() );
#endif
			private__::variant__<dataType>  * p = static_cast< private__::variant__<dataType> *>( __p_data );
			if( !p ){
				throw std::runtime_error( "data empty" );
			}

			return p->get();
		}
		/**
		 * @brief 判断类型是否相符合.
		 *   例如：
		 *       if( is<int>() ){
		 *            do something
		 *            ... ...
		 *       }
		 * @return 符合返回true，否则返回false
		 */
		template< typename dataType >
		bool is(){
		      return (__m_typeinfo == typeid( dataType ).name() );
	        }
		
		/**
		 * @brief 指定数据
		 */
		template< typename dataType >
		void set( const dataType& data ){
			std::lock_guard< std::mutex > lock( __m_mutex );
#if VARIANT_USE_TYPE_CHECK
			assert( __m_typeinfo == typeid( dataType ).name() );
#endif
			private__::variant__<dataType>  * p = static_cast< private__::variant__<dataType>* >( __p_data );
			if( p ){
				return p->set( data );
			}else{
				throw std::runtime_error( "数据内存错误" );
			}
		}

				
	};
}
