import matplotlib.pyplot as plt
import scipy.optimize as opt;
import numpy as np;

INPUT_FILE = "profile.csv"
OUTPUT_FILE = "calibration.csv"

POLY_CURVE = 13

dataBase = []
times = []
volVal = []
timesPercent = []
profileCurve = None
profileCurveCal = None

def PrepareData():
    global times
    global volVal
    global timesPercent

    # Get raw data
    for data in dataBase:
        times.append(data[0])
        volVal.append(float(int(data[1] * 100)) / 100)

    maxTimes = max(times)
    for timeData in times:
        timesPercent.append(timeData * 100 / maxTimes)

def ReadProfile(inputFilename):
    global dataBase
    minTime = 0
    maxSkew = 0

    with open(inputFilename) as file:
        for line in file:
            data = line.replace('\n', '').replace('\r', '').replace(' ', '').split(',')
            time = int(float(data[2]))

            if len(dataBase) > 0:
                if(time <= dataBase[-1][0]):
                    print("Dataset not ordered!")
                    exit(1)

                if maxSkew < (time - minTime) - dataBase[-1][0]:
                    maxSkew = (time - minTime) - dataBase[-1][0]
                if (time - minTime > dataBase[-1][0] + 15):
                    print("WARNING: Detected skew in time: {}".format(time - minTime - dataBase[-1][0]))
                    print("         At {}".format(time))
            else:
                minTime = time

            dataBase.append([time - minTime, float(data[1])])

    print("Read profile, max skew {}".format(maxSkew))

def PlotData():
    # Plot Voltage
    steps = (max(times)) / 10
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(0, max(times), steps))

    fit = np.polyfit(times, volVal, POLY_CURVE)
    fit_fn = np.poly1d(fit)

    plt.plot(times, fit_fn(times))
    plt.scatter(times, volVal, alpha=0.002)

    plt.grid()
    plt.xlabel("Time (s)")
    plt.ylabel("Voltage (mV)")
    plt.draw()
    plt.pause(0.001)

    # Plot Voltage With percent
    steps = (max(timesPercent)) / 10
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(0, max(timesPercent), steps))

    fit = np.polyfit(timesPercent, volVal, POLY_CURVE)
    fit_fn = np.poly1d(fit)

    plt.plot(timesPercent, fit_fn(timesPercent))

    plt.scatter(list(reversed(timesPercent)), list(reversed(volVal)), alpha=0.002)
    plt.grid()
    plt.xlabel("Percent (%)")
    plt.ylabel("Voltage (mV)")
    plt.draw()
    plt.pause(0.001)



def ComputeCurves():
    global profileCurve

    # Compute the Voltage Inverse Curve
    fit = np.polyfit(timesPercent, volVal, POLY_CURVE)
    profileCurve = np.poly1d(fit)

def SaveData():
    with open(OUTPUT_FILE, 'w') as file:
        file.write("static const uint32_t skBatteryProfile[101] = {\n")
        endLine = 3
        for i in range(100, 0, -1):
            if endLine == 0:
                file.write("\n    ")
                endLine = 3
            else:
                file.write("    ")
            file.write("{} /* {:<4}% */,".format(int(profileCurve(i)), 100 - i))

            endLine -= 1
        file.write("\n};\n")

if __name__ == "__main__":
    print("==================================")
    print("#   EConBadge Battery Analyzer   #")
    print("==================================")

    ReadProfile(INPUT_FILE)
    PrepareData()
    ComputeCurves()
    PlotData()
    SaveData()

    print("> Analysis done");
    input("Press [enter] to continue.")
