/**
 * @brief 堆的实现
 * @version 1.0
 * @author 宋炜
 */

#pragma once

#include <typte_traits>
#include <vector>

namespace wheels { namespace container{
		
template<typename Key, typename Value , bool isLess = true >
class heap {
private:
	struct Node {
		Key key;
		Value value;

		bool operator<( const Node& b ){ return key < b.key; }
		bool operator>( const Node& b ){ return key > b.key; }
	};
	using Compare = typename std::conditional< isLess , std::less< Node > , std::greater< Node > >::type;
private:
	std::vector<Node> m_data__;
	Compare cmp__;
public:
	using key_type = typename std::remove_pointer< typename std::decay< Key >::type >::type;
	using value_type = typename std::remove_pointer< typename std::decay< Value >::type >::type;
	
	using iterator = std::vector< Node >::iterator;
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
	 * @brief 弹出堆最后一个元素，并返回其内容
	 * @return std::pair<Key, Value>
	 */
	std::pair<Key, Value> pop() {
		std::pop_heap(m_data__.begin(), m_data__.end(), cmp__);
		Node top_node = m_data__.back();
		m_data__.pop_back();
		return std::make_pair(top_node.key, top_node.value);
	}
	
	/**
	 * @brief 判断是否为空堆
	 * @return 如果为空返回true，否则返回false
	 */
	bool empty() const {
		return m_data__.empty();
	}
	/**
	 * @brief 返回堆大小
	 */
	size_t size() const{ return m_data__.size(); }
	/**
	 * @brief 删除指定的元素
	 * @param key[ I ]， key值
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
	/**
	 * @brief 查找指定key的内容
	 * @param key[ I ],要查找的键
	 * @return 返回对应的迭代器
	 */
	iterator find(const Key& key) {
		for( auto it = m_data__.begin(); it != m_data__.end(); ++it ){
			if( it->key == key ) return it;
		}

		return m_data__.end();
	}
	/**
	 * @brief 判断是否存在key为指定内容的数据
	 * @return key[ I ]， 要判断的键
	 */
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
