AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=monoco
CXX_FLAGS=-std=c++1z -l boost_system -lboost_filesystem -lboost_system  -lpthread -lboost_filesystem -lcrypto -lboost_thread
INCLUDES= -I /usr/local/include -I /usr/local/opt/openssl/include/
LDADD= -L /usr/local/lib -L /usr/local/Cellar/openssl/1.0.2k/lib/
monoco_SOURCES =main.cpp aof.hpp common.hpp  db.hpp mbj.hpp mht.hpp mset.hpp mzset.hpp  zl_entry.hpp cmds.cpp mdict.cpp utility.hpp
#Monoco_SOURCES = main.cpp
