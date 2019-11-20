#define _CRT_SECURE_NO_WARNINGS

#include "FileService.h"

#include <chrono>
#include <ctime>
#include <Windows.h>

#include <cpprest/filestream.h>

#define PATH LR"(C:\Users\raidg\Documents\raport_dzienny\raport.xlsx)"

void FileService::handleGet(http_request message)
{
	const auto path = requestPath(message);
	if (path[0] == L"raport")
	{
		auto fileStream = Concurrency::streams::fstream::open_istream(PATH).get();
		if (!fileStream) {
			throw std::runtime_error("Can not open file");
		}
		else {
			http_response resp;
			resp.set_status_code(200);
			resp.headers().set_content_type(L"application/octet-stream");
			resp.set_body(fileStream);
			message.reply(resp);
		}
	}
}

void FileService::initRestOpHandlers()
{
	_listener.support(methods::GET, std::bind(&FileService::handleGet, this, std::placeholders::_1));
}

std::wstring FileService::get_formatted_filename() const
{
	char * name = new char[1024];
	DWORD size = 1024;
	GetComputerNameA(name, &size);
	std::string format = "raport_";
	format += name;
	format += "_%Y-%m-%d.xlsx";
	std::string s(255, '\0');
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::strftime(&s[0], s.size(), format.c_str(), std::localtime(&now));
	std::wstring ret;
	ret.reserve(s.size());
	for (const auto var : s)
		ret += static_cast<wchar_t>(var);
	return ret;
}