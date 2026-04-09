TARGET_EXEC := app

BUILD_DIR := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LIBS := /usr/lib/x86_64-linux-gnu/libglfw.so* /usr/lib/x86_64-linux-gnu/libvulkan*

CPPFLAGS := $(INC_FLAGS) -MMD -MP -O1

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

run:
	./build/$(TARGET_EXEC)

touch:
	touch $(SRCS)


-include $(DEPS)