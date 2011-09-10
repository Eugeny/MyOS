#ifndef UTIL_LINKEDLIST_H
#define UTIL_LINKEDLIST_H

class LinkedListEntry<T> {
public:
    T value;
    LinkedListEntry<T>* next;
};

class LinkedListIter<T> {
public:
    LinkedListIter<T>(LinkedList<T>* l) {
        c = l->getRoot();
    }
    
    void next() {
        c = c->next;
    }
    
    int end() {
        return (c == 0) or (c->next == 0);
    }
    
    T get() {
        return c->value;
    }
private:
    LinkedListEntry<T>* c;    
};

class LinkedList<T> {
public:
    LinkedList<T>() {
        root = 0;
    }
    
    LinkedListEntry<T> getRoot() {
        return root;
    }
    
    T get(int idx) {
        LinkedListEntry<T>* p = root;
        for (int i = 0; i < idx; i++)
            p = p->next;
        return p->value;
    }

    T insert(T val, int idx) { // TODO
        LinkedListEntry<T>* p = root;
        for (int i = 0; i < idx; i++)
            p = p->next;
        return p->value;
    }

    
private
    LinkedListEntry<T>* root;    
};


#endif 
