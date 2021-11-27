//#include <iostream>
//#include <thread>
//#include <vector>
//
//using namespace std;
//
//void thread_worker( int my_id )
//{
//	// 내 자신을 this라고 함
//	// cout << "I am Thread [" << this_thread::get_id() << "]\n" << endl;
//	cout << "I am Thread [" << my_id << "]\n" << endl;
//}
//
//int main()
//{
//	vector<thread> workers;
//	for (int i = 0; i < 10; ++i) {
//		workers.emplace_back(thread_worker, i);
//	}
//
//	for (auto& th : workers) {
//		th.join();
//	}
//}



//// 현재 싱글 쓰레드
//#include <iostream>
//#include <chrono>
//
//using namespace std;
//using namespace std::chrono;
//
//volatile int sum;
//
//int main()
//{
//	auto start_t = high_resolution_clock::now();
//
//	for (int i = 0; i < 500000000; ++i)
//		sum += 2;
//	cout << "sum = " << sum << endl;
//
//	auto end_t  = high_resolution_clock::now();
//
//	cout << "Exe time : " <<  duration_cast<milliseconds>(end_t - start_t).count() << "ms" <<  endl;
//}

// 현재 싱글 쓰레드
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
using namespace std::chrono;


volatile int sum = 0;

mutex my_lock;
void worker(int num_threads)
{
	int loop_count = 500000000 / num_threads;
	volatile int local_sum = 0;		// 하이퍼 쓰레드? 도 열일을 함
	for (int i = 0; i < loop_count; ++i) {
		local_sum += 2;
	}
	my_lock.lock();
	sum += local_sum;
	my_lock.unlock();


}

int main()
{
	cout << "Single Thread Result" << endl;
	auto start_t = high_resolution_clock::now();

	for (int i = 0; i < 500000000; ++i)
		sum += 2;
	cout << "sum = " << sum << endl;
	worker(1);

	auto end_t = high_resolution_clock::now();

	cout << "Exe time : " << duration_cast<milliseconds>(end_t - start_t).count() << "ms" << endl;

	sum = 0;


	cout << "Multi Thread Result" << endl;

	for (int i = 1; i <= 8; ++i) {
		sum = 0;
		start_t = high_resolution_clock::now();
		vector<thread> workers;
		for (int j = 1; j <= i; ++j) {
			workers.emplace_back(worker, i);
		}

		for (auto& th : workers)
			th.join();

		end_t = high_resolution_clock::now();
		cout << "sum = " << sum << endl;
		cout << "Exe time : " << duration_cast<milliseconds>(end_t - start_t).count() << "ms" << endl;
	}
}