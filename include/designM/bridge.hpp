/**
 * @brief 桥接模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-23
 */

#pragma once
#include <type_traits>
#include <iostream>
namepace wheels
{
	namespace dm
	{
		template< typename itfcType , typename implType >
		class bridge
		{
			// 实际实现类必须是接口类的子类
			static_assert( std::is_base_of<itfcType , implType >::value, "Interface type must be base class of implemention class." );
		public:
			enum emErrCode{
				ERR_IMP_NULL = -1,
				OK = 0
			};
		private:
			itfcType	* __p_imp;
		public:
			bridge(){}
			template< typename ...Args >
			bridge(Args... args ){
				__p_imp = new implType( (0,args)...);		
			}
	
			virtual ~bridge(){
				if( __p_imp ){
					delete __p_imp;
				}
			}
			
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
				if( __p_imp == nullptr ) throw ERR_IMP_NULL;
				return __p_imp;
			}

			implType& operator*(){
				if( __p_imp == nullptr ) throw ERR_IMP_NULL;
				return *(implType*)__p_imp;
			}
	
		};
	}
}
