/**
 * @brief 装饰器模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once

#include <type_traits>
#include <vector>
#include <omp.h>
namespace wheels
{
	template< typename itfcType >
	class decorator
	{		
	public:
		/**
		 * @brief 装饰品包装，类似与化妆品包装盒
		 */
		template< typename itfcType >
		class decoratee 
		{
		protected:
			itfcType  * __p_imp;     // 实际的装饰品，类似实际的化妆品
		public:
			decoratee():__p_imp( nullptr ){}
			decoratee( itfcType * imp ):__p_imp( imp ){}
			decoratee( itfcType&& b ): __p_imp( b.__p_imp ){}

			decoratee& operator=( itfcType&& b ){
				__p_imp = b.__p_imp;
				return *this;
			}
			
			itfcType * operator->(){ return __p_imp; }

			void set( itfcType * imp ){ __p_imp = imp; }
			itfcType * get(){ return __p_imp; }
		};

		// 装饰品数据类型包装数据类型
		using dcrte_t = decoratee< itfcType > ;
		// 装饰品
		using data_t = std::vector< dcrte_t >;
		using iterator = data_t :: iterator;
	protected:
		data_t __m_dcrtes;
	public:
		decorator(){}
		virtual ~decorator(){}

		/**
		 * @brief 添加装饰品.
		 * @param dcrtee[ I ], 装饰品对象
		 * @return 装饰id,也就是索引号。
		 * @note 添加装饰的时候返回的是其索引，如果后续修改了需要同步操作
		 */
		size_t decrat( decrat_t& dcrtee ){
			__m_dcrtes.push_back( decrat_t );
			return __m_dcrtes.size() - 1;
		}
		/**
		 * @brief 通过接口类型添加装饰品
		 * @param dcrtee
		 * @return
		 */
		size_t decrat( itfcType * dcrtee ) {
			__m_dcrtes.push_back( decrat_t( dcrtee ) );
			return __m_dcrtes.size() - 1;
		}
			
		/**
		 * @brief 移除装饰品
		 */
		void remove( size_t idx ){
			if( idx < __m_dcrtes.size() ){
				__m_dcrtes.erase( idx );
			}
		}
		/**
		 * @brief 实现具体业务的接口，在回调函数中执行装饰任务
		 * @param cb[ I ]回调函数， void fun( decrat_t& item );
		 *          
		 * @note 这个接口会使用openomp的方式并行执行，如果在业务程序中存在互斥数据应该自行处理好
		 */
		void decratMe( std::functional< void (dcrte_t & ) > cb ){
			size_t count = __m_dcrtes.size();
			
                        #pragma omp parallel for
			for( size_t i = 0; i < count; i ++ ){
				cb( __m_dcrtes[ i ] );
			}
		}

		void decratMe( std::functional< void (itfcType * itfc ) > cb ){
			size_t count = __m_dcrtes.size();
			
                        #pragma omp parallel for
			for( size_t i = 0; i < count; i ++ ){
				cb( __m_dcrtes[ i ] );
			}
		}
		
	        iterator begin(){ return __m_dcrtes.begin(); }
		iterator end(){ return __m_dcrtes.end(); }
	};
};
