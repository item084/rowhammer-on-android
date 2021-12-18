import matplotlib.pyplot as plt

def main():
    nops = []
    flips = []
    times = []
    with open("log-5-nop", "r") as lines:
        for line in lines:
            if "HAMMERING with" in line:
                nops.append(int(line.split()[-2])*2)
                times.append([])
            if "time-at" in line:
                times[-1].append(int(line.split()[-1]))
            if "Flipped:" in line:
                flips.append(int(line.split()[-1]))
    for i in range(len(times)):
        times[i].sort()
        times[i] = times[i][len(times[i])//2]
    print(len(nops), nops)
    print(len(times), times)
    print(len(flips), flips)

    fig, ax1 = plt.subplots()
    ax1.set_xlim([0, 120])
    ax1.set_ylim([0, 1400])
    color = 'tab:red'
    ax1.set_xlabel('# NOP instructions')
    ax1.set_ylabel('# Observed bit flips', color=color)
    ax1.plot(nops, flips, color=color, label = 'bit flips')
    ax1.tick_params(axis='y')#, labelcolor=color)
    ax1.legend(bbox_to_anchor=(0, 1), loc='upper left', ncol=1)
    ax1.grid(linestyle=':')

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylim([0, 220])
    color = 'tab:blue'
    ax2.set_ylabel('median access time per read (ns)', color=color)  # we already handled the x-label with ax1
    ax2.plot(nops, times, color=color, label = 'time per read')
    ax2.tick_params(axis='y')#, labelcolor=color)
    ax2.legend()
    # ax2.grid(linestyle=':')


    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.show()

    # https://matplotlib.org/stable/gallery/subplots_axes_and_figures/two_scales.html

if __name__ == "__main__":
    main()
