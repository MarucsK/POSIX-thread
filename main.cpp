#include "thread.h"

void func(int &a)
{
    std::cout << &a << '\n';
}
void func2(const int &a)
{
    std::cout << &a << '\n';
}
struct X
{
    void f() { std::cout << "X::f\n"; }
};

int main()
{
    std::cout << "main thread id: " << Marcus::this_thread::get_id() << '\n';

    int a = 10;
    std::cout << &a << '\n';
    Marcus::thread t{func, std::ref(a)};
    t.join();

    Marcus::thread t2{func2, a};
    t2.join();

    Marcus::thread t3{[]
                      { std::cout << "thread id: " << Marcus::this_thread::get_id() << '\n'; }};
    t3.join();

    X x;
    Marcus::thread t4{&X::f, &x};
    t4.join();

    Marcus::thread{[]
                   { std::cout << "👉🤣\n"; }}
        .detach();
    sleep(1);
}
