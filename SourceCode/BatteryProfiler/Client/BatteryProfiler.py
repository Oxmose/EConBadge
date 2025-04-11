import urllib.request
import time
import os
import datetime

SERVER_URL = "http://192.168.4.1"
OUTPUT_FILE = "profile.csv"

measureId = 0

def GetContent():
    global measureId
    try:
        contents = urllib.request.urlopen(SERVER_URL).read().decode("utf-8")
        print("Content: " + contents)
        profileData = contents.replace(' ', '').replace('\n', '').replace('\r', '')

        with open(OUTPUT_FILE, 'a') as file:
            file.write("{}, {}, {}, {}\n".format(measureId, profileData, time.time(), datetime.datetime.now().time()))

        measureId += 1

        return True
    except Exception as inst:
        print("> Lost contact with the server")
        print(inst)
        return False

if __name__ == "__main__":
    print("==================================")
    print("#   EConBadge Battery Profiler   #")
    print("==================================")

    while True:
        retVal = GetContent()
        if not retVal:
            break
        time.sleep(10)

    print("> Profiling done");
