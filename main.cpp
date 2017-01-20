#include "ThreadPool.h"

#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;
int main()
{
	ThreadPool t(2);
	vector<future<int>> futures;
	for (int i = 0; i < 4; i++)
	{
		std::function<int(int)> f = [i](int m)
		{
			this_thread::sleep_for(milliseconds(30));
			cout << "Computing function number " << i << " on thread " << this_thread::get_id() <<endl;
			return i + m;
		};
		auto fut = t.submit(f, 10 * i);
		futures.push_back(move(fut));
	}
	for (int i = 0; i < 4; i++)
	{
		cout << futures.at(i).get() << endl;
	}
	cout << "done" << endl;
	cin.getline(nullptr, 0);

}