ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings
COPS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding

all: kernel.img

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.img

boot.o: boot.s
	$(ARMGNU)-as $(AOPS) boot.s -o boot.o

main.o: main.c
	$(ARMGNU)-gcc $(COPS) -c main.c -o main.o

nop.o: nop.c
	$(ARMGNU)-gcc $(COPS) -c nop.c -o nop.o

main.elf: boot.ld boot.o main.o nop.o
	$(ARMGNU)-ld boot.o main.o nop.o -T boot.ld -o main.elf

kernel.img: main.elf
	$(ARMGNU)-objcopy main.elf -O binary kernel.img

