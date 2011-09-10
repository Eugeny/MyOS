#ifndef UTIL_SINGLETON_H
#define UTIL_SINGLETON_H

template<class T>
class Singleton
{
public:
    static T* get() {
        if (!_instance)
            _instance = new T();
        return _instance;
    }

protected:
    static T* _instance;
    Singleton(){}
    ~Singleton(){}

private:
    Singleton(const Singleton &);
    Singleton &operator = (const Singleton &);
};

template<class T>
T* Singleton<T>::_instance = 0;
#endif 
