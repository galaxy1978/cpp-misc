#include <iostream>
#include <thread>
#include "designM/state.hpp"

using namespace wheels;
using namespace dm;
enum class MyState {
STATE_A,
STATE_B,
STATE_C
};

int main() {
using fsm_t = wheels::dm::state<MyState, int>;
using event_t = fsm_t::evtType_t;
fsm_t fsm;

// 添加状态
fsm.addState(MyState::STATE_A);
fsm.addState(MyState::STATE_B);
fsm.addState(MyState::STATE_C);

// 添加状态转换
fsm.addArc(MyState::STATE_A, MyState::STATE_B, [](const int& condition) { return condition > 0; });
fsm.addArc(MyState::STATE_B, MyState::STATE_C, [](const int& condition) { return condition > 1; });
fsm.addArc(MyState::STATE_C, MyState::STATE_A, [](const int& condition) { return condition > 2; });

// 设置起始和结束状态
fsm.setStart(MyState::STATE_A);
fsm.setEnd(MyState::STATE_C);

// 注册状态转换动作函数
fsm.on([](MyState state, event_t event, int condition) {
	switch( event ){
	case event_t::EVT_ENT:
		std::cout << "EVT_ENT ";
		break;
	case event_t::EVT_LEAVE:
		std::cout << "EVT_LEAVE ";
		break;
	case event_t::EVT_READY: // 模块就绪事件
		std::cout << "EVT_READY ";
		break;
	case event_t::EVT_END:
		std::cout << "EVT_END ";
		break;
	}
    switch (state) {
    case MyState::STATE_A:
        std::cout << "State A\n";
        break;
    case MyState::STATE_B:
        std::cout << "State B\n";
        break;
    case MyState::STATE_C:
        std::cout << "State C\n";
        break;
    }
    });

// 启动状态机
fsm.start(true);

// 执行状态转换
fsm.execute(1);
fsm.execute(2);
fsm.execute(3);

std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
fsm.start( false );
return 0;