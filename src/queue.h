#ifndef QUEUE_H_
#define QUEUE_H_

#include <algorithm>

template<typename _Key, typename _Compare>
class HeapPriorityQueue {
  public:
   HeapPriorityQueue() {//头部元素值最大
     compare_ = _Compare();
     elements_ = std::vector<_Key>();
     std::make_heap(elements_.begin(), elements_.end(), compare_);
   }

   bool empty() const { return elements_.empty(); }

   int size() const { return static_cast<int>(elements_.size()); }

   void reset() {
     while (elements2_.size()) {
       push(elements2_.back());  //push elements in elements2_ to elements_ and push heap
       elements2_.pop_back();
     }
   }

   bool next(_Key& key) {
     if (elements_.size() > 0) {
       key = top();
       pop();
       elements2_.push_back(key);  //push elements that poped from elements_ to elements2_
       return true;
     }
     else
       return false;
   }

   _Key& top() { return elements_.front(); } //??返回的是值

   void push(const _Key& key)
   {
     elements_.push_back(key);//添加到最后位置
     std::push_heap(elements_.begin(), elements_.end(), compare_);//把新加入的元素放到堆的适当位置
   }

   void pop() {
     std::pop_heap(elements_.begin(), elements_.end(), compare_);//取出最大元素并在堆中删除
     elements_.pop_back();
   }

   void pop(_Key& key) {
     key = elements_.front();
     std::pop_heap(elements_.begin(), elements_.end(), compare_);
     elements_.pop_back();
   }

   void clear() {
     elements2_.clear();
     elements_.clear();
   }

   std::vector<_Key>& elements() {
     return elements_;
   }

   void resort() {
     std::make_heap(elements_.begin(), elements_.end(), compare_);
   }

   void reset_dist_to_steiner() {
     const int elements_size = static_cast<int>(elements_.size());
     for (int i = 0; i < elements_size; i++) {
       elements_[i].set_dist_to_steiner(0);
       elements_[i].set_cost(0);
     }
     std::make_heap(elements_.begin(), elements_.end(), compare_);
   }

  private:
   std::vector<_Key> elements_;
   std::vector<_Key> elements2_; // to store the keys are poped in function "next"
   _Compare compare_;

};

#endif
