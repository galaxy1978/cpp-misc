/**
 * @brief 建造者模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-4
*/

#pragma once
namespace wheels{ namespace dm {
template< typename... Parts > class product;

template< typename Part1 , typename... Parts > class product : public Part1 , public product< Parts >
{
public:
	virtual ~product(){}
};

template<> product{};

/**
 * @brief 
 * @tparam Parts ，组件表
*/
template< typename... Parts >
class director 
{
public:
	using product_t = product< Parts >

public:
	template< typename... Params >
	static  std::shared_ptr< product_t > build( std::function< void ( std::shared_ptr< product_t > ) >fun ){
		std::shared_ptr< product_t >   pt_product;
		try{
			pt_product.reset( new product );
			if( fun ){
				fun( pt_product );
			}
		}catch( std::bad_alloc& e ){
			std::cout << e.what() << std::endl;		
			if( pt_product ){
				pt_product.reset( nullptr );
			}
		}catch( std::runtime_error& e  ){
			std::cout << e.what() << std::endl;
			if( pt_product ){
				pt_product.reset( nullptr );
			}
		}
		
		return pt_product;
	}
};
}}