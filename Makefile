CXX=g++
CXXFLAGS= -c `sdl2-config --cflags` -std=c++11
INCLUDES= -Iinclude
LFLAGS= `sdl2-config --libs` -lGLEW -lGL
BUILDDIR=build
SRCDIR=src
SRC=$(wildcard $(SRCDIR)/*.cpp)
_OBJ=$(SRC:.cpp=.o)
OBJ=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(_OBJ))
TARGET=prac1
TARGETPATH=$(BUILDDIR)/$(TARGET)
RESOURCES_DIR=resources
RESOURCES=$(wildcard $(RESOURCES_DIR)/*)

build:	$(OBJ) $(TARGET) copy_resources

run:
		cd $(BUILDDIR); ./$(TARGET)

$(TARGET): $(OBJ)
		$(CXX) $(OBJ) -o $(TARGETPATH) $(LFLAGS)

copy_resources:
		@cp -r $(RESOURCES) $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
		$(CXX) $(INCLUDES) $(CXXFLAGS) $< -o $@

clean:
		rm -f $(TARGETPATH)
		rm -f $(OBJ)
