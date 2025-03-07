import matplotlib.pyplot as plt
import scipy.optimize as opt;
import numpy as np;

INPUT_FILE = "profile.csv"
OUTPUT_FILE = "calibration.csv"

POLY_CURVE = 8

dataBase = []
times = []
volVal = []
volSteps = []
voltPercent = []
voltPercentCal = [0 for i in range(101)]
profileCurve = None
profileCurveCal = None

def PrepareData():
    global times
    global volVal
    global volSteps

    # Get raw data
    for data in dataBase:
        times.append(data[0])
        volVal.append(float(int(data[1] * 100)) / 100)

    # Get 1% steps
    steps = (len(volVal)) // 100
    i = 0
    tableIdx = 0
    valueVol = 0
    for j in range(len(volVal)):
        valueVol += volVal[j]
        voltPercentCal[tableIdx] = max(voltPercentCal[tableIdx], volVal[j])
        if i == steps:
            tableIdx += 1
            volSteps.append(valueVol / (steps + 1))
            valueVol = 0
            i = 0
        else:
            i += 1
    if valueVol != 0:
        volSteps.append(valueVol / i)

    for i in range(len(voltPercentCal)):
        if voltPercentCal[i] == 0:
            voltPercentCal[i] = voltPercentCal[i - 1]

def ReadProfile(inputFilename):
    global dataBase
    minTime = 0

    with open(inputFilename) as file:
        for line in file:
            data = line.replace('\n', '').replace('\r', '').replace(' ', '').split(',')
            time = int(float(data[2]))
            if len(dataBase) > 0:
                if(time <= dataBase[-1][0]):
                    print("Dataset not ordered!")
                    exit(1)
                if (time - minTime > dataBase[-1][0] + 20):
                    print("WARNING: Detected skew in time: {}".format(time - minTime - dataBase[-1][0]))
                    print("\tAt {}".format(time))
            else:
                minTime = time

            dataBase.append([time - minTime, float(data[1])])

def func(x, a, b, c):
     return a * np.exp(-b * x) + c

def PlotData():
    steps = (max(times)) / 10

    # Plot Voltage
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(0, max(times), steps))
    plt.scatter(times, volVal)
    plt.grid()
    plt.xlabel("Time (s)")
    plt.ylabel("Voltage (v)")
    plt.draw()
    plt.pause(0.001)



    # Plot normalized voltage
    xnorm = [i for i in range(len(volSteps), 0, -1)]
    fit = np.polyfit(xnorm, volSteps, POLY_CURVE)
    fit_fn = np.poly1d(fit)
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(0, 100, 10))
    plt.scatter(xnorm, volSteps, alpha=0.2)
    plt.plot(xnorm, fit_fn(xnorm))
    plt.grid()
    plt.xlabel("Percentage (%)")
    plt.ylabel("Voltage (v)")
    plt.draw()
    plt.pause(0.001)

    # Plot Percentage
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(min(voltPercent), max(voltPercent), 1000))
    plt.plot(voltPercent, [i for i in range(101)])
    plt.grid()
    plt.xlabel("Voltage (v)")
    plt.ylabel("Percentage (%)")
    plt.draw()
    plt.pause(0.001)

    # Plot Percentage calibrated
    fig = plt.figure()
    ax = fig.gca()
    ax.set_xticks(np.arange(0, 100, 10))
    fit = np.polyfit([i for i in range(len(voltPercentCal))], voltPercentCal, POLY_CURVE)
    fit_fn = np.poly1d(fit)
    plt.scatter([i for i in range(len(voltPercentCal))], np.flip(voltPercentCal), alpha=0.2)
    plt.plot([i for i in range(101)], np.flip(fit_fn([i for i in range(101)])))
    plt.grid()
    plt.ylabel("Voltage (v)")
    plt.xlabel("Percentage (%)")
    plt.draw()
    plt.pause(0.001)

def ComputeCurves():
    global voltPercent
    global profileCurve
    global profileCurveCal

    # Compute the Voltage Inverse Curve
    xnorm = [i for i in range(len(volSteps), 0, -1)]
    fit = np.polyfit(xnorm, volSteps, POLY_CURVE)
    profileCurve = np.poly1d(fit)
    for i in range(101):
        voltPercent.append(profileCurve(i))

    xnorm = [i for i in range(len(voltPercentCal), 0, -1)]
    fit = np.polyfit(xnorm, voltPercentCal, POLY_CURVE)
    profileCurveCal = np.poly1d(fit)

def SaveData():
    with open(OUTPUT_FILE, 'w') as file:
        file.write("static const uint32_t skBatteryProfile[101] = {\n")
        for i in range(101):
            if i != 0 and i % 3 == 0:
                file.write("\n    ")
            else:
                file.write("    ")
            file.write("{} /* {:<4}% */,".format(int(profileCurve(i) * 1000), i))
        file.write("\n};\n")
        file.write("\n\nstatic const uint32_t skBatteryProfile[101] = {\n")
        for i in range(101):
            if i != 0 and i % 3 == 0:
                file.write("\n    ")
            else:
                file.write("    ")
            file.write("{} /* {:<4}% */,".format(int(voltPercentCal[-i-1] * 1000), i))
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
