#pragma once

#include <list>

template <typename T>
class slidingWindow
{
  private:
    std::list<T> queue;
    int size;
  
  public:
    slidingWindow(int size = 1)
    : size(size) {}
    
    virtual ~slidingWindow(){}

    double operator() ()
    {
        if(queue.empty())
            return 0;

        double sum = 0;

        for(auto& item : queue)
            sum += item;

        return sum/queue.size();
    }
    
    void operator= (T newItem)
    {
        if(queue.size() < size)
            queue.push_back(newItem);
        else
        {
            queue.push_back(newItem);
            queue.pop_front();
        }
    }

    friend std::ostream& operator<< (std::ostream& os, const slidingWindow& sw){
        for(auto& item : sw.queue)
            os << item << " ";
        return os << std::endl;
    }
};