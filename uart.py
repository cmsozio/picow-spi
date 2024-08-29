import serial, sys, time, mmap, json, random

if __name__ == "__main__":
    b = random.randbytes(256)
    BAUD = 115200

    ser = serial.Serial('/dev/tty.usbserial-0001', BAUD, bytesize=8, parity='N', stopbits=1)
    
    if (ser.in_waiting > 0):
        rx = ser.read(ser.in_waiting)
        print(rx.hex())

    ser.write(b)

    s = ser.read(256)
    print("Data: {}\n".format(s.hex()))
    ser.close()
