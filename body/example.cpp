#include "..\head\list.hpp" 

int main() {
    MyList<int> lst;
    lst.push_front(1);
    lst.push_front(2);
    for (int x : lst) std::cout << x << " ";
}