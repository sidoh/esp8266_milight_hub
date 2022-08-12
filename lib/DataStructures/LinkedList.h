/*
  ********* Adapted from: *********
  https://github.com/ivanseidel/LinkedList
  Created by Ivan Seidel Gomes, March, 2013.
  Released into the public domain.
  *********************************

  Changes:
    - public access to ListNode (allows for splicing for LRU)
    - doubly-linked
    - remove caching stuff in favor of standard linked list iterating
    - remove sorting
*/

#ifndef LinkedList_h
#define LinkedList_h

#include <stddef.h>

template<class T>
struct ListNode {
  T data;
  ListNode<T> *next;
  ListNode<T> *prev;
};

template <typename T>
class LinkedList {

protected:
  size_t _size;
  ListNode<T> *root;
  ListNode<T>  *last;

public:
  LinkedList();
  ~LinkedList();

  /*
    Returns current size of LinkedList
  */
  virtual size_t size() const;
  /*
    Adds a T object in the specified index;
    Unlink and link the LinkedList correcly;
    Increment _size
  */
  virtual bool add(int index, T);
  /*
    Adds a T object in the end of the LinkedList;
    Increment _size;
  */
  virtual bool add(T);
  /*
    Adds a T object in the start of the LinkedList;
    Increment _size;
  */
  virtual bool unshift(T);
  /*
    Set the object at index, with T;
    Increment _size;
  */
  virtual bool set(int index, T);
  /*
    Remove object at index;
    If index is not reachable, returns false;
    else, decrement _size
  */
  virtual T remove(int index);
  virtual void remove(ListNode<T>* node);
  /*
    Remove last object;
  */
  virtual T pop();
  /*
    Remove first object;
  */
  virtual T shift();
  /*
    Get the index'th element on the list;
    Return Element if accessible,
    else, return false;
  */
  virtual T get(int index);

  /*
    Clear the entire array
  */
  virtual void clear();

  ListNode<T>* getNode(int index);
  virtual void spliceToFront(ListNode<T>* node);
  ListNode<T>* getHead() { return root; }
  T getLast() const { return last == NULL ? T() : last->data; }

};


template<typename T>
void LinkedList<T>::spliceToFront(ListNode<T>* node) {
  // Node is already root
  if (node->prev == NULL) {
    return;
  }

  node->prev->next = node->next;
  if (node->next != NULL) {
    node->next->prev = node->prev;
  } else {
    last = node->prev;
  }

  root->prev = node;
  node->next = root;
  node->prev = NULL;
  root = node;
}

// Initialize LinkedList with false values
template<typename T>
LinkedList<T>::LinkedList()
{
  root=NULL;
  last=NULL;
  _size=0;
}

// Clear Nodes and free Memory
template<typename T>
LinkedList<T>::~LinkedList()
{
  ListNode<T>* tmp;
  while(root!=NULL)
  {
    tmp=root;
    root=root->next;
    delete tmp;
  }
  last = NULL;
  _size=0;
}

/*
  Actualy "logic" coding
*/

template<typename T>
ListNode<T>* LinkedList<T>::getNode(int index){

  int _pos = 0;
  ListNode<T>* current = root;

  while(_pos < index && current){
    current = current->next;
    _pos++;
  }

  return current; //might be null
}

template<typename T>
size_t LinkedList<T>::size() const{
  return _size;
}

template<typename T>
bool LinkedList<T>::add(int index, T _t){

  if(index >= _size)
    return add(_t);

  if(index == 0)
    return unshift(_t);

  ListNode<T> *tmp = new ListNode<T>(),
         *_prev = getNode(index-1);
  tmp->data = _t;
  tmp->next = _prev->next;
  //we know that there is a next node, since 0<index<_size
  tmp->next->prev = tmp;
  tmp->prev = _prev;
  _prev->next = tmp;

  _size++;
  return true;
}

template<typename T>
bool LinkedList<T>::add(T _t){

  ListNode<T> *tmp = new ListNode<T>();
  tmp->data = _t;
  tmp->next = NULL;

  if(root){
    // Already have elements inserted
    last->next = tmp;
    tmp->prev = last;
    last = tmp;
  }else{
    // First element being inserted
    root = tmp;
    last = tmp;
  }

  _size++;

  return true;
}

template<typename T>
bool LinkedList<T>::unshift(T _t){

  if(_size == 0)
    return add(_t);

  ListNode<T> *tmp = new ListNode<T>();
  tmp->next = root;
  root->prev = tmp;
  tmp->data = _t;
  root = tmp;

  _size++;

  return true;
}

template<typename T>
bool LinkedList<T>::set(int index, T _t){
  // Check if index position is in bounds
  if(index < 0 || index >= _size)
    return false;

  getNode(index)->data = _t;
  return true;
}

template<typename T>
T LinkedList<T>::pop(){
  if(_size <= 0)
    return T();

  if(_size >= 2){
    ListNode<T> *tmp = last->prev;
    T ret = tmp->next->data;
    delete(tmp->next);
    tmp->next = NULL;
    last = tmp;
    _size--;
    return ret;
  }else{
    // Only one element left on the list
    T ret = root->data;
    delete(root);
    root = NULL;
    last = NULL;
    _size = 0;
    return ret;
  }
}

template<typename T>
T LinkedList<T>::shift(){
  if(_size <= 0)
    return T();

  if(_size > 1){
    ListNode<T> *_next = root->next;
    T ret = root->data;
    delete(root);
    _next->prev = NULL;
    root = _next;
    _size --;

    return ret;
  }else{
    // Only one left, then pop()
    return pop();
  }

}

template<typename T>
void LinkedList<T>::remove(ListNode<T>* node){
  if (node == root) {
    shift();
  } else if (node == last) {
    pop();
  } else {
    ListNode<T>* prev = node->prev;
    ListNode<T>* next = node->next;

    prev->next = next;
    next->prev = prev;

    delete node;
    --_size;
  }
}

template<typename T>
T LinkedList<T>::remove(int index){
  if (index < 0 || index >= _size)
  {
    return T();
  }

  if(index == 0)
    return shift();

  if (index == _size-1)
  {
    return pop();
  }

  ListNode<T> *tmp = getNode(index - 1);
  ListNode<T> *toDelete = tmp->next;
  T ret = toDelete->data;
  tmp->next = toDelete->next;
  toDelete->next->prev = tmp;
  delete(toDelete);
  _size--;
  return ret;
}


template<typename T>
T LinkedList<T>::get(int index){
  ListNode<T> *tmp = getNode(index);

  return (tmp ? tmp->data : T());
}

template<typename T>
void LinkedList<T>::clear(){
  while(size() > 0)
    shift();
}
#endif
