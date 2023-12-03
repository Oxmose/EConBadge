import bluetooth
import socket

import BtImage

print("Starting");

target_name = "Olson's ECB"
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
        print("INPUT")
        if text == "quit":
            break

        if text == "IMAGE_SEND":
            dataBytes = bytes(BtImage.btImage)

            print("Sending Image")
            i = 0
            while(i < 134400):
                s.send(dataBytes[i:min(i+8192, 134400)])
                i = i+8192

                data = s.recv(5)
                print(data);
                data = s.recv(2)
                print(data);

            print("Image Sent")
            data = s.recv(5)
            print(data);
            data = s.recv(7)
            print(data);
            continue

        elif text == "UPDATE":
            with open("firmware.bin", mode='rb') as file:
                fileContent = file.read()
                toSend = len(fileContent)
                offset = 0
                print("TO SEND " + str(toSend))
                while(toSend > 0):
                    effective = min(toSend, 8184)
                    # Send Magic
                    data = []
                    if effective != 8184:
                        data.append(160)
                        data.append(160)
                    else:
                        data.append(224)
                        data.append(224)

                    data.append(224)
                    data.append(224)

                    # Send size
                    dataBytes = bytearray(bytes(data))
                    totalSize = effective + 8
                    dataBytes.extend(totalSize.to_bytes(4, 'little'))
                    dataBytes.extend(fileContent[offset:offset + effective])

                    print("SENDING " + str(offset) + ":" + str(offset + effective) + " -> " + str(len(dataBytes)))

                    s.send(dataBytes)
                    data = s.recv(5)
                    print(data);
                    data = s.recv(2)
                    print(data);

                    offset += effective;
                    toSend -= effective;

                    print("LEFT TO SEND " + str(toSend))

        else:
            numbers = text.split(' ')

            data = []

            for i in range(68):
                if(len(numbers) <= i):
                    data.append(int(0))
                else:
                    if numbers[i] == ' ' or len(numbers[i]) == 0: continue
                    data.append(int(numbers[i]))

            dataBytes = bytes(data)
            s.send(dataBytes)

        data = s.recv(4)
        print(data);
        data = s.recv(1)
        size = int.from_bytes(data, "little")
        print(size)
        data = s.recv(size)
        print(data);

    s.close()