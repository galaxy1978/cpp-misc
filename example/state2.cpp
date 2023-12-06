#include <iostream>
#include <functional>
#include "designM/state.hpp"
#include "designM/strategy.hpp"

using namespace wheels;
using namespace dm;

enum class MyState {
	STATE_Start,
	STATE_A,
	STATE_B,
	STATE_C,
	STATE_End,
};

const int StartToA = 0;
const int AToB = 1;
const int BToC = 2;
const int CTOEND = 3;

int main() {
	using fsm_t = wheels::dm::state<MyState, int>;
	using event_t = fsm_t::evtType_t;
	using strategy_t = strategy< MyState , std::function< void ( MyState , event_t , int ) > >;
	fsm_t fsm;
	strategy_t branch_caller;

	branch_caller.add( MyState::STATE_Start , []( MyState state , event_t event, int  data){
		std::cout << "STATE_Start: ";
		if (event == event_t::EVT_ENT) {
			
		}
	} );

	branch_caller.add( MyState::STATE_A , [&]( MyState state , event_t event, int  data){
		std::cout << "STATE_A: ";
		if (event == event_t::EVT_ENT) {
			std::cout << "(DoSomeThing)";
			fsm.execute(AToB);
		}
	} );

	branch_caller.add( MyState::STATE_B , [&]( MyState state , event_t event, int  data){
		std::cout << "State B: ";
		if (event == event_t::EVT_ENT) {
			std::cout << "(DoSomeThing)";
			fsm.execute(BToC);
		}
	} );

	branch_caller.add( MyState::STATE_C , [&]( MyState state , event_t event, int  data){
		std::cout << "State C: ";
		if (event == event_t::EVT_ENT) {
			std::cout << "(DoSomeThing)";
			fsm.execute(CTOEND);
		}
	} );

	branch_caller.add( MyState::STATE_End, [&]( MyState state , event_t event, int  data){
		std::cout << "STATE_End: ";
	} );

	// 添加状态
	fsm.addState(MyState::STATE_Start);
	fsm.addState(MyState::STATE_A);
	fsm.addState(MyState::STATE_B);
	fsm.addState(MyState::STATE_C);
	fsm.addState(MyState::STATE_End);

	// 添加状态转换
	fsm.addArc(MyState::STATE_Start, MyState::STATE_A, [](const int& condition) { return condition == StartToA; });
	fsm.addArc(MyState::STATE_A, MyState::STATE_B, [](const int& condition) { return condition == AToB; });
	fsm.addArc(MyState::STATE_B, MyState::STATE_C, [](const int& condition) { return condition == BToC; });
	fsm.addArc(MyState::STATE_C, MyState::STATE_End, [](const int& condition) { return condition == CTOEND; });

	// 设置起始和结束状态
	fsm.setStart(MyState::STATE_Start);
	fsm.setEnd(MyState::STATE_End);

	// 注册状态转换动作函数
	fsm.on([&](MyState state, event_t event, int condition) {
		// 执行分支
		branch_caller.call( state , state , event , condition );
		
		switch (event) {
		case event_t::EVT_ENT:
			std::cout << "EVT_ENT \n";
			break;
		case event_t::EVT_LEAVE:
			std::cout << "EVT_LEAVE \n";
			break;
		case event_t::EVT_READY: // 模块就绪事件
			std::cout << "EVT_READY \n";
			break;
		case event_t::EVT_END:
			std::cout << "EVT_END \n";
			break;
		}
	});


	// 启动状态机
	fsm.start(true);

	// 执行状态转换
	fsm.execute(StartToA);

	std::this_thread::sleep_for(std::chrono::seconds(1));
	fsm.start(false);
	return 0;
}
