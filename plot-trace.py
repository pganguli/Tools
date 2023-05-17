import subprocess

import matplotlib
from matplotlib import pyplot as plt

from power_trace import parse_power_script, trace_to_plot

matplotlib.rcParams.update({
    'font.size': 16,
})

def main():
    script_names = [
        'script-solar.csv',
        'script-thermal.csv',
        'script-rf.csv',
    ]
    ylim = [6, 6, 20]
    #yticks = [
    #    range(5),
    #    range(5),
    #    range(0, 20, 5),
    #]
    labels = [
        'Solar', 'Thermal', 'RF',
    ]

    fig, axs = plt.subplots(len(script_names), 1)
    fig.set_size_inches(12, 9)

    for script_idx, script_name in enumerate(script_names):
        normalized_power_trace = parse_power_script(script_name, normalized_average_current=0.004)

        x, y = trace_to_plot(normalized_power_trace)

        ax = axs[script_idx]
        #ax.plot(x, y, color='black')
        ax.fill_between(x, y)
        #ax.text(4, ylim[script_idx] * 0.4, labels[script_idx])

        ax.set_xlim(0, 60)
        ax.set_ylim(0, ylim[script_idx])
        #ax.set_yticks(yticks[script_idx])
        if script_idx == len(script_names) - 1:
            ax.set_xlabel('Time (s)')
        else:
            ax.set_xticks([])
        ax.set_ylabel('Power (mW)')
        ax.yaxis.set_label_coords(-0.05, 0.5)

    plt.subplots_adjust(left=0.1, bottom=0.2, right=0.95, top=0.95,
                        hspace=0.15, wspace=0)

    plt.savefig('dynamic.pdf')
    subprocess.check_call(['pdfcrop', 'dynamic.pdf', 'dynamic-cropped.pdf'])

if __name__ == '__main__':
    main()
