// 这段示例程序已经在gcc和vs中测试通过
// @Leezheng1提交的bug测试，并提交了示例代码。感谢@Leezheng1无私奉献。

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include "designM/mediator.hpp"

using namespace wheels::dm;

class MyColleague;
using mediator_t = wheels::dm::mediator< MyColleague , std::string >;

// 定义一个类，继承自colleagueItfc接口
class MyColleague : public wheels::dm::colleagueItfc<mediator_t, std::string> {
public:
	MyColleague(const std::string& name , std::shared_ptr<mediator_t> ptr ) : colleagueItfc(ptr) , name_(name) {}

	void recv(const std::tuple<std::string>& msg) override {
		std::cout << name_ << " received: " << std::get<0>(msg) << std::endl;
	}

private:
	std::string name_;
};

int main() {
	auto med = std::make_shared<mediator_t>();

	auto col1 = std::make_shared<MyColleague>("Colleague1", med );
	auto col2 = std::make_shared<MyColleague>("Colleague2", med );
	auto col3 = std::make_shared<MyColleague>("Colleague3", med );

	med->add(col1);
	med->add(col2);
	med->add(col3);
	
	med->run( true );
	
	col3->send( "Hello, everyone!");
	col2->sendTo(col3, std::string( "Hello, Colleague3!" ));

	std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
	med->run( false );
	return 0;
}