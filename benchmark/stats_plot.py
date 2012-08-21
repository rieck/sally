
import numpy as np
import matplotlib.pyplot as plt

datasets = [ 'enron', 'sprot' ]
sample = 1000

for dataset in datasets:
    plt.clf(); 
    plt.figure(figsize=(4.5, 3))

    dfile = 'results/sally-%s.stats.gz' % dataset
    x = np.loadtxt(dfile, usecols=(1, 3))
    
    # Add different times (read, embed, write)
    t = np.zeros(x.shape[0] / 3)
    l = np.zeros(x.shape[0] / 3)
    for i in range(x.shape[0] / 3):
        l[i] = x[3 * i, 0]
        t[i] = np.sum(x[3 * i: 3 * (i + 1), 1]) * 1000
        
    # Sort input
    j = np.argsort(l);
    t = t[j]; l = l[j];

    # Estimate BLUE
    X = np.c_[np.log(l),np.ones(len(l))]
    y = np.log(t)
    C = np.dot(X.T,X)
    b = np.dot(np.dot(np.linalg.inv(C), X.T), y)
    s = np.dot(X,b)    

    # Determine nice sample (we can not plot all points)
    k = np.random.permutation(len(l))

    # Plot samle of full data
    x = l[k[1:sample]]; y = t[k[1:sample]]
    plt.hold('on')
    plt.loglog(x, y, '.', color=(0.6,0.6,0.6), label = '_nolegend_', \
               markeredgecolor=(0.6,0.6,0.6), markersize=10)
    
    # Plot BLUE
    plt.loglog(l, np.exp(s), 'k', linewidth=1.5)

    # Plot mean
    mx = np.mean(l); my = np.mean(t)
    plt.loglog(mx, my, 'o', markeredgewidth=1.5, color = 'w', markersize=10)
    print 'mx:%f my:%f\n' % (mx, my) 

    plt.grid('on')    
    plt.xlabel('Size of strings'); plt.ylabel('Run-time (ms)')
    plt.legend(('Estimate', 'Average'), 'upper left', numpoints = 1)
    leg = plt.gca().get_legend()
    ltext = leg.get_texts()
    plt.setp(ltext, fontsize='small')

    plt.tight_layout()
    plt.savefig('results/sally-%s.pdf' % dataset)
