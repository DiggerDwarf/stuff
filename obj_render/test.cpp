#include <iostream>
#include <list>


int main(int argc, char const *argv[])
{
    std::list<int> vec;

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);

    std::list<int>::iterator it = vec.begin()++;
    it ++;
    it ++;

    int* save = &*it;

    std::cout << *save << "\n";

    it = vec.begin()++;

    vec.erase(it);

    std::cout << *save << "\n";

    return 0;
}
