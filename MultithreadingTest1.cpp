#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>

//int main()
//{
//
//	std::string name;
//	std::mutex name_mutex;
//	printf("Please input your name \n");
//	std::cin >> name;
//
//	std::thread th1([&]() {
//			name_mutex.lock();
//			name = std::string("Julia");
//			std::this_thread::sleep_for(std::chrono::seconds(2));
//			printf("Oops I changed your name to %s! \n", name.c_str());
//			name_mutex.unlock();
//		});
//
//	std::this_thread::sleep_for(std::chrono::seconds(1));
//
//	std::thread th2([&]() {
//			name_mutex.lock();
//			printf("Hello %s! \n", name.c_str());
//			name_mutex.unlock();
//		});
//
//	th1.join();
//	th2.join();
//
//	return 0;
//}
