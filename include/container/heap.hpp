/**
 * @brief 堆的实现
 * @version 1.0
 * @author 宋炜
 */

#pragma once

#include <typte_traits>
#include <vector>

namespace wheels { namespace container{
		
template<typename Key, typename Value >
class heap {
private:
	struct Node {
		Key key;
		Value value;
	};
private:
	std::vector<Node> m_data__;
	Compare cmp__;

public:
	using key_type = typename std::remove_pointer< typename std::decay< Key >::type >::type;
	using value_type = typename std::remove_pointer< typename std::decay< Value >::type >::type;
	using Compare = std::less< key_type >;
	using iterator = std::value< Node >::iterator;
public:
	heap() {}

	virtual ~heap(){}
	/**
	 * @brief 插入数据
	 */
	void insert(const Key& key, const Value& value) {
		Node node{key, value};
		m_data__.push_back(node);
		std::push_heap(m_data__.begin(), m_data__.end(), cmp__);
	}
	/**
	 * @brief 
	 */
	std::pair<Key, Value> pop() {
		std::pop_heap(m_data__.begin(), m_data__.end(), cmp__);
		Node top_node = m_data__.back();
		m_data__.pop_back();
		return std::make_pair(top_node.key, top_node.value);
	}


	bool empty() const {
		return m_data__.empty();
	}

	size_t size() const{ return m_data__.size(); }
	/**
	 * @brief 删除
	 */
	void erase(const Key& key) {
		auto it = std::find_if(m_data__.begin(), m_data__.end(), [key](const Node& node) {
			return node.key == key;
		});
		if (it != m_data__.end()) {
			std::swap(*it, m_data__.back());
			m_data__.pop_back();
			std::make_heap(m_data__.begin(), m_data__.end(), cmp__);
		}
	}

	const iterator find(const Key& key) const {
		for( auto it = m_data__.begin(); it != m_data__.end(); ++it ){
			if( it->key == key ) return it;
		}

		return m_data__.end();
	}

	bool has( const Key& key ){
		for( auto it = m_data__.begin(); it != m_data__.end(); ++it ){
			if( it->key == key ) return true;
		}

		return false;
	}

	iterator begin(){ return m_data__.begin(); }
	iterator end(){ return m_data__.end(); }
};
	
}}
