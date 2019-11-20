#pragma once
#include "basiccontroller.h"

using std::vector;
using real = double;
using num = real;

class Service : public BasicController 
{
public:
	Service(const std::wstring& address,const int port) : BasicController(address,port) {}
	~Service() {}
	void handleGet(http_request message);
	void handlePost(http_request message);
	void initRestOpHandlers() override;

private:

	void cpu_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data = json::value::null()) noexcept;
	void disk_decision		(const vector<utility::string_t>& path, http_request& decision, json::value post_data = json::value::null()) noexcept;
	void mem_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data = json::value::null()) noexcept;
	void net_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data = json::value::null()) noexcept;
	void system_decision	(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data = json::value::null()) noexcept;
};