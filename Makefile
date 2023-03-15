.PHONY: all
all: main


BUILD_DIR := build
BIN_DIR   := bin

mkdir_build:
	mkdir -p $(BUILD_DIR)
mkdir_bin:
	mkdir -p $(BIN_DIR)


asound_LINK := -lasound

wsp_OBJS 		:= wav_read.o random.o
wsp_SRCS 		:= $(patsubst %.o, %.c, $(wsp_OBJS))
wsp_OBJS 		:= $(patsubst %, $(BUILD_DIR)/%, $(wsp_OBJS))

wsp_INCLUDE_DIR := -Iinclude

$(wsp_OBJS): $(wsp_SRCS) mkdir_build
	$(CC) -fPIC -o $@ -c $(patsubst $(BUILD_DIR)/%.o,%.c,$@) $(wsp_INCLUDE_DIR)

wsp_LIB 		:= $(BIN_DIR)/libwsp.so
wsp_LINKS 	:= $(asound_LINK)

$(wsp_LIB): $(wsp_OBJS) mkdir_bin
	$(LD) -shared -o $@ $(wsp_OBJS) $(wsp_LINKS)


wsp_LINK := -L$(BIN_DIR) -lwsp

main_LINKS 			:= $(wsp_LINK) $(asound_LINK)
main_INCLUDE_DIRS 	:= $(wsp_INCLUDE_DIR)
main_LDFLAGS 		:= -Wl,-R,$(shell pwd)/$(BIN_DIR)
main_FLAGS 			:= $(main_LINKS) $(main_INCLUDE_DIRS) $(main_LDFLAGS)

main_SRCS 			:= main.c
main_EXE 			:= $(BIN_DIR)/wav_sound_player

$(main_EXE): $(wsp_LIB) $(main_SRCS) mkdir_bin
	$(CC) -o $@ $(main_SRCS) $(main_FLAGS)

.PHONY: main
main: $(main_EXE)

.PHONY: clean
clean:
	rm -r bin build
