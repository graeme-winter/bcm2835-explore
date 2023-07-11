import serial

dev = "/dev/tty.usbmodem14232202"

uart = serial.Serial(dev, baudrate=115200)

message = bytearray([0, 1, 0, 0]) + bytearray(range(256))

uart.write(message)
data = uart.read(4)
print(data)
