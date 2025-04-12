
allocator:
	cc allocator.c -shared -fPIC -DLOGGER=0 -DTRACE_ON=0 -o allocator.so