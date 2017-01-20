# ThreadPool
Simple header-only ThreadPool class using c++14


This is a simple, header only class which implements a thread pool that work can be submitted to. 

After creating the threadpool object, specifying the number of worker threads in the constructor, submit a callable object and it's args
(similar to how the std::thread constructor works). The submit method returns a std::future of the return type of the function. (automatically deduced!)


At a later time, call get() on the returned future to get the value, or the exception if you function threw one. 


