CXX=g++

all: simple simple_plugins multitype

clean:
	rm simple simple_plugins multitype

simple: simple.cpp
	$(CXX) -o simple simple.cpp \
		/usr/local/lib/libwiztoolkit.a \
		/usr/local/lib/libstealth_logging.a \
		-lgmp

simple_plugins: simple_plugins.cpp
	$(CXX) -o simple_plugins simple_plugins.cpp \
		/usr/local/lib/libwiztoolkit.a \
		/usr/local/lib/libstealth_logging.a \
		-lgmp

multitype: multitype.cpp
	$(CXX) -o multitype multitype.cpp \
		/usr/local/lib/libwiztoolkit.a \
		/usr/local/lib/libstealth_logging.a \
		-lgmp
