EXE 		= obj-render

SRC 		= $(wildcard src/*.cpp)
OBJ 		= $(subst src, build, $(patsubst %.cpp, %.o, $(SRC))) build/cow.o

DBG 		= 	# debug flags

INCLUDE 	= -I ../include
# LIB 		= -L ../lib/SFML -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 	# this is for linking SFML with shared libraries
LIB 		= -L ../lib_static/SFML -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32	# this is for static linking SFML
EXTRA		=  # for any extra flags you might want to insert
STATIC  	= -static-libgcc -static-libstdc++ -static 	# for static linking with libgcc and libstdc++ and more

all: link

remake: clean all

image/%:	# if you ever need to link an image
	objcopy -I binary -O elf64-x86-64 obj/$*.obj build/$*.o

run:
	$(EXE)

clean:
	erase $(subst build/, build\, $(OBJ))

mkbuild:
	mkdir build

build/%.o: src/%.cpp
	g++ $(INCLUDE) -c src/$*.cpp -o build/$*.o $(DBG)

link: $(OBJ)
	g++ $(OBJ) -o $(EXE) $(LIB) $(EXTRA) $(STATIC) $(DBG)
