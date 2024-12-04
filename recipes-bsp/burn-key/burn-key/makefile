TARGET=burn_key

LDFLAGS= -lz

all: $(TARGET)
burn_key: burnkey.o
	$(CC) $(CFLAGS) -o $@ $^ 


clean:
	rm -f $(OBJS) $(TARGET)
