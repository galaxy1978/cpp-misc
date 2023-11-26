/**
 * @brief 建造者模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-4~2023-11-26
 *
 * 2023-11-26 增加std::tuple支持，改善名字冲突的问题
*/

#pragma once
namespace wheels{ namespace dm {
#if BUILD_USE_TUPLE == 1
#include <tuple>
template< itfcType , typename ...Parts >
class product
{
protected:
	std::tuple< Parts... >   m_parts__;
public:
	virtual ~product(){}
	
	template< int N >
	auto get()->decltype(typename std::tuple_element< N , prdtTypes >::type){
		return std::get< N >( m_parts__ );
	}
	
	template< int idx , typename PART_TYPE >
	void set( PART_TYPE param ){
		std::get< idx >( m_parts__ ) = param;
	}
};
#else
template< typename itfcType , typename... Parts > class product{};

template< typename itfcType , typename Part1 , typename... Parts > 
class product<itfcType , Part1 , Parts... > : public Part1 , public product< itfcType , Parts... >
{
public:
	virtual ~product(){}
};


template< typename itfcType > 
class product<itfcType>: public itfcType{
public:
	virtual ~product(){}
};
#endif
/**
 * @brief 
 * @tparam Parts ，组件表
*/
template< typename itfcType , typename... Parts >
class director 
{
public:
	using product_t = product< itfcType , Parts... >;

public:
	template< typename... Params >
	static  std::shared_ptr< product_t > 
	build( std::function< void ( std::shared_ptr< product_t > ) >fun ){
		std::shared_ptr< product_t >   pt_product;
		try{
			pt_product.reset( new product_t );
			if( fun ){
				fun( pt_product );
			}
		}catch( std::bad_alloc& e ){
			std::cout << e.what() << std::endl;		
			if( pt_product ){
				pt_product.reset(  );
			}
		}catch( std::runtime_error& e  ){
			std::cout << e.what() << std::endl;
			if( pt_product ){
				pt_product.reset( );
			}
		}
		
		return pt_product;
	}
};
}}