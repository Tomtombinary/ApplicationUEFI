CC=x86_64-w64-mingw32-gcc
CFLAGS=-ffreestanding
LDFLAGS=-nostdlib -Wl,-dll -shared -Wl,--subsystem,10
OBJS=hello.o data.o
INCLUDES=-I/usr/local/include/efi -I/usr/local/include/efi/x86_64 -I/usr/local/include/efi/protocol

LIBS=-lgcc
EXEC=BOOTX64.EFI
BOOTABLE=fat.img

all:$(BOOTABLE)

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -e efi_main -o $(EXEC) -L/usr/local/lib $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(BOOTABLE):$(EXEC)
	dd if=/dev/zero of=$(BOOTABLE) bs=1k count=1440
	mformat -i $(BOOTABLE) -f 1440 ::
	mmd -i $(BOOTABLE) ::/EFI
	mmd -i $(BOOTABLE) ::/EFI/BOOT
	mcopy -i $(BOOTABLE) $(EXEC) ::/EFI/BOOT


clean:
	rm -rf $(BOOTABLE)
	rm -rf $(EXEC)
	rm -rf *.o
	rm -rf *~

