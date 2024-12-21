CXX           = g++

TREE_SRC      = main.cpp

TARGET        = binaryTree

SUBMODULE_SRC = customWarning/customWarning.h

BUILD_DIR     = build/
SRC_DIR       = src/
CFLAGS        = -I customWarning/ -I include/

OBJECT        = $(patsubst %.cpp, %.o, $(SRC))
BUILD_OBJ     = $(addprefix $(BUILD_DIR), $(OBJECT))

GREEN_TEXT   			= \033[1;32m
YELLOW_TEXT  			= \033[1;33m
CYAN_TEXT         = \033[1;36m
DEFAULT_BOLD_TEXT = \033[1;37m
DEFAULT_TEXT 			= \033[0m

DED_FLAGS    = -D _DEBUG -ggdb2 -std=c++17 -O0 -Wall -Wextra -Weffc++                                     \
			    	   -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations                      \
				       -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported                       \
				       -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral            \
				       -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor  \
				       -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls  \
				       -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2         \
				       -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types                 \
				       -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef                      \
				       -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros                               \
				       -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing                         \
				       -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation        \
				       -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer         \
				       -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla

vpath %.o   $(BUILD_DIR)
vpath %.cpp $(SRC_DIR)

.PHONY: all

all:
	@make -s clean
	make quickTree
	@printf "\n$(CYAN_TEXT)RUNNING...\n\n$(DEFAULT_TEXT)"

$(TARGET): $(BUILD_DIR) $(OBJECT)
	$(CXX)   $(BUILD_OBJ) -o $(TARGET) -D _NDEBUG
	@printf "$(GREEN_TEXT)$(TARGET) COMPILED$(DEFAULT_TEXT)\n"

$(BUILD_DIR):
	@mkdir -p build

$(OBJECT): %.o : %.cpp
	$(CXX) $(CFLAGS) -c $^ -o $(addprefix $(BUILD_DIR), $@)

quickTree: $(addprefix $(SRC_DIR), $(TREE_SRC))
	make -s $(BUILD_DIR)
	@$(CXX) $(CFLAGS) $^ $(SUBMODULE_SRC) -o $(addprefix $(BUILD_DIR), $(TARGET))

clean:
	@printf "$(GREEN_TEXT)► $(CYAN_TEXT)build/ $(DEFAULT_BOLD_TEXT)was removed!\n$(DEFAULT_TEXT)"
	@rm -rf build