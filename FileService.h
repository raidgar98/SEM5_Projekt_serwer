#pragma once
#include "basiccontroller.h"
class FileService :
	public BasicController
{
public:

	FileService(const std::wstring& address, const int port) : BasicController(address, port) {}
	~FileService() {}

	void handleGet(http_request message);
	void initRestOpHandlers() override;
	std::wstring get_formatted_filename() const;
};

