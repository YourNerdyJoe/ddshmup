
TARGET = pvr_test.elf
OBJS = $(patsubst src/%.c, build/%.o, $(wildcard src/*.c)) $(patsubst src/dc/%.c, build/%.o, $(wildcard src/dc/*.c))

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

CFLAGS += -std=c2x

clean: rm-elf
	-rm -f $(OBJS)

rm-elf:
	-rm -f $(TARGET)

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

build/%.o: src/%.c
	kos-cc $(CFLAGS) $(CPPFLAGS) -c $< -o $@

build/%.o: src/dc/%.c
	kos-cc $(CFLAGS) $(CPPFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	kos-c++ $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/%.o: src/dc/%.cpp
	kos-c++ $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
