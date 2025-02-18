import bluetooth
import socket

import BtImage

print("Starting");

target_name = "POWER_CONS"
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
    data = [i % 256 for i in range (8192)]
    dataBytes = bytes(data)

    while 1:
        s.send(dataBytes[0:8192])
        print("Sent")
        data = s.recv(8192)
        print("RECV")
    s.close()