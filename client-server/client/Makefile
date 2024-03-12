LINKFLAGS = -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64" -lws2_32
objects = client.o

client : $(objects)
	gcc -o client $(objects) $(LINKFLAGS)

client.o : client.c
	gcc -c client.c
