#include "vector.h"
#include <iostream>

int main() {
  rainLyn::vector<int> arr;
  for (int i = 0; i < 5; i++) {
    arr.push_back(i);
    std::cout << arr[i] << " ";
  }
  std::cout << std::endl;

  auto n = arr.emplace_back(20);
  for (auto i = arr.begin(); i < arr.end(); i++) {
    std::cout << *i << " ";
  }
  std::cout << std::endl;
  std::cout << "n: " << n << std::endl;
}
