#pragma once
#include <cpprest/http_listener.h>
#include <pplx/pplxtasks.h>
#include <string>
#include <cpprest/http_msg.h>
using namespace web;
using namespace http;
using namespace http::experimental::listener;

class BasicController {
public:
	BasicController(const std::wstring& address,const size_t port);
	~BasicController();
	void setEndpoint(const std::wstring& mount_point);
	std::wstring endpoint() const;
	pplx::task<void> accept();
	pplx::task<void> shutdown();

	virtual void initRestOpHandlers() = 0;

	std::vector<utility::string_t> requestPath(const http_request & message);
protected:
	http_listener _listener;
	utility::string_t quick_response(const std::wstring& key, const json::value& val) const;
private:
	uri_builder endpointBuilder;
};