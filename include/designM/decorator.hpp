/**
 * @brief 装饰器模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once
#include <type_traits>
#include <vector>
#include <functional>

namespace wheels
{
	namespace dm
	{
		template< typename itfcType >
		class decorator
		{		
		public:
            using itfc_t = typename std::remove_pointer< typename std::decay<itfcType>::type >::type;
			/**
			 * @brief 装饰品包装，类似与化妆品包装盒
			 */
			class decoratee 
			{
			protected:
                itfc_t  * p_imp__;     // 实际的装饰品，类似实际的化妆品
			public:
                decoratee():p_imp__( nullptr ){}
                decoratee( itfcType * imp ):p_imp__( imp ){}
                decoratee( itfcType&& b ): p_imp__( b.p_imp__ ){}

				decoratee& operator=( itfcType&& b ){
                    p_imp__ = b.p_imp__;
					return *this;
				}
			
                itfcType * operator->(){ return p_imp__; }

                void set( itfcType * imp ){ p_imp__ = imp; }
                itfcType * get(){ return p_imp__; }
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
                m_dcrtes__.push_back( decrat_t( dcrtee ) );
                return m_dcrtes__.size() - 1;
			}
			
			/**
			 * @brief 移除装饰品
			 */
			void remove( size_t idx ){
                if( idx < m_dcrtes__.size() ){
                    m_dcrtes__.erase( idx );
				}
			}
			/**
			 * @brief 实现具体业务的接口，在回调函数中执行装饰任务
			 * @param cb[ I ]回调函数， void fun( decrat_t& item );
			 *          
			 * @note 这个接口会使用openomp的方式并行执行，如果在业务程序中存在互斥数据应该自行处理好
			 */
            void decratMe( std::function< void (dcrte_t & ) > cb ){
                size_t count = m_dcrtes__.size();
			
				for( size_t i = 0; i < count; i ++ ){
                    cb( m_dcrtes__[ i ] );
				}
			}

            void decratMe( std::function< void (itfcType * itfc ) > cb ){
                size_t count = m_dcrtes__.size();
			
				for( size_t i = 0; i < count; i ++ ){
                    cb( m_dcrtes__[ i ] );
				}
			}
		
            iterator begin(){ return m_dcrtes__.begin(); }
            iterator end(){ return m_dcrtes__.end(); }
		};
	}
}
