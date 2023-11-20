/**
 * @brief 访问者模式
 * @versioin 1.0
 * @author 宋炜
*/
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <string>
namespace wheels{namespace dm{
template< typename DATA_ITFC , typename RET >
class vistor
{
public:
	using dataItem_t  = typename std::remove_pointer< typename std::decay< DATA_ITFC >::type >::type;
	using func_t = std::function< RET ( dataItem_t& ) >;
	using funcTan_t = typename std::unordered_map< std::string , func_t >;
protected:
	funcTan_t    m_funcs__;
public:
    vistor(){}
    virtual ~vistor(){}
	
    bool addMethod( const std::string& name , func_t func ){
		auto rst = m_funcs__.insert( std::make_pair( name , func ) );
		return rst.second;
	}
	
    bool eraseMethod( const std::string& name ){
		auto it = m_funcs__.find( name );
		if( it ){
			m_funcs__.erase( it );
			return true;
		}
		
		return false;
	}
	
	RET call( const std::string& name, dataItem_t& data ){
		auto it = m_funcs__.find( name );	
		if( it ){
			return it->second( data );
		}
		
		return {};
	}
	
	template< typename InputIterator >
	void callEach( const std::string& name , InputIterator begin , InputIterator end ){
		auto it = m_funcs__.find( name );
		
		for( auto it1 = begin; it1 != end; ++it1 ){
			it->second( *it1 );
		}
	}
};
}}