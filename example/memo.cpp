#include "memo.hpp"
int main() {
    using crtk = caretaker<int>;
    crtk caretaker;
    crtk::orgnt_t originator;

    // 设置初始状态
    int state = 5;
    originator.setState(&state);

    std::cout << "初始状态: " << *originator.getState() << std::endl;

    // 创建备忘录并保存到caretaker中
    caretaker.add(originator.createMemento());

    // 修改状态
    *originator.getState() = 10;
    std::cout << "修改后的状态: " << *originator.getState() << std::endl;

    // 使用备忘录恢复原始状态
    originator.restoreMemento(caretaker.get(0));
    std::cout << "恢复后的状态: " << *originator.getState() << std::endl;

    return 0;
}