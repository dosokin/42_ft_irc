CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -g -std=c++98
SRC =   src/main.cpp \
		src/net/TcpConn.cpp \
		src/net/Message.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/irc.cpp \
		src/Channel.cpp \
		src/commands/KickCommand.cpp \
		src/commands/NickCommand.cpp \
		src/commands/ModeCommand.cpp \
		src/commands/UserCommand.cpp \
		src/commands/JoinCommand.cpp \
		src/commands/PrivMsgCommand.cpp \
		src/commands/TopicCommand.cpp \
		src/commands/InviteCommand.cpp \
		src/commands/PassCommand.cpp

OBJ = $(SRC:.cpp=.o)
INC = -Iinc -Iinc/net -Iinc/commands
EXEC = ircserv

ifdef TEST
	CXXFLAGS = -std=c++17
endif

ifdef RELEASE
	DEFINES += -DIRC_RELEASE
	CXXFLAGS += -O3
endif

ifdef CLANG
	CXX = clang++
endif

all : $(EXEC)

$(EXEC) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(DEFINES) $^ -o $@ $(INC)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -c $< -o $@ $(INC)

clean :
	rm -f $(OBJ)
	make clean -C koikoubot

fclean : clean
	rm -f $(EXEC)
	rm -f koikoubot/bot

bonus : all
	make -C koikoubot

re : fclean all
