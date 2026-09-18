FLAGS = -std=c++17 -Wall -Wextra -g
SRCS = main.cpp url_parser.cpp http_client.cpp http_response.cpp https_client.cpp
LIBS = -lssl -lcrypto

monitor: $(SRCS)
	g++ $(FLAGS) -o monitor $(SRCS) $(LIBS)

clean:
	rm -f monitor