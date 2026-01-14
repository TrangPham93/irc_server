CXX = c++
FLAGS = -std=c++20 -Wall -Wextra -Werror

OBJS_DIR = obj
HDRS_DIR = inc/
SRCS_DIR = srcs/
CMDS_DIR = srcs/cmds/
HDRS = -I$(HDRS_DIR)

NAME = ircserv

SRCS = $(SRCS_DIR)main.cpp \
		$(SRCS_DIR)Server.cpp \
		$(SRCS_DIR)Client.cpp \
		$(SRCS_DIR)utils.cpp \
		$(CMDS_DIR)Join.cpp \
		$(CMDS_DIR)Topic.cpp \
		$(CMDS_DIR)Mode.cpp \
		$(CMDS_DIR)ModeHandlers.cpp \
		$(SRCS_DIR)Channel.cpp \
		$(CMDS_DIR)Invite.cpp \
		$(CMDS_DIR)Pass.cpp \
		$(CMDS_DIR)Nick.cpp \
		$(CMDS_DIR)User.cpp \
		$(CMDS_DIR)Ping.cpp \
		$(SRCS_DIR)message.cpp \
		$(SRCS_DIR)parsing.cpp \
		$(CMDS_DIR)Privmsg.cpp \
		$(CMDS_DIR)Kick.cpp \
		$(CMDS_DIR)Part.cpp \
		$(CMDS_DIR)Quit.cpp

OBJS = $(patsubst srcs/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))

GREEN = \033[32m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Compiling executable...$(RESET)"
	@$(CXX) $(FLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)COMPILATION COMPLETE!$(RESET)"
	@echo "Usage: ./ircserv <port> <set_password>"

$(OBJS_DIR)/%.o: srcs/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(FLAGS) $(HDRS) -MMD -MP -c $< -o $@

clean:
	@echo "$(GREEN)Cleaning object files$(RESET)"
	rm -rf $(OBJS_DIR)

fclean: clean
	@echo "$(GREEN)Removing executable$(RESET)"
	rm -f $(NAME)

re: fclean all

-include $(OBJS:.o=.d)

.SECONDARY: $(OBJS)

.PHONY: all clean fclean re
