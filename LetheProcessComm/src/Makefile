INCLUDE_FLAGS:=-I../include \
               -I../../LetheCommon/include \
               -I../../thirdParty

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*' 2>&-)

COMPILE_FLAGS:=-c -std=c++0x -Wall -Wextra -Werror -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-rcv

LIBRARY_DIR  :=../bin
LIBRARY_FILE :=$(LIBRARY_DIR)/LetheProcessComm.a

OBJECT_FILES :=ByteStream/ProcessByteStream.o \
               MessageStream/ProcessMessageStream.o \
               MessageStream/ProcessMessageHeader.o \
               MessageStream/ProcessMessageList.o \
               MessageStream/ProcessMessageReceiveList.o \
               MessageStream/ProcessMessageUnallocList.o \
               MessageStream/ProcessMessage.o \
               LinuxHandleTransfer.o \
               TempProcessStream.o

all: $(LIBRARY_FILE)

clean:
	rm -rf $(LIBRARY_FILE) $(OBJECT_FILES)

$(LIBRARY_FILE): $(OBJECT_FILES)
	mkdir -p $(LIBRARY_DIR)
	ar $(LINKER_FLAGS) $@ $(OBJECT_FILES)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

