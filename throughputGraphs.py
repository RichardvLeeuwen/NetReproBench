import subprocess
import argparse
import time
import os
import shutil
import matplotlib.pyplot as plt
import glob
from os import walk
from scipy.stats import shapiro
from scipy.stats import normaltest
from statsmodels.graphics.gofplots import qqplot
import numpy as np

from matplotlib.font_manager import FontProperties


def main():
    path = "/home/richard/CloudRes/"
    pathPlots = "/home/richard/CloudPlots/"
    filenames = os.listdir(path)

    transmitterNodeNames = []

    for name in filenames:
        newName = name.split(">")
        if newName[0] in transmitterNodeNames:
            continue
        else:
            transmitterNodeNames.append(newName[0])
    print(transmitterNodeNames)

    tp = []
    for node in transmitterNodeNames: #modify to pop first 2 val
        nodeFile = node + ">"
        plt.figure()
        for name in filenames:
            nodeData = []
            if nodeFile in name:
                filePath = path + name
                f = open(filePath, "r")
                while True:
                    line = f.readline()
                    if not line:
                        break
                    nodeData.append(float(line))
                for i in range(len(nodeData)):
                    s1 = nodeData[i]
                    if i == 0:
                        tp.append(s1)
                    else:
                        s2 = nodeData[i-1]
                        s3 = s1 - s2
                        tp.append(s3)

    plt.hist(tp)
    plt.show()
    plt.close()



if __name__ == "__main__":
    main()
