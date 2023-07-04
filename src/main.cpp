#include <unistd.h>
#include <iostream>
#include <functional>
#include "../include/thread.hpp"

int main(int argc, char const *argv[])
{
    int twelve = 12;
    int five = 5;

    Thread<long> first_thread([twelve] {
        usleep(2 * 1000 * 1000);
        return twelve;
    });

    Thread<long> second_thread([five] {
        usleep(3 * 1000 * 1000);
        return five;
    });

    long first_output = first_thread.join();
    long second_output = second_thread.join();
    std::cout << (first_output + second_output) << std::endl;
}
