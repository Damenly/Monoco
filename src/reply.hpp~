#ifndef _REPLY_HPP
#define _REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

struct reply
{
	std::vector<std::string> headers;

	std::vector<boost::asio::const_buffer> to_buffers()
		{
			std::vector<boost::asio::const_buffer> buffers;
			
			for (std::size_t i = 0; i < headers.size(); ++i)
			{
				buffers.push_back(boost::asio::buffer(headers[i]));
			}
			
			return buffers;
		}

	reply() {}
};

NAMESPACE_END(monoco)
#endif
