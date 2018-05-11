#OBJS specifies which files to compile as part of the project
OBJS = main.cpp glad.c dep/stb_image/stb_image.cpp dep/imgui/*.cpp

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
linux : $(OBJS)
		$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(OBJ_NAME) $(LINKER_FLAGS)
