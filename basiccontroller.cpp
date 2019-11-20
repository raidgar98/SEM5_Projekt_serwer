#include "basiccontroller.h"


BasicController::BasicController(const std::wstring& naddress,const size_t nport) 
{
	this->endpointBuilder.set_host(naddress);
	this->endpointBuilder.set_port(static_cast<int>(nport));
	this->endpointBuilder.set_scheme(utility::string_t(L"http"));
}
BasicController::~BasicController() { }

void BasicController::setEndpoint(const std::wstring& mount_point)
{
	endpointBuilder.set_path(mount_point);
	_listener = http_listener(endpointBuilder.to_uri());
}

pplx::task<void> BasicController::accept() 
{
	initRestOpHandlers();
	return _listener.open();
}
pplx::task<void> BasicController::shutdown() 
{
	return _listener.close();
}

std::vector<utility::string_t> BasicController::requestPath(const http_request& message)
{
	auto relativePath = uri::decode(message.relative_uri().path());
	return uri::split_path(relativePath);
}

utility::string_t BasicController::quick_response(const std::wstring& key, const json::value& val) const
{
	json::value tmp = json::value::object();
	tmp[key] = val;
	return tmp.serialize();
}

