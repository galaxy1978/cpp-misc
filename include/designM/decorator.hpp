/**
 * @brief 装饰器模式
 * @version 1.1
 * @author 宋炜
 * @date 2023-4-23
 * 
 *
 * 2024-1-7 新增接口约束方式；新增接口声明宏 
 */

#pragma once
#include <type_traits>
#include <vector>
#include <functional>

namespace decorator_private__
{
	struct stDecorateeItfc__{
		virtual ~stDecorateeItfc__(){}
	};
}

#define BEGIN_DECLARE_DECORTEE_ITFC( name )								\
	struct name: public  decorator_private__::stDecorateeItfc__{		\
		virtual ~name(){}
		
#define DECORTEE_METHOD( RET , NAME , ... )    							\
	virtual RET NAME( __VA_ARGS__ ) = 0;
	
#define END_DECLARE_DECORTEE_ITFC()	};

#define DECLARE_DECORTEE_DEFAULT( name , ... )    						\
	BEGIN_DECLARE_DECORTEE_ITFC( name )									\
		DECORTEE_METHOD( void , operation , __VA_ARGS__ )				\
	END_DECLARE_DECORTEE_ITFC()

namespace wheels
{
	namespace dm
	{
		template< typename itfcType >
		class decorator : public  decorator_private__::stDecorateeItfc__
		{		
		public:
            using itfc_t = typename std::remove_pointer< typename std::decay<itfcType>::type >::type;
			static_assert( std::is_base_of<decorator_private__::stDecorateeItfc__ , itfc_t>::value , "" );
			/**
			 * @brief 装饰品包装，类似与化妆品包装盒
			 */
			class decoratee 
			{
			protected:
                itfc_t  * p_imp__;     // 实际的装饰品，类似实际的化妆品
			public:
                decoratee():p_imp__( nullptr ){}
                decoratee( itfc_t * imp ):p_imp__( imp ){}
                decoratee( decoratee&& b ): p_imp__( b.p_imp__ ){}

				decoratee& operator=( decoratee&& b ){
                    p_imp__ = b.p_imp__;
					return *this;
				}
			
                itfc_t * operator->(){ return p_imp__; }

                void set( itfc_t * imp ){ p_imp__ = imp; }
                itfc_t * get(){ return p_imp__; }
			};

			// 装饰品数据类型包装数据类型
            using dcrte_t = decoratee;
			// 装饰品
			using data_t = std::vector< dcrte_t >;
            using iterator = typename data_t :: iterator;
		protected:
            data_t m_dcrtes__;
		public:
			decorator(){}
			virtual ~decorator(){}

			/**
			 * @brief 添加装饰品.
			 * @param dcrtee[ I ], 装饰品对象
			 * @return 装饰id,也就是索引号。
			 * @note 添加装饰的时候返回的是其索引，如果后续修改了需要同步操作
			 */
            size_t decrat( dcrte_t& dcrtee ){
                m_dcrtes__.push_back( dcrtee );
                return m_dcrtes__.size() - 1;
			}
			/**
			 * @brief 通过接口类型添加装饰品
			 * @param dcrtee
			 * @return
			 */
			size_t decrat( itfcType * dcrtee ) {
                m_dcrtes__.push_back( dcrte_t( dcrtee ) );
                return m_dcrtes__.size() - 1;
			}
			
			/**
			 * @brief 移除装饰品
			 */
			void remove( size_t idx ){
                if( idx < m_dcrtes__.size() ){
                    m_dcrtes__.erase( m_dcrtes__.begin() + idx );
				}
			}
			/**
			* @brief 使用默认接口方式试下配合使用的装饰执行函数
			*/
			template< typename ...Params >
			void decratMe( Params&& ...args ){
				for( size_t i = 0; i < m_dcrtes__.size(); ++i ){
                  m_dcrtes__[i]->operation( std::forward<Params>( args )... );
				}
			}
			/**
			 * @brief 实现具体业务的接口，在回调函数中执行装饰任务
			 * @param cb[ I ]回调函数， void fun( decrat_t& item );
			 *          
			 * @note 这个接口会使用openomp的方式并行执行，如果在业务程序中存在互斥数据应该自行处理好
			 */
            void decratMeCallback( std::function< void (dcrte_t & ) > cb ){
                size_t count = m_dcrtes__.size();
			
				for( size_t i = 0; i < count; i ++ ){
                    cb( m_dcrtes__[ i ] );
				}
			}

            void decratMeCallback( std::function< void (itfc_t * itfc ) > cb ){
                size_t count = m_dcrtes__.size();
			
				for( size_t i = 0; i < count; i ++ ){
                    cb( m_dcrtes__[ i ].get() );
				}
			}
		
            iterator begin(){ return m_dcrtes__.begin(); }
            iterator end(){ return m_dcrtes__.end(); }
		};
	}
}
