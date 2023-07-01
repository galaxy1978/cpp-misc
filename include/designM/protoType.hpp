/**
 * @brief 原型模式实现
 * @author 宋炜
 * @version 1.0
 * @date 2023-7-1
*/

#pragma once

template< typename T,
		  typename midType = typename std::decay< T >::type ,
		  typename midType2 = typename std::remove_pointer< midType >::type,
		  typename realType = typename std::enable_if< 
									std::is_copy_constructible<midType2>::value && std::is_class< midType2 >::value , midType2 
							>::type
        >
struct protoType
{
public:
	realType * clone( const realType& old ) const {
		realType * ret = nullptr;
		try{
			ret = new realType( old );
		}catch( std::bad_alloc& e ){
			std::cout << e.what() << std::endl;
		}
		
		return ret;
	}
}

template< typename T ,
		  typename midType = typename std::decay< T >::type ,
		  typename midType2 = typename std::remove_pointer< midType >::type,
		  typename realType = typename std::enable_if< 
									std::is_copy_constructible<midType2>::value && std::is_class< midType2 >::value , midType2 
							>::type>
realType * clone( const realType& old )
{
	realType * ret = nullptr;
		
	try{
		ret = new realType( old );
	}catch( std::bad_alloc& e ){
		std::cout << e.what() << std::endl;
	}
		
	return ret;
}