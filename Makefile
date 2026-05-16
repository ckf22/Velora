TARGET_EXEC := app

BUILD_DIR := ./build
SRC_DIRS := ./src
SHADER_DIR := ./shaders


SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LIBS := /usr/lib/x86_64-linux-gnu/libglfw.so* /usr/lib/x86_64-linux-gnu/libvulkan*

CPPFLAGS := $(INC_FLAGS) -MMD -MP

VERT_SRCS := $(shell find $(ShADER_DIR) -type f -name '*.vert')
VERT_OBJS := $(patsubst %.vert, %.vert.spv, $(VERT_SRCS))

FRAG_SRCS := $(shell find $(ShADER_DIR) -type f -name '*.frag')
FRAG_OBJS := $(patsubst %.frag, %.frag.spv, $(FRAG_SRCS))

$(BUILD_DIR)/$(TARGET_EXEC): $(VERT_OBJS) $(FRAG_OBJS)

# shader compiling
%.spv: %
	glslc $< -o $@

# linking
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) 
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) -L $(LIBS)

# c++ compiling
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	rm -f $(SHADER_DIR)/*.spv

run:
	./build/$(TARGET_EXEC)

touch:
	touch $(SRCS)
	touch $(VERT_SRCS) $(FRAG_SRCS)

app:
	$(CXX) $(INC_FLAGS) $(CXXFLAGS) $(SRCS) -O3 -o app -L $(LIBS)



-include $(DEPS)