import numpy as np 

data = np.genfromtxt("./hyper_param_test.txt", delimiter=' ',dtype=float,skip_header=1)

print(data.shape)

dim = data.shape[0]

max_idx = np.argmax(data[:,3])
max_idxs = np.argwhere(data[:,3]>0.50)
max_idxs=max_idxs[0:30]
e_mean = np.mean(data[max_idxs,0])
e_std = np.std(data[max_idxs,0])
a_mean = np.mean(data[max_idxs,1])
a_std = np.std(data[max_idxs,1])
g_mean = np.mean(data[max_idxs,2])
g_std = np.std(data[max_idxs,2])

print(data[max_idx,:],len(max_idxs))
print(e_mean, e_std, a_mean, a_std, g_mean, g_std)