import sys
import struct

# TO UPDATE WITH YOUR DESIRED VALUES (ACCORDING TO THE PALETTE YOU USED TO
# CONVERT YOUR IMAGE).
# The default values are based on the WaveShareTable.act palette
USED_PALETTE = dict([
    (0x000000, 0),
    (0xFFFFFF, 1),
    (0x4b6e54, 2),
    (0x37436a, 3),
    (0xa4504b, 4),
    (0xdccc5f, 5),
    (0xc06650, 6)
])

DYN_PALETTE_TABLE = [0 for i in range(7)]

width = 0
height = 0

def ValidateHeader(fileContent):
    global width
    global height
    # Check header:
    # Type: BMP 8 Bits
    # Size: 600x448 or 448x600
    # Colors
    # No compression

    # Check Magic
    magic = fileContent[0:2].decode()
    if(magic != "BM"):
        print("Wrong BMP magic, expected BM, got " + magic)
        return False

    # Check size
    width = struct.unpack('I', fileContent[18:22])[0]
    height = struct.unpack('I', fileContent[22:26])[0]
    bpp = struct.unpack('H', fileContent[28:30])[0]
    comp = struct.unpack('I', fileContent[30:34])[0]
    paletteSize = struct.unpack('I', fileContent[46:50])[0]

    startBitmap = struct.unpack('I', fileContent[10:14])[0]
    startPalette = startBitmap - paletteSize * 4

    valid =  width == 600 and height == 448 and bpp == 8 and comp == 0 and paletteSize == 7

    print("Header: " + magic)
    print("Width: " + str(width))
    print("Height: " + str(height))
    print("Bits per pixel: " + str(bpp))
    print("Palette size: " + str(paletteSize))
    print("Bitmap start: " + str(startBitmap))
    print("Palette start: " + str(startPalette))

    return valid, startBitmap, startPalette

def ContructPalette(fileContent, startPalette):
    for i in range(7):
        color = struct.unpack('I', fileContent[startPalette + i * 4: startPalette + (i + 1) * 4])[0]
        if color not in USED_PALETTE:
            print("Unknown color " + str(color) + ", please update the palette in the python script.")
            exit(1)

        DYN_PALETTE_TABLE[i] = USED_PALETTE[color]
    return True

def ConvertImage(fileContent, startOffset, outputFileName, mode):
    global width
    global height
    if mode == "c":
        with open(outputFileName, 'w') as outputFile:
            outputFile.write("const unsigned char Image7color[134400] = {")

            for i in range(448):
                for j in range(600):
                    if j % 30 == 0:
                        outputFile.write("\n\t")

                    if(j % 2 == 0):
                        outputFile.write("0x")
                    color = struct.unpack('B', fileContent[startOffset + (447 - i) * 600 + j: startOffset + (447 - i) * 600 + j + 1])[0]
                    outputFile.write(str(DYN_PALETTE_TABLE[color]))

                    if(j % 2 == 1):
                        outputFile.write(", ")
            outputFile.write("\n};\n")
    elif mode == "b":
        with open(outputFileName, 'wb') as outputFile:
            currValue = 0
            for i in range(448):
                for j in range(600):
                    color = struct.unpack('B', fileContent[startOffset + (447 - i) * 600 + j: startOffset + (447 - i) * 600 + j + 1])[0]

                    if(j % 2 == 1):
                        currValue |= DYN_PALETTE_TABLE[color] << 4
                        outputFile.write(bytearray([currValue]))
                        #print(bytearray([currValue]))
                    else:
                        currValue = DYN_PALETTE_TABLE[color]
    else:
        print("Mode shall either be b or c.")

    return True;

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: " + sys.argv[0] + " [input_filename] [output_filename] [mode]")

    inputFileName = sys.argv[1]
    outputFileName = sys.argv[2]
    mode = sys.argv[3]

    with open(inputFileName, mode = 'rb') as inputFile:
        fileContent = inputFile.read();

        valid, startOffset, startPalette = ValidateHeader(fileContent)
        if(valid):
            print("====> Bitmap is valid")
        else:
            print("====> Bitmap is invalid")
            exit(1)

        ContructPalette(fileContent, startPalette)
        print("Palette => " + str(DYN_PALETTE_TABLE))

        ConvertImage(fileContent, startOffset, outputFileName, mode)
