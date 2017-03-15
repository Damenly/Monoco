#include <termio.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include <iostream>
#include <boost/asio.hpp>
#include <array>

using boost::asio::ip::tcp;
using namespace std;

enum { max_length = 1024 };

int getch() {
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}


string getpass(const char *prompt, bool show_asterisk=true)
{
  const char BACKSPACE = 127;
  const char RETURN = 10;

  string password;
  unsigned char ch = 0;

  cout << prompt << endl;

  while ((ch = getch()) != RETURN)
    {
       if (ch == BACKSPACE)
         {
            if (password.length()!=0)
              {
                 if (show_asterisk)
					 cout <<"\b \b";
                 password.resize(password.length() - 1);
              }
         }
       else
         {
             password += ch;
             if(show_asterisk)
                 cout <<'*';
         }
    }
  
  cout <<endl;
  return password;
}


int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({argv[1], argv[2]}));

	bool logined = false;
  next:
    std::cout << ">>>>>>>>>>>>>>>>>>";
    char request[max_length];
	if (logined)
		std::cin.getline(request, max_length);
	else {
		cout << "Please input password" << endl;
		auto pwd = getpass("Please enter the password: ",true);
		strcpy(request, pwd.c_str());
	}
	
    size_t request_length = std::strlen(request);
	
	if (request_length == 0)
		goto next;
	
    boost::asio::write(s, boost::asio::buffer(request, request_length));
	
	char reply[8192];
	boost::system::error_code ec;
    size_t reply_length = s.read_some(boost::asio::buffer(reply));
	
	if (logined)
		std::cout.write(reply, reply_length);
	
	if (!logined) {
		if (memcmp(reply, "0", 1)) {
			cout << "wrong password" << endl;
			exit(1);
		}
		else {
			cout << "Hello" << endl;
			logined = true;
		}
	}
	
	goto next;
	
  }
  catch (std::exception& e)
  {
	  std::cerr << "Connection is down" << endl;
  }

  return 0;
}
