#include "service.h"
#include "info_collector.h"

#define str std::wstring

void Service::initRestOpHandlers() 
{
	_listener.support(methods::GET,std::bind(&Service::handleGet, this, std::placeholders::_1));
	_listener.support(methods::POST, std::bind(&Service::handlePost, this, std::placeholders::_1));
}

void Service::handleGet(http_request message) {
	vector<str> path = requestPath(message);
	//std::wcout << L"[LOG] GET request FROM " << message.get_remote_address().c_str() << L" with path: '" << message.relative_uri().to_string().c_str() << L"'" << std::endl;
	if(path.empty()) 
		message.reply(status_codes::OK, quick_response(L"online", json::value::boolean(true)));
	else if( path[0] == L"cpu" )
		cpu_decision(path, message);
	else if(path[0] == L"net" )
		net_decision(path, message);
	else if(path[0] == L"disk" )
		disk_decision(path, message);
	else if(path[0] == L"system" )
		system_decision(path, message);
	else if(path[0] == L"mem" )
		mem_decision(path, message);
	else message.reply(status_codes::BadRequest, quick_response(L"result", json::value::string(L"invalid path")));
}

void Service::handlePost(http_request message) 
{
	// std::wcout << "[LOG] POST request FROM " << message.get_remote_address() << " with path: '" << message.relative_uri().to_string() << "' and data: " << message.extract_json().get() << std::endl;
	vector<str> path = requestPath(message);
	auto task = message.extract_json();
	if(path.empty()) 
		message.reply(status_codes::OK, quick_response(L"online", json::value::boolean(true)));
	else if( path[0] == L"cpu" )
		cpu_decision(path, message, task.get());
	else if(path[0] == L"net" )
		net_decision(path, message, task.get());
	else if(path[0] == L"disk" )
		disk_decision(path, message, task.get());
	else if(path[0] == L"system" )
		system_decision(path, message, task.get());
	else if(path[0] == L"mem" )
		mem_decision(path, message, task.get());
	else message.reply(status_codes::BadRequest, quick_response(L"result", json::value::string(U("invalid path"))));
}

void Service::cpu_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data) noexcept
{
	json::value ret = json::value::object();

	if(path.size() >= 2)
	{
		if(path[1] == L"cores")
			ret[L"cores"] = json::value::number(WindowsInfoCollector::get_amount_of_cores());
		else if(path[1] == L"usage")
			ret[L"usage"] = json::value::number(WindowsInfoCollector::cpu_usage());
		else if(path[1] == L"model")
			ret[L"model"] = json::value::string(WindowsInfoCollector::get_cpu_info());
		else
		{
			decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"invalid path or corrupted POST data provided")));
			return;
		}
	}
	else
	{
		ret[L"usage"] = json::value::number(WindowsInfoCollector::cpu_usage());
		ret[L"cores"] = json::value::number(WindowsInfoCollector::get_amount_of_cores());
	}
	
	decision.reply(status_codes::OK, ret.serialize());
}

void Service::disk_decision		(const vector<utility::string_t>& path, http_request& decision, json::value post_data) noexcept
{
	json::value ret = json::value::object();
	if(path.size() >= 2)
	{
		if(path[1] == L"space_total")
		{
			const auto tot = WindowsInfoCollector::SImplify(WindowsInfoCollector::total_disks_space());
			ret[L"space_total"] = json::value::number(tot.first);
			ret[L"SI"] = json::value::string(tot.second);
		}
		else if(path[1] == L"space_total_free")
			ret[L"space_total_free"] = json::value::number(WindowsInfoCollector::total_free_space());
		else if(path[1] == L"disks")
		{
			const std::vector<int> tab{ WindowsInfoCollector::avaiable_disk() };
			std::vector<json::value> tmp;
			tmp.reserve(tab.size());
			for(const int var : tab)
				tmp.emplace_back(json::value::number(var));
			ret[L"disks"] = json::value::array(tmp);
		}
		else if(path[1] == L"space" && post_data.at(L"letter").is_string())
		{
			const wchar_t ch = post_data.at(L"letter").as_string()[0];
			bool found = false;
			for( const int var : WindowsInfoCollector::avaiable_disk())
				if(wchar_t(var) == ch)
				{
					found = true;
					break;
				}
			if(!found)
			{
				decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"no such disk")));
				return;
			}
			const auto tot = WindowsInfoCollector::SImplify(WindowsInfoCollector::disk_space(ch));
			ret[L"space_total"] = json::value::number(tot.first);
			ret[L"SI"] = json::value::string(tot.second);
		}
		else if(path[1] == L"space_free" && post_data.at(L"letter").is_string())
		{
			const char ch = post_data.at(L"letter").as_string()[0];
			{
				bool found = false;
				for( const char var : WindowsInfoCollector::avaiable_disk())
					if(var == ch)
					{
						found = true;
						break;
					}
				if(!found)
				{
					decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"no such disk")));
					return;
				}
			}
			ret[L"space_free"] = json::value::number(WindowsInfoCollector::free_space(ch));
		}
		else
		{
			decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"invalid path or corrupted POST data provided")));
			return;
		}
	}
	else
	{
		const auto tot = WindowsInfoCollector::SImplify(WindowsInfoCollector::total_disks_space());
		json::value val = json::value::object();
		val[L"space"] = json::value::number(tot.first);
		val[L"SI"] = json::value::string(tot.second);
		ret[L"space_total"] = val;
		ret[L"space_total_free"] = json::value::number(WindowsInfoCollector::total_free_space());
	}
	
	decision.reply(status_codes::OK, ret.serialize());
}

