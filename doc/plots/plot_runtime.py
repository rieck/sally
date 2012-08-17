import numpy as np
import matplotlib.pyplot as plt

prefix = '/Users/rieck/Work/data/sally/runtime/results02'
datasets = ['enron', 'sprot']
nlen = 5
num = 5000

for data in datasets:

    plt.clf(); plt.figure(figsize=(3.8,3))
    plt.subplots_adjust(left=0.22, right=0.95, top=0.9, bottom=0.17)

    dfile = '%s/%s-%d.log' % (prefix, data, nlen)
    m = np.loadtxt(dfile).T
    x = m[0][1:num]
    y = (m[1][1:num] + m[2][1:num]) * 1000

    plt.hold('on')
    plt.loglog(x, y, '.', color=(0.6,0.6,0.6), label = '_nolegend_')

    if data == 'sprot':
        plt.xlim((1e1, 1e4))
        plt.ylim((1e-3, 1e1))
    else:
        plt.xlim((1e1, 1e4))
        plt.ylim((1e-3, 1e1))

    plt.xlabel('Size of strings')
    plt.ylabel('Run-time (ms)')
    plt.grid('on')

    X = np.c_[np.log(x),np.ones(len(x))]
    y = np.log(y)

    # Estimate
    C = np.dot(X.T,X)
    b = np.dot(np.dot(np.linalg.inv(C), X.T), y)
    s = np.dot(X,b)

    i = np.argsort(x)
    x = x[i]; s = s[i]

    plt.loglog(x, np.exp(s), 'k', linewidth=1.5)

    mx = np.mean(x)
    my = np.mean(np.exp(y))

    plt.loglog(mx, my, 'o', markeredgewidth=1.5, color = 'w', markersize=10)

    plt.legend(('Estimate', 'Average'), 'upper left', numpoints = 1)
    leg = plt.gca().get_legend()
    ltext = leg.get_texts()
    plt.setp(ltext, fontsize='small')

    plt.savefig('%s.pdf' % data)