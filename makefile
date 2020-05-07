

TARGET =doom
TARGET_DIR =target
OUTPUT_DIR =build
SDL_DIR =lib/SDL-1.2.15

CSRCS = \
$(wildcard src/*.c) \
$(TARGET_DIR)/i_main.c \
$(TARGET_DIR)/i_system.c \
$(TARGET_DIR)/i_video_sdl.c \
$(TARGET_DIR)/i_sound_sdl.c \
$(TARGET_DIR)/i_net.c \

INCLUDES =$(SDL_DIR)/include dev/inc inc

GCFLAGS =-O3 -m32
LDFLAGS =-L$(SDL_DIR)/lib/x86 -m32
LIBS =-lSDL -lSDLmain -lSDL_mixer -lpthread -lm -ldl # -lc

OBJECTS =$(addprefix $(OUTPUT_DIR)/, $(notdir $(CSRCS:.c=.o)))

GCC =$(PREFIX)gcc
VPATH +=$(dir $(CSRCS))

all: $(TARGET)

_disco:
	@echo "disco"	
	"$(MAKE)" -C target/disco BUILD_DIR=$(OUTPUT_DIR) flash-openocd

$(TARGET): $(OBJECTS)
	@echo "--- Linking ---" $@
	$(GCC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET) data/doom.wad

$(OUTPUT_DIR):
	mkdir -p $@

$(OUTPUT_DIR)/%.o : %.c | $(OUTPUT_DIR)
	@echo "--- Compile" $< "---->" $@
	$(GCC) $(GCFLAGS) $(SYMBOLS) $(WARNINGS) $(addprefix -I, $(INCLUDES)) -c $< -o $@
