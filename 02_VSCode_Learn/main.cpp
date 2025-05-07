#include "Object.h"
#include <iostream>
#include <vector>

int main() {
    std::vector<std::string> arr{"abc", "bcd", "sdf", "fgk", "gkj"};
    for(std::string &item: arr){
        std::cout << item << std::endl;
    }

    Object obj("SampleObject");
    obj.printName();  // 输出: Object Name: SampleObject

    return 0;
}