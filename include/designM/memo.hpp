/**
 * @brief 备忘录模式
 * @version 1.0
 * @author 宋炜
 */

#include <type_traits>
#include <vector>

namespace wheels
{
	namespace dm
	{

		namespace private__{
			template<typename T>
			class memento {
				static_assert( std::is_copy_constructible< T >::value , "必须支持拷贝构造" );
				static_assert( std::is_copy_assignable< T >::value , "必须支持赋值拷贝操作" );
			public:
				Memento(const T& state) : m_state__(state) {}

				T getState() const {
					return state;
				}

				T * operator->(){ return &m_state__;}
				T& operator*(){ return m_state__; }
			private:
				T m_state__;
			};

			template<typename T>
			class originator {
			public:
				void setState(const T& state) {
					this->state = state;
				}

				T getState() const {
					return state;
				}

				Memento<T> createMemento() const {
					return Memento<T>(state);
				}

				void restoreMemento(const Memento<T>& memento) {
					this->state = memento.getState();
				}

				T * operator->(){ return &m_state__;}
				T& operator*(){ return m_state__; }

			private:
				T state;
			};
		}

		template<typename T>
		class caretaker {
		public:
			using stat_t = typename std::remove_pointer< typename std::decay< T >::type >::type;
			
			using memo_t = private__::memento< stat_t >;
			using orgnt_t = private__::originator< stat_t >;
		public:
			void add(const memo_t& memento) {
				m_mementos__.push_back(memento);
			}

			memo_t get(int index) const {
				return m_mementos__[index];
			}

			void remove( int idx ){
				if( idx < m_mementos__.size() ){
					m_mementos__.erase( idx );
				}
			}

			void clear(){
				m_mementos__.erase( m_mementos__.begin() , m_mementos__.end() );
			}

			memo_t& operator[]( int idx ){
				return m_mementos__[ idx ];
			}
		private:
			std::vector<memo_t> m_mementos__;
		};

	}

}
