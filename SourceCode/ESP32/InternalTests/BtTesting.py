import bluetooth
import socket

import BtImage

print("Starting");

target_name = "ECB-9c9ef0c8"
target_address = None
nearby_devices = bluetooth.discover_devices(lookup_names=True,lookup_class=True)
print(nearby_devices)
for btaddr, btname, btclass in nearby_devices:
    if target_name == btname:
        target_address = btaddr
        break
if target_address is not None:
    serverMACAddress = target_address
    port = 1
    s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    s.connect((serverMACAddress,port))
    print("connected to {}".format(target_name))
    while 1:
        text = input()
        if text == "quit":
            break

        if text != "IMAGE_SEND":
            numbers = text.split(' ')

            data = []

            for number in numbers:
                data.append(int(number))

            dataBytes = bytes(data)
            s.send(dataBytes)

            data = s.recv(1024)
            if data:
                print(data)
        else:
            dataBytes = bytes(BtImage.btImage)

            print("Sending Image")
            i = 0
            while(i < 134400):
                s.send(dataBytes[i:min(i+8192, 134400)])
                i = i+8192

                data = s.recv(3)
                print(data);

            print("Image Sent")

    s.close()