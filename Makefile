CXX            = g++

FRONT_SRC      = main.cpp lexer.cpp
SUBMODULE_SRC  = colorPrint/colorPrint.cpp

FRONT_DIR      = front-end/
BUILD_DIR      = build/
SRC_DIR        = src/
INC_DIR        = include/

CFLAGS         = -I colorPrint/ -I customWarning -I myOnegin/ -I $(addprefix $(FRONT_DIR), $(INC_DIR)) -I Buffer/

FRONTEND       = frontend
OBJECT         = $(patsubst %.cpp, %.o, $(SRC))

GREEN_COLOR    = \033[1;32m
YELLOW_COLOR   = \033[1;33m
DEFAULT_COLOR  = \033[0m

DED_FLAGS      = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations 								   \
							 -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported    \
							 -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security			   \
							 -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual    \
							 -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo 			   \
							 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods 			   \
							 -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef 				   \
							 -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix 					 		   \
							 -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new \
							 -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer       \
							 -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,   \
							 float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,      \
							 return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr,

.PHONY: all

frontend: $(addprefix $(addprefix $(FRONT_DIR), $(SRC_DIR)), $(FRONT_SRC))
	clear
	@mkdir -p $(addprefix $(FRONT_DIR), $(BUILD_DIR))
	@$(CXX) $(CFLAGS) $^ $(SUBMODULE_SRC) -o $(addprefix $(addprefix $(FRONT_DIR), $(BUILD_DIR)), $(FRONTEND))
	@printf "$(GREEN_COLOR)$(FRONTEND) COMPILED$(DEFAULT_COLOR)\n"

frontend_clean :
	@rm -f -r $(addprefix $(addprefix $(FRONT_DIR), $(BUILD_DIR)), *.o)
	@printf  "$(YELLOW_COLOR)$(TARGET) CLEANED$(DEFAULT_COLOR)\n"
