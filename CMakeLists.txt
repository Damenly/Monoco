cmake_minimum_required(VERSION 2.8.9)
project(Monoco)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(SOURCE src/)

#include lib headers
include_directories(src/ /usr/local/include -I /usr/local/opt/openssl/include/)

#link libs 
link_directories(/usr/local/lib/
	/usr/local/Cellar/openssl/1.0.2k/lib/)

#Can manually add the sources using the set command as follows:
set(SERVER src/main.cpp)
set(SLAVE src/slave.cpp)
set(SENTRY src/sentry.cpp)
set(CLIENT src/client.cpp)

set(LIBS boost_system boost_filesystem
					  boost_system pthread boost_filesystem
					  crypto boost_thread-mt)

add_executable(server ${SERVER})
target_link_libraries(server ${LIBS})

add_executable(client ${CLIENT})
target_link_libraries(client ${LIBS})

add_executable(sentry ${SENTRY})
target_link_libraries(sentry ${LIBS})

add_executable(slave ${SLAVE})
target_link_libraries(slave ${LIBS})


