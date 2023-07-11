ARMGNU ?= arm-none-eabi

AFLAGS = --warn --fatal-warnings
CFLAGS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding

# bespoke optimisation flags
RPI0 = -O2 -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

all: kernel.img

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.img

boot.o: boot.s
	$(ARMGNU)-as $(AFLAGS) boot.s -o boot.o

main.o: main.c
	$(ARMGNU)-gcc $(CFLAGS) $(RPI0) -c main.c -o main.o

main.elf: boot.ld boot.o main.o 
	$(ARMGNU)-ld boot.o main.o -T boot.ld -o main.elf

kernel.img: main.elf
	$(ARMGNU)-objcopy main.elf -O binary kernel.img

