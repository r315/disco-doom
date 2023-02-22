

TARGET =doom
TARGET_DIR =x86
TARGET_PATH =$(TARGET_DIR)
DOOM_ROOT_PATH =../..
OUTPUT_DIR =$(DOOM_ROOT_PATH)/build/$(TARGET_PATH)
SDL_DIR =$(HOME)/Documents/SDL-1.2.15


CSRCS = \
$(wildcard $(DOOM_ROOT_PATH)/src/*.c) \
$(TARGET_DIR)/i_main.c \
$(TARGET_DIR)/i_video_sdl.c \
$(TARGET_DIR)/i_sound_sdl.c \
$(TARGET_DIR)/i_net.c \

INCLUDES =$(SDL_DIR)/include $(DOOM_ROOT_PATH)/inc

GCFLAGS =-O0 -m32
LDFLAGS =-L"$(SDL_DIR)/lib/x86" -m32

ifeq ($(shell uname -s), Linux)
LIBS =-lSDL -lSDLmain -lpthread -lm -ldl -static
else
LIBS =-mwindows -lmingw32 -lSDLmain -lSDL -lSDL_mixer -lpthread -lm
endif

OBJECTS =$(addprefix $(OUTPUT_DIR)/, $(notdir $(CSRCS:.c=.o)))

GCC =$(PREFIX)gcc
VPATH +=$(dir $(CSRCS))
LD =$(PREFIX)gcc

all: $(TARGET)


$(TARGET): $(OBJECTS)
	@echo "--- Linking ---" $@
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	rm -r $(OUTPUT_DIR)

run: $(TARGET)
	./$(TARGET) data/doom.wad

$(OUTPUT_DIR):
	mkdir -p $@

$(OUTPUT_DIR)/%.o : %.c | $(OUTPUT_DIR)
	@echo "CC" $< "---->" $@
	@$(GCC) $(GCFLAGS) $(SYMBOLS) $(WARNINGS) $(addprefix -I, $(INCLUDES)) -c $< -o $@

.PHONY:
