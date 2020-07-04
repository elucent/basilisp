CXX := g++
CXXFLAGS := -Iinclude -std=c++14

SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst %.cpp,%.o,${SRCS})

OUTPUT := basilisp

clean:
	rm -f ${OBJS} ${OUTPUT}

debug: CXXFLAGS += "-g3"
release: CXXFLAGS += "-Os"

debug: ${OBJS}
	${CXX} ${CXXFLAGS} $^ -o ${OUTPUT}

release: ${OBJS}
	${CXX} ${CXXFLAGS} $^ -o ${OUTPUT}

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@