import subprocess
import argparse
import time
import os
import shutil
import matplotlib.pyplot as plt
import glob
from os import walk
import numpy as np

from matplotlib.font_manager import FontProperties


def main():
    path = "/home/richard/cloudBenchCPU/"
    pathPlots = "/home/richard/CloudPlots/"
    allStr = "all"
    txtStr = ".txt"
    filenames = os.listdir(path)

    transmitterNodeNames = []

    for name in filenames:
        newName = name.split(":")
        if newName[0] in transmitterNodeNames:
            continue
        else:
            transmitterNodeNames.append(newName[0])
    print(transmitterNodeNames)

    for node in transmitterNodeNames:
        completePath = path + node
        f = open(completePath, "r")
        labels = []
        nodeData = []
        while True:
            line = f.readline()
            if not line:
                break
            splitted = line.split(":")
            name = splitted[0]
            usage = splitted[1]
            labels.append(str(name))
            nodeData.append(float(usage))
        X_axis = np.arange(len(labels))
        plt.figure(figsize=(15,6) )
        plt.bar(X_axis + 0.2, nodeData, label = 'CPU%')
        plt.ylim(0, 100)
        plt.xticks(X_axis, labels)
        plt.xlabel("Cores")
        plt.ylabel("CPU %")
        name = node[:-4]
        plt.title("CPU Usage for " + name)
        plt.savefig(pathPlots+name+".png")
        plt.close()


if __name__ == "__main__":
    main()
