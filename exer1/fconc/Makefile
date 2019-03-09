OBJS = fconc.o defs.o
TARGET = fconc

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $@

# general rule
%.o: %.c
	gcc $< -o $@ -c -Wall

########################
#  ___ optional ___    #
########################

TARGET2=fconc2
OBJS2 = fconc2.o defs.o

$(TARGET2): $(OBJS2)
	gcc $(OBJS2) -o $@

clean: clean-objs
	rm -f $(TARGET) $(TARGET2)

clean-objs:
	rm -f $(OBJS) $(OBJS2)

