CFLAGS+=-I/usr/local/include
LDFLAGS+=-L/usr/local/lib -lrt -lzmq 

all: rtifsrd
rtifsrd: main.c
	cc main.c -o rtifsrd $(CFLAGS) $(LDFLAGS)

clean:
	@rm a.out
