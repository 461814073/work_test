#ifndef SINGLETON_H
#define SINGLETON_H

#include <stdio.h>
#include <utility>

template <typename T>
class Singleton
{
public:
    Singleton()
    {

    }

    // static T* c_instance;
public:
    template<typename ... Args>
    static T* GetInstance(Args&&... args)
    {
        static T instance(std::forward<Args>(args)...);
        return &instance;
    }
};

#endif // SINGLETON_H
