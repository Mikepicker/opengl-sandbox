#OBJS specifies which files to compile as part of the project
OBJS = main.cpp glad.c

# Dependencies
STB = dep/stb_image/stb_image.cpp
IMGUI = dep/imgui/imgui.cpp
IMGUI_DRAW = dep/imgui/imgui_draw.cpp
IMGUI_IMPL = dep/imgui/imgui_impl_glfw_gl3.cpp

#CC specifies which compiler we're using
CC = g++ -std=c++14 -Wall -Wextra -Wpedantic -g

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
#  COMPILER_FLAGS = -w
#
#  #LINKER_FLAGS specifies the libraries we're linking against

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = cg
LINKER_FLAGS = -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor

#This is the target that compiles our executable
all: stb.o imgui.o imgui_draw.o imgui_impl.o linux

linux: $(OBJS)
	$(CC) $(OBJS) stb.o imgui.o imgui_draw.o imgui_impl.o $(COMPILER_FLAGS) -o $(OBJ_NAME) $(LINKER_FLAGS)

stb.o: $(STB)
	g++ -std=c++14 $(STB) -c -o stb.o

imgui.o: $(IMGUI)
	g++ -std=c++14 $(IMGUI) -c -o imgui.o

imgui_draw.o: $(IMGUI_DRAW)
	g++ -std=c++14 $(IMGUI_DRAW) -c -o imgui_draw.o

imgui_impl.o: $(IMGUI_IMPL)
	g++ -std=c++14 $(IMGUI_IMPL) -c -o imgui_impl.o

clean:
	rm -f $(OBJ_NAME)
