#include "threadpool.h"
#include <windows.h>

void hello()
{

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	std::cout <<
		"Hello world, I'm a function runing in a thread!"
		<< std::endl;
}


class A
{
private:
	int m_n;
public:
	A(int n)
		:m_n(n)
	{}
	~A(){}
public:
	void foo(int k)
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::cout << "n*k = " << k*m_n << std::endl;
		m_n++;
	}
};


int main()
{
	cpp11_thread_pool threadpool(2);
	srand((unsigned int)time(0));
	A a(1), b(2), c(3);
	int nsleep = rand() % 900 + 100;

	threadpool.append(&hello);

	threadpool.append
		(
		[&nsleep]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::cout << "I'm a lamda runing in a thread" << std::endl;
	}
	);
	//append object method with copy-constructor(value-assignment)
	threadpool.append(std::bind(&A::foo, a, 10));
	threadpool.append(std::bind(&A::foo, b, 11));
	threadpool.append(std::bind(&A::foo, c, 12));
	threadpool.append(std::bind(&A::foo, a, 100));
	Sleep(1000);
	//append object method with address assignment, will cause the objects' member increase.
	threadpool.append(std::bind(&A::foo, &a, 10));
	threadpool.append(std::bind(&A::foo, &b, 11));
	threadpool.append(std::bind(&A::foo, &c, 12));
	threadpool.append(std::bind(&A::foo, &a, 100));
	//wait for all tasks done.
	threadpool.wait_for_idle();
	while (1)
	{
		threadpool.append(std::bind(&A::foo, &a, 100));
		Sleep(500);
	}
	
	//
	threadpool.terminate();
	//
	threadpool.join();

	//test function
	std::function < void(void) > func1 = &hello;
	std::function < void(void) > func2 = &hello;
	if (func1.target<void(void) >() != func2.target<void(void)>())
		return 0;
	else
		return 1;
}
