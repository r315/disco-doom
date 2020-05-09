

TARGET =doom
TARGET_DIR =target
OUTPUT_DIR =build
SDL_DIR =lib/SDL-1.2.15

CSRCS = \
$(wildcard src/*.c) \
$(TARGET_DIR)/i_main.c \
$(TARGET_DIR)/i_video_sdl.c \
$(TARGET_DIR)/i_sound_sdl.c \
$(TARGET_DIR)/i_net.c \

INCLUDES =$(SDL_DIR)/include dev/inc inc

GCFLAGS =-O0 -m32
LDFLAGS =-L"$(SDL_DIR)/lib/x86" -m32

ifeq ($(shell uname -s), Linux)
LIBS =-lSDL -lSDLmain -lSDL_mixer -lpthread -lm -ldl
else
LIBS =-mwindows -lmingw32 -lSDLmain -lSDL -lSDL_mixer -lpthread -lm
endif

OBJECTS =$(addprefix $(OUTPUT_DIR)/, $(notdir $(CSRCS:.c=.o)))

GCC =$(PREFIX)gcc
VPATH +=$(dir $(CSRCS))
LD =$(PREFIX)gcc

all: $(TARGET)

disco:
	@echo "disco"	
	"$(MAKE)" -C target/disco $(ARGS)

$(TARGET): $(OBJECTS)
	@echo "--- Linking ---" $@
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS) $(TARGET)
	"$(MAKE)" -C target/disco clean

run: $(TARGET)
	./$(TARGET) data/doom.wad

$(OUTPUT_DIR):
	mkdir -p $@

$(OUTPUT_DIR)/%.o : %.c | $(OUTPUT_DIR)
	@echo "CC" $< "---->" $@
	@$(GCC) $(GCFLAGS) $(SYMBOLS) $(WARNINGS) $(addprefix -I, $(INCLUDES)) -c $< -o $@

.PHONY: disco clean