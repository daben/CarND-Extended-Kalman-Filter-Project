import sys, os
import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
import pandas as pd

def visualize_output(filename):
    columns = ('px_est','py_est','vx_est','vy_est','px_meas','py_meas','px_gt','py_gt','vx_gt','vy_gt')
    df = pd.read_table(filename, sep='\t', header=None, names=columns, lineterminator='\n')
    
    fig = plt.figure(figsize=(8,5), dpi=300)
    # plt.gca().set_aspect('equal')
    plt.title(os.path.basename(filename))
    plt.grid("on")
    plt.plot(df.px_gt, df.py_gt, lw=.5, label='Ground Truth')
    plt.plot(df.px_meas, df.py_meas, '.', color='red', ms=3., alpha=.75, label='Measurements')
    plt.plot(df.px_est, df.py_est, color='g', lw=.5, label='Estimates')
    plt.legend(loc=0)
    plt.xlabel("px")
    plt.ylabel("py")
    return fig

for arg in sys.argv[1:]:
    fig = visualize_output(arg)
    output_file = os.path.splitext(arg)[0]+".png"
    fig.savefig(output_file)
    print(output_file)