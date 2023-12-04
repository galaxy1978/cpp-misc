/**
 * @brief 装饰器模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once

#include <type_traits>
#include <vector>

namespace wheels
{
	namespace dm
	{
		template< typename itfcType >
		class decorator
		{		
		public:
			template< typename itfcType >
			class decoratee 
			{
			public:
				imp_ptr = std::shared_ptr< itfcType >;
			protected:
				imp_ptr pt_imp__;  
			public:
				decoratee():pt_imp__( nullptr ){}
				decoratee( imp_ptr imp ):pt_imp__( imp ){}

				decoratee& operator=( itfcType& b ){
					pt_imp__ = b.pt_imp__;
					return *this;
				}
			
				imp_ptr operator->(){ return pt_imp__; }
				itfcType& operator*(){ return *pt_imp__; }
				void set( imp_ptr imp ){ pt_imp__ = imp; }
				imp_ptr get(){ return pt_imp__; }
			};

			using dcrte_t = decoratee< itfcType > ;
			using data_t = std::vector< dcrte_t >;
			using iterator = data_t :: iterator;
		protected:
			data_t m_dcrtes__;
		public:
			decorator(){}
			virtual ~decorator(){}

			size_t decrat( decrat_t& dcrtee ){
				m_dcrtes__.push_back( decrat_t );
				return m_dcrtes__.size() - 1;
			}

			size_t decrat( itfcType * dcrtee ) {
				m_dcrtes__.push_back( decrat_t( dcrtee ) );
				return m_dcrtes__.size() - 1;
			}
			
			void remove( size_t idx ){
				if( idx < m_dcrtes__.size() ){
					m_dcrtes__.erase( idx );
				}
			}

			void decratMe( std::functional< void (dcrte_t & ) > cb ){
				size_t count = m_dcrtes__.size();
			
				for( size_t i = 0; i < count; i ++ ){
					cb( m_dcrtes__[ i ] );
				}
			}

			void decratMe( std::functional< void (itfcType * itfc ) > cb ){
				size_t count = m_dcrtes__.size();
				for( size_t i = 0; i < count; i ++ ){
					cb( m_dcrtes__[ i ].get().get() );
				}
			}
		
			iterator begin(){ return m_dcrtes__.begin(); }
			iterator end(){ return m_dcrtes__.end(); }
		};
	}
}
