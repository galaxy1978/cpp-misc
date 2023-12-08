#include <iostream>

#include "designM/memo.hpp"

using namespace wheels;
using namespace dm;

int main() {
    using crtk = caretaker<int>;
    crtk caretaker;
    crtk::orgnt_t originator;

    // 设置初始状态
    int state = 5;
    originator.setState(state);

    std::cout << "init state: " << originator.getState() << std::endl;

    // 创建备忘录并保存到caretaker中
    caretaker.add(originator.createMemento());

    // 修改状态
    originator.getState() = 10;
    std::cout << "updated state: " << originator.getState() << std::endl;

    // 使用备忘录恢复原始状态
    originator.restoreMemento(caretaker.get(0));
    std::cout << "restored: " << originator.getState() << std::endl;

    return 0;
}
