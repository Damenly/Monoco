#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <string>
#include <boost/tokenizer.hpp>
#include <memory>

#include "config.hpp"
#include "common.hpp"
#include "db.hpp"
#include "cmds.cpp"

NAMESPACE_BEGIN(monoco)

struct reply;
struct request;

class request_handler
{
public:
	request_handler(){} 
	request_handler& operator=(const request_handler&) = delete;


	explicit request_handler(std::shared_ptr<mdb>ptr): db(ptr)
		{}

  void handle_request(request& req, reply& rep)
		{
			std::string res = "";
			int ret = handle_cmds(db, req.args, res);
			if (ret != 0)
				rep.headers.push_back(std::to_string(ret));
		}

	std::shared_ptr<mdb> db = nullptr;
};

NAMESPACE_END(monoco)
#endif
