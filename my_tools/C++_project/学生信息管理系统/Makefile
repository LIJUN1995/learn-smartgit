CC = g++
LD = $(CC)

# TARGET = $(notdir $(CURDIR))
TARGET = ./obj/main

SRC_DIR = ./Src
INCLUDE_DIR += ./Include
OUT_DIR =

C_FLAGS = -g -Wall
LD_FLAFS =  
LD_LIBS =
INCLUDES = -I$(INCLUDE_DIR)

# ifeq ($(CC), g++)
	TYPE = cpp
	SRCS += $(wildcard $(SRC_DIR)/*.$(TYPE))
	DIR += $(notdir $(SRCS))
	OBJS += $(patsubst %.$(TYPE), %.o, $(SRCS))
	
# else
# 	TYPE = c
# 	SRCS += $(wildcard $(SRC_DIR)/*.$(TYPE))
# 	OBJS += $(patsubst %.$(TYPE), %.o, $(SRCS))
# endif

all : $(TARGET)
	@echo "Builded target:" $^
	@echo "Done"
		
$(TARGET) : $(SRCS)
	@echo "Linking" $@ "from" $^ "..."
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS) $(INCLUDES)
	@echo "Compile finished\n\n"
								
.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files"
	rm -f $(TARGET)
cleanobj :
	@echo "Remove binary files"
	rm -f *.o
