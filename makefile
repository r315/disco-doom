

ifeq ("$(TARGET)", "disco")
#PREFIX=arm-linux-gnueabi-
PREFIX=arm-none-eabi-
GCFLAGS =-Os -D_NEWLIB_
TARGET =disco_doom.elf
LIBS =-lc #-specs=nosys.specs
PLATFORM_PATH =target/disco
else
TARGET =doom
PLATFORM_PATH =target
SDL =lib/SDL-1.2.15

GCFLAGS =-O3 -m32
LDFLAGS =-L$(SDL)/lib/x86 -m32
LIBS =-lSDL -lSDLmain -lSDL_mixer -lpthread -lm -ldl # -lc
endif


OUTPUT_DIR =build
GCC =$(PREFIX)gcc

CSRCS =$(wildcard src/*.c) \
$(PLATFORM_PATH)/i_main.c \
$(PLATFORM_PATH)/i_system.c \
$(PLATFORM_PATH)/i_video_sdl.c \
$(PLATFORM_PATH)/i_sound_sdl.c \
$(PLATFORM_PATH)/i_net.c \

INCLUDES =$(SDL)/include dev/inc inc

OBJECTS =$(addprefix $(OUTPUT_DIR)/, $(notdir $(CSRCS:.c=.o)))

VPATH +=$(dir $(CSRCS))

ifeq ("$(TARGET)", "disco")
all:	
	"$(MAKE)" -C $(PLATFORM_PATH) BUILD_DIR=$(OUTPUT_DIR)

else
all: $(TARGET)
endif

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
