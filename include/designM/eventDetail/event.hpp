/**
 * @brief 事件对象
 */

#pragma once

#include <type_traits>
#include <queue>
#include <memory>

#include "container/variant.hpp"

namespace wheels
{
	namespace dm
	{
		template< typename idType >
		struct eventData {
			idType                   __m_id;
			wheels::variant          __m_data;

			eventData(){}
			template< typename dataType >
			eventData( const idType& evt , const dataType& data ):
				__m_id( evt ) , __m_data( wheels::variant::make( data ) ){}
			eventData( const eventData& b ):
				__m_id( b.__evt ) , __m_data( b.__m_data ){}

			eventData& operator=( const eventData& b ){
				__m_id = b.__m_id;
				__m_data = b.__m_data;

				return *this;
			}
		};

	}
}


