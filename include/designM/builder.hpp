namespace wheels{ namespace dm {
template< typename... Parts > class product{};

template< typename Part1 , typename... Parts > 
class product<Part1, Parts...> : public Part1 , public product< Parts... >
{
public:
	virtual ~product(){}
};



template<> class product<>{};

/**
 * @brief 
 * @tparam Parts ，组件表
*/
template< typename... Parts >
class director 
{
public:
	using product_t = product< Parts... >;

public:
	template< typename... Params >
	static  std::shared_ptr< product_t > build( std::function< void ( std::shared_ptr< product_t > ) >fun ){
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