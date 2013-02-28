#include <string.h>
#include <kutil.h>


#ifndef LANG_POOL_H
#define LANG_POOL_H


template<typename T, int size>
class Pool
{
public:
    Pool() {
        capacity = size;
//for(;;);
  //      memset(items, 0, sizeof(T*) * size);
    }

    ~Pool () {
        delete items;
    }

    void add(T item) {
        for (int i = 0; i < size; i++)
            if (!items[i]) {
                items[i] = item;
                return;
            }
    }

    void remove(T item) {
        for (int i = 0; i < size; i++)
            if (items[i] == item)
                items[i] = NULL;
    }
    
    T operator[] (int index) {
        return items[index];
    }

    class iterator {
    public:
        iterator(Pool<T, size>* p, int pos) {
            pool = p;
            index = pos;
        }

        operator T&() {
            return pool->items[index];
        }

        T& operator*() const {
            return pool->items[index];
        }

        const iterator& operator++() {
            do {
                index++;
            } while (!((*pool)[index]) && (index < size));
            return *this;
        }

        bool operator==(iterator i) {
            return i.index == index;
        }

        bool operator!=(iterator i) {
            return i.index != index;
        }
    protected:
        Pool<T, size>* pool;
        int index;
    };

    iterator begin() {
        iterator i(this, -1);
        ++i;
        return i;
    }

    iterator end() {
        return iterator(this, size);
    }

    int capacity;
protected:
    T items[size];

};

template<typename T, int size>
typename Pool<T, size>::iterator begin(Pool<T, size> p) {
    return p.begin();
}

template<typename T, int size>
typename Pool<T, size>::iterator end(Pool<T, size> p) {
    return p.end();
}

#endif