void Service::mem_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data) noexcept
{
	json::value ret = json::value::object();

	if(path.size() >= 2)
	{
		if(path[1] == L"size")
		{
			const auto tot = WindowsInfoCollector::SImplify(WindowsInfoCollector::get_physical_memory());
			ret[L"space_total"] = json::value::number(tot.first);
			ret[L"SI"] = json::value::string(tot.second);
		}
		else if(path[1] == L"used")
			ret[L"used"] = json::value::number(WindowsInfoCollector::get_physical_memory_usage());
		else
		{
			decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"invalid path or corrupted POST data provided")));
			return;
		}
	}
	else
	{
		json::value val = json::value::object();
		const auto tot = WindowsInfoCollector::SImplify(WindowsInfoCollector::get_physical_memory());
		val[L"space"] = json::value::number(tot.first);
		val[L"SI"] = json::value::string(tot.second);
		ret[L"total_space"] = val;
		ret[L"used"] = json::value::number(WindowsInfoCollector::get_physical_memory_usage());
	}
	
	decision.reply(status_codes::OK, ret.serialize());
}

void Service::net_decision		(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data) noexcept
{
	json::value ret = json::value::object();

	const std::set<str> tab{ WindowsInfoCollector::avaiable_ips() };
	std::vector<json::value> tmp;
	tmp.reserve(tab.size());
	for(const str& var : tab)
		tmp.emplace_back(json::value::string(var));
	ret[L"avaiable_ips"] = json::value::array(tmp);
	
	decision.reply(status_codes::OK, ret.serialize());
}

void Service::system_decision	(const vector<utility::string_t>& path, http_request& decision, const json::value& post_data) noexcept
{
	json::value ret = json::value::object();

	if(path.size() >= 2)
	{
		if(path[1] == L"os_info")
			ret[L"os_info"] = json::value::string(WindowsInfoCollector::os_info());
		else if (path[1] == L"os_version")
		{
			const auto res = WindowsInfoCollector::version();
			ret[L"os"] = json::value::object();
			ret[L"sp"] = json::value::object();
			ret[L"os"][L"major"] = json::value::number(res.first.first);
			ret[L"os"][L"minor"] = json::value::number(res.first.second);
			ret[L"sp"][L"major"] = json::value::number(res.second.first);
			ret[L"sp"][L"minor"] = json::value::number(res.second.second);
		}
		else
		{
			decision.reply(status_codes::BadRequest, quick_response(L"reason", json::value::string(L"invalid path or corrupted POST data provided")));
			return;
		}
	}
	else
	{
		ret[L"os_info"] = json::value::string(WindowsInfoCollector::os_info());
		ret[L"os_version"] = json::value::object();
		const auto res = WindowsInfoCollector::version();
		ret[L"os_version"][L"os"] = json::value::object();
		ret[L"os_version"][L"os"][L"major"] = json::value::number(res.first.first);
		ret[L"os_version"][L"os"][L"minor"] = json::value::number(res.first.second);
		ret[L"os_version"][L"sp"] = json::value::object();
		ret[L"os_version"][L"sp"][L"major"] = json::value::number(res.second.first);
		ret[L"os_version"][L"sp"][L"minor"] = json::value::number(res.second.second);
	}
	
	decision.reply(status_codes::OK, ret.serialize());
}