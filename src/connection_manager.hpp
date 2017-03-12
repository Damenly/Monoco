#ifndef _CONNECTION_MANAGER_HPP
#define _CONNECTION_MANAGER_HPP

#include <set>
#include <memory>

#include "config.hpp"


NAMESPACE_BEGIN(monoco)

template <typename T>
class connection_manager
{
public:

	connection_manager(const connection_manager&) = delete;
	connection_manager& operator=(const connection_manager&) = delete;

	connection_manager() {};
	
	typedef std::shared_ptr<T> connection_ptr;
	
	void start(connection_ptr c)
		{
			_connections.insert(c);
			c->start();
		}

	void stop(connection_ptr c)
		{
			_connections.erase(c);
			c->stop();
		}

	void stop_all()
		{
			for (auto &c: _connections)
				c->stop();
			_connections.clear();
		}

private:
	std::set<connection_ptr> _connections;
};

NAMESPACE_END(monoco)

#endif 
