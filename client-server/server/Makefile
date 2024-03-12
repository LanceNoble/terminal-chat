LINKFLAGS = -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64" -lws2_32
objects = server.o

server : $(objects)
	gcc -o server $(objects) $(LINKFLAGS)

server.o : server.c
	gcc -c server.c
