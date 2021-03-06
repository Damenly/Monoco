#ifndef _REQUEST_PARSER_HPP_
#define _REQUEST_PARSER_HPP_

#include <tuple>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include "config.hpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

struct request;

class request_parser
{
public:
	request_parser() {};

	void reset();

	enum result_type { good, bad, indeterminate };

	template <typename InputIterator>
	std::tuple<result_type, InputIterator> parse(request& req,
												 InputIterator begin, InputIterator end)
		{
	  while (begin != end)
	  {
		  string arg(begin, end);
		  result_type result = handle_arg(req, arg);
		  if (result == good || result == bad)
			  return std::make_tuple(result, begin);
	  }
	  return std::make_tuple(indeterminate, begin);
		}

	result_type
	handle_arg(request& req, string& str)
		{

			boost::char_separator<char> sep(" ");
			boost::tokenizer<boost::char_separator<char>> tokens(str, sep);
			for (const auto& t : tokens) {
				req.args.push_back(t);
			}
			return good;
		}
};

NAMESPACE_END(monoco)

#endif
