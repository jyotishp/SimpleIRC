#include<iostream>
#include<ctime>
#include <cstring>

void printCurrentTime()
{
	// Declaring argument for time()
	time_t tt;

	// Declaring variable to store return value of
	// localtime()
	struct tm * ti;
	// Applying time()
	time (&tt);

	// Using localtime()
	ti = localtime(&tt);
	char *str_time = asctime(ti);
	str_time[strlen(str_time) - 1] = 0;
	std::cout << str_time << " | ";
}

void debugLog(auto msg)
{

	std::cout << "\033[0;33m" << msg
	<< "\033[0m" << std::endl;
}
