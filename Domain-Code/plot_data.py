import numpy as np
import matplotlib.pyplot as plt
from numpy import genfromtxt

train = genfromtxt('data/train.txt', delimiter=' ',dtype=float)
evaluation = genfromtxt('data/eval.txt', delimiter=' ',dtype=float)

print(train.shape)

fig, ax = plt.subplots(4,2) # type: tuple(plt.Figure ,List[plt.Axes])


def moving_average(data, periods=3,axis=0):
    weights = np.ones(periods,dtype=float) / periods

    new_shape = list(data.shape)
    new_shape[0]-=(periods-1)

    filtered_data = np.zeros(new_shape)

    print(data.shape,new_shape)
    if len(new_shape)<2:
        filtered_data=np.convolve(data.squeeze(), weights, mode='valid')
    else:
        for i in range(new_shape[1]):
            filtered_data[:,i]=np.convolve(data[:,i], weights, mode='valid')

    return filtered_data


wins = train[:,0:1]*100
accumulated_reward = train[:,4:5]
evaluation = evaluation[:,0]*100
qsize = train[:,8:9]
player_was_sent_home = train[:,12:16]
player_sent_someone_home = train[:,16:20]
stars_hit = train[:,20:24]
globes_hit = train[:,24:28]
nrTrainGames = 10



ax[0][0].plot(moving_average(wins,nrTrainGames))
ax[0][0].plot(moving_average(wins,nrTrainGames*100))
ax[0][0].set_title("Training Win Percentage")

ax[1][0].plot(moving_average(accumulated_reward,nrTrainGames))
ax[1][0].plot(moving_average(accumulated_reward,nrTrainGames*10))
ax[1][0].set_title("Training Accumulated reward per game")

ax[2][0].plot(moving_average(qsize,nrTrainGames))
ax[2][0].set_title("Training Q tabel size")
ax[2][0].legend(["Player  {}".format(i) for i in range(4)])

ax[3][0].plot([i*nrTrainGames for i in range(evaluation.shape[0])],evaluation)
evaluation = moving_average(evaluation,5)
ax[3][0].plot([i*nrTrainGames for i in range(evaluation.shape[0])],evaluation)
ax[3][0].set_title("Evaluation Win Percentage")
ax[3][0].grid()

names = ["Was sent home","Sent enemy home","Stars hit","Globes hit"]
cnt= [player_was_sent_home,player_sent_someone_home,stars_hit,globes_hit]
for i in range(4):
    ax[i][1].plot(moving_average(cnt[i],100))
    ax[i][1].set_title(names[i])

plt.show()
