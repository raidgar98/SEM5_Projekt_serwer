#include <iostream>
#include <thread>
#include "service.h"
#include "FileService.h"
using namespace std;

//mainCRTStartup

int main()
{
	Service serv(L"127.0.0.1", 9001);
	FileService fserv(L"127.0.0.1", 9000);
	serv.setEndpoint(L"/api");
	fserv.setEndpoint(L"/file");
	serv.accept().wait();
	fserv.accept().wait();
	while(true)
		std::this_thread::yield();
	return 0;

}