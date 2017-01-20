#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>
#include <future>
#include <queue>
#include <memory>

class ThreadPool
{
private:
	using func_type = std::function<void()>;
	std::vector<std::thread> m_threads;
	std::queue<func_type> m_pending_work;
	std::mutex m_submit_mutex;
	std::condition_variable m_cv;
	bool m_should_quit = false;

	void worker()
	{
		using namespace std;
		while (! m_should_quit)
		{
			unique_lock<mutex> lock(m_submit_mutex);
			//wait for either work or destruction.
			if(m_pending_work.empty())
			{
				m_cv.wait(lock, [this] {return (! m_pending_work.empty()) || m_should_quit; });
			}
			if (m_should_quit) return;
			//we hold the lock at this point. 
			auto f = move(m_pending_work.front());
			m_pending_work.pop();
			lock.unlock();
			//actually call lambda
			f();
		}
	}
public:
	ThreadPool(size_t nthreads)
	{
		for (size_t i = 0; i < nthreads; i++)
		{
			m_threads.push_back(std::thread(&ThreadPool::worker, this));
		}
	}
	~ThreadPool()
	{
		using namespace std;
		unique_lock<mutex> lock(m_submit_mutex);
		m_should_quit = true;
		lock.unlock();
		m_cv.notify_all();
		for (auto& t : m_threads)
		{
			if (t.joinable()) t.join();
		}
	}
	template<typename Ret, typename... Args>
	std::future<Ret> submit(std::function<Ret(Args...)> work, Args&&... args)
	{
		using namespace std;
		using f_type = std::function<Ret(Args...)>;
		auto prom = make_shared<std::promise<Ret>>();
		std::future<Ret> f = prom->get_future();
		//m_pending_work.push([prom=std::forward<promise<Ret>>(prom), work=std::forward<f_type>(work), args=std::forward<Args...>(args...)]
		auto l = [prom(move(prom)), work(forward<f_type>(work)), args(forward<Args>(args)...)](Args... args)
		{
			try
			{
				Ret r = work(std::forward<Args>(args)...);
				prom->set_value(move(r));
			}
			catch (std::exception const& ex)
			{
				prom->set_exception(std::current_exception());
			}
		};

		auto lb = std::bind(move(l), std::forward<Args>(args)...);
		unique_lock<mutex> lock(m_submit_mutex);
		m_pending_work.push(move(lb));
		lock.unlock();
		m_cv.notify_one();
		return f;
	}

};