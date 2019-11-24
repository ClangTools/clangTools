# thread_pool 线程池说明
## 实现原理
  “管理一个任务队列，一个线程队列，然后每次取一个任务分配给一个线程去做，循环往复。” 
这个思路有神马问题？
线程池一般要复用线程，所以如果是取一个 task 分配给某一个 thread，执行完之后再重新分配，
在语言层面基本都是不支持的：一般语言的 thread 都是执行一个固定的 task 函数，
执行完毕线程也就结束了(至少 c++ 是这样)。
so 要如何实现 task 和 thread 的分配呢？

让每一个 thread 都去执行调度函数：循环获取一个 task，然后执行之。

这样可以保证了 thread 函数的唯一性，而且复用线程执行 task 。

1. 一个线程 pool，一个任务队列 queue ；
2. 任务队列是典型的生产者-消费者模型，本模型至少需要两个工具：一个 mutex + 一个条件变量，或是一个 mutex + 一个信号量。mutex 实际上就是锁，保证任务的添加和移除(获取)的互斥性，一个条件变量是保证获取 task 的同步性：一个 empty 的队列，线程应该等待(阻塞)；
3. atomic<bool> 本身是原子类型，从名字上就懂：它们的操作 load()/store() 是原子操作，所以不需要再加 mutex。

## c++ 语言细节

1. using Task = function<void()> 是类型别名，简化了 typedef 的用法。function<void()> 可以认为是一个函数类型，接受任意原型是 void() 的函数，或是函数对象，或是匿名函数。void() 意思是不带参数，没有返回值。

2. pool.emplace_back([this]{...}) 和 pool.push_back([this]{...}) 功能一样，只不过前者性能会更好；

3. pool.emplace_back([this]{...}) 是构造了一个线程对象，执行函数是`拉姆达匿名函数` ；

4. 所有对象的初始化方式均采用了 {}，而不再使用 () 方式，因为风格不够一致且容易出错；

5. 匿名函数： [this]{...} 。[] 是捕捉器，this 是引用域外的变量 this指针， 内部使用死循环, 由cv_task.wait(lock,[this]{...}) 来阻塞线程；

6. delctype(expr) 用来推断 expr 的类型，和 auto 是类似的，相当于类型占位符，占据一个类型的位置；auto f(A a, B b) -> decltype(a+b) 是一种用法，不能写作 decltype(a+b) f(A a, B b)， c++ 规定！

7. commit 方法可以带任意多的参数，第一个参数是 f，后面依次是函数 f 的参数！(注意:参数要传struct/class的话,建议用pointer,小心变量的作用域) 可变参数模板是 c++11 的一大亮点，至于为什么是 Arg... 和 arg... ，因为规定就是这么用的！

8. commit 直接使用只能调用stdcall函数，但有两种方法可以实现调用类成员，一种是使用 bind： .commit(std::bind(&Dog::sayHello, &dog))； 一种是用 mem_fn： .commit(std::mem_fn(&Dog::sayHello), &dog)；

9. make_shared 用来构造 shared_ptr 智能指针。用法大体是 shared_ptr<int> p = make_shared<int>(4) 然后 *p == 4 。智能指针的好处就是， 自动 delete ！

10. bind 函数，接受函数 f 和部分参数，返回currying后的匿名函数，譬如 bind(add, 4) 可以实现类似 add4 的函数！

11. forward() 函数，类似于 move() 函数，后者是将参数右值化，前者是... 大概意思就是：不改变最初传入的类型的引用类型(左值还是左值，右值还是右值)；

12. packaged_task 就是任务函数的封装类，通过 get_future 获取 future ， 然后通过 future 可以获取函数的返回值(future.get())；packaged_task 本身可以像函数一样调用 () ；

13. queue 是队列类， front() 获取头部元素， pop() 移除头部元素；back() 获取尾部元素，push() 尾部添加元素；

14. lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()，是 c++ RAII 的 idea；

15. condition_variable cv; 条件变量， 需要配合 unique_lock 使用；unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()。 cv.wait() 之前需要持有 mutex，wait 本身会 unlock() mutex，如果条件满足则会重新持有 mutex。

16. 最后线程池析构的时候,join() 可以等待任务都执行完在结束!


作者：Caiaolun

链接：https://www.jianshu.com/p/eec63026f8d0

来源：简书

著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

