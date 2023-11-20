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
	/**
	 * @brief 添加处理方法
	 * @param name[ I ], 方法名称
	 * @param func,方法函数对象
	 * @return 如果成功添加方法，返回true，否则返回false
	*/
    bool addMethod( const std::string& name , func_t func ){
		auto rst = m_funcs__.insert( std::make_pair( name , func ) );
		return rst.second;
	}
	/**
	 * @brief 移除方法
	 * @param 方法名称
	 * @return 如果没有找到方法，返回false,否则删除方法返回true
	*/
    bool eraseMethod( const std::string& name ){
		auto it = m_funcs__.find( name );
		if( it ){
			m_funcs__.erase( it );
			return true;
		}
		
		return false;
	}
	/**
	 * @brief 这是一个方便操作接口，可以用过 vistor["abc"]( data )调用处理数据
	*/
	func_t operator[]( const std::string& name ){
		auto it = m_funcs__.find( name );	
		if( it ){
			return it->second;
		}
		return {};
	}
	/**
	 * @brief 调用方法处理数据
	 * @param name[ I ], 方法名称
	 * @param 要处理的数据
	*/
	RET call( const std::string& name, dataItem_t& data ){
		auto it = m_funcs__.find( name );	
		if( it ){
			return it->second( data );
		}
		return {};
	}
	/**
	 * @brief 调用方法处理批量数据
	 * @param name[ I ], 方法名称
	 * @param 要处理的数据
	*/
	template< typename InputIterator >
	void callEach( const std::string& name , InputIterator begin , InputIterator end ){
		auto it = m_funcs__.find( name );
		
		for( auto it1 = begin; it1 != end; ++it1 ){
			it->second( *it1 );
		}
	}
	/**
	 * @brief 检查是否存在指定的方法
	*/
	bool has( const std::string& name ){
		auto it = m_funcs__.find( name );	
		return it != m_funcs__.end();
	}
};
}}