from matplotlib import pyplot as plt
from matplotlib.pyplot import figure
import numpy as np

def plot_row():
    area = 128
    Z = np.zeros((area, area))

    with open('log-xl-rowsize', 'r') as log:
        vals = log.readlines()[-1].split()
        for i, v in enumerate(vals):
            Z[i // area, i % area] = float(v)

    plt.pcolormesh( Z , cmap = 'coolwarm', shading='flat' )

    # Plot a colorbar with label.
    cb = plt.colorbar()
    cb.set_label('time per pair accesses (ns)')

    # Add title and labels to plot.
    plt.xlabel('KB x')
    plt.ylabel('KB y')
    plt.xticks(np.arange(0, area+1, 16))
    plt.yticks(np.arange(0, area+1, 16))

    # Show the plot.
    plt.show()

def plot_flip():
    Z = np.zeros((8*1024)) # byte
    a=[0,0]

    with open('log-5-single', 'r') as log:
        vals = log.readlines()[-2].split()
        for v in vals:
            Z[int(v) % (8*1024) ] += 1
            a[int(v) % (2*1024) // (1024) ] += 1

    # print(vals[0:10])
    # print(Z)
    Z = Z.reshape((1, 8*1024))
    print(a)

    # print(Z)

    # exit()
    plt.figure(figsize=(12, 4))
    plt.pcolormesh( Z , cmap = 'gist_yarg', shading='flat' )

    # Plot a colorbar with label.
    cb = plt.colorbar()
    cb.set_label('1-to-0 bit flips density')

    # Add title and labels to plot.
    plt.xlabel('offsets in a bank')
    plt.ylabel('flips')
    plt.yticks([])
    # plt.xticks(np.arange(0, 8193, 2048))
    # plt.yticks(np.arange(0, 65, 16))
    
    # Show the plot.
    plt.show()

if __name__ == "__main__":
    plot_row()
    # plot_flip()