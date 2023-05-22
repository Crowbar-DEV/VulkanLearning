CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

triangle: main.cpp
	g++ $(CFLAGS) -o triangle main.cpp $(LDFLAGS)

.PHONY: test clean

test: triangle
	./triangle

clean:
	rm -f triangle
