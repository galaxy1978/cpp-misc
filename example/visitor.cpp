#include <iostream>
#include <string>
#include <vector>

#include "designM/vistor.hpp"

using namespace wheels;
using namespace dm;
// 自定义数据类型
struct Data {
  Data(int value) : value(value) {}
  int value;
};

// 处理函数1
int func1(Data& data) {
  std::cout << "func1 called with value: " << data.value << std::endl;
  return data.value + 1;
}

// 处理函数2
int func2(Data& data) {
  std::cout << "func2 called with value: " << data.value << std::endl;
  return data.value * 2;
}

int main() {
  // 创建vistor对象，并指定数据类型为Data，返回类型为int
  vistor<Data, int> myVistor;

  // 添加方法到vistor对象
  myVistor.addMethod("func1", func1);
  myVistor.addMethod("func2", func2);

  // 创建数据对象
  Data myData(10);

  // 调用指定名称的方法处理数据
  int result1 = myVistor["func1"](myData);
  std::cout << "Result1: " << result1 << std::endl;

  int result2 = myVistor["func2"](myData);
  std::cout << "Result2: " << result2 << std::endl;

  // 批量处理数据
  std::vector<Data> dataVec = {Data(20), Data(30)};
  myVistor.callEach("func1", dataVec.begin(), dataVec.end());

  return 0;
}
