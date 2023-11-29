/**
 * @brief 桥接模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once
#include <type_traits>
#include <memory>
#include <iostream>
namespace wheels
{
	namespace dm
	{
		template< typename itfcType , typename implType >
		class bridge
		{
        public:
            using itfc_t = typename std::remove_pointer< typename std::decay< itfcType >::type >::type;
            using impl_t = typename std::remove_pointer< typename std::decay< implType >::type >::type;
			// 实际实现类必须是接口类的子类
			static_assert( std::is_base_of<itfcType , implType >::value, "Interface type must be base class of implemention class." );
		public:
			enum emErrCode{
				ERR_IMP_NULL = -1,
				OK = 0
			};
		private:
            std::shared_ptr< itfc_t > pt_imp__;
		public:
			bridge(){}
			template< typename ...Args >
            bridge(Args&&... args ){
                pt_imp__ = std::make_shared< impl_t >( std::forward< Args >(args)...);
			}
	
            virtual ~bridge(){}
			
			template< typename ...Args >
			static bridge * create( Args... args ){
				try{
					bridge * ret = new bridge( (0,args)... );
					return ret;
				}catch( std::bad_alloc& e ){
					std::cout << e.what();
				}
				
				return nullptr;
			}

			/**
			 * @brief 重载方便访问具体实现的接口。
			 * @exception 如果实现对象指针为空，则抛出异常
			 */
			itfcType * operator->(){
                if( pt_imp__ == nullptr ) throw ERR_IMP_NULL;
                return pt_imp__;
			}

			implType& operator*(){
                if( pt_imp__ == nullptr ) throw ERR_IMP_NULL;
                return *(implType*)pt_imp__;
			}
	
		};
	}
}
