import numpy as np
import matplotlib.pyplot as plt
from numpy import genfromtxt
import csv


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

def load_train_data(train_path):
    f = open(train_path)
    reader = csv.reader(f)
    headers = next(reader, None)
    f.close()
    nr_q_players = int(headers[0])
    data_fields_pr_player = int(headers[1])
    trainIterations = int(headers[2])

    train = genfromtxt(train_path, delimiter=',',dtype=float,skip_header=1)
    trainDataBlocks = [train[:,i:(i+nr_q_players)] for i in range(0,nr_q_players*data_fields_pr_player,nr_q_players)]#

    print("Loaded training data for {} Q-players with {} data fields and therefor shape {}".format(nr_q_players,data_fields_pr_player,train.shape))

    return trainDataBlocks,nr_q_players, data_fields_pr_player, trainIterations

def load_eval_data(eval_path):
    evaluation = genfromtxt(eval_path, delimiter=',',dtype=float)
    print("Loaded evaluation data for with shape {}".format(evaluation.shape))
    return evaluation

def load_train_and_eval_data(train_path='data/train.txt',eval_path='data/eval.txt'):
    trainDataBlocks,nr_q_players, data_fields_pr_player, trainIterations = load_train_data(train_path)
    evaluation = load_eval_data(eval_path)
    return trainDataBlocks,nr_q_players, data_fields_pr_player, trainIterations, evaluation






plt.rcParams.update({'font.size': 12})
def decorate_plot(title=None,legend=None,xlabel=None,ylabel=None):
    if title is not None:
        plt.title(title)
    if legend is not None:
        plt.legend(legend)
    if xlabel is not None:
        plt.xlabel(xlabel)
    if ylabel is not None:
        plt.ylabel(ylabel)

    plt.rcParams.update({'font.size': 12})
    plt.tight_layout()

def save_plot(save_path=None,format='eps',title=None,legend=None,xlabel=None,ylabel=None):
    decorate_plot(title,legend,xlabel,ylabel)

    if save_path is not None:
        plt.savefig(save_path+"."+format, format=format, pad_inches=0.0,)

def plot_evaluation(evaluation, nr_q_players, data_fields_pr_player, trainIterations, save_path=None,format='eps'):

    fig, ax = plt.subplots(1,figsize = (6,4)) # type: tuple(plt.Figure ,List[plt.Axes])
    rows,cols = evaluation.shape

    x = [i*trainIterations for i in range(rows)];

    if evaluation.shape[1] < 3:
        legends = ["P1"]
        ax.plot(x,evaluation[:,0])

    else:
        legends = ["P"+str(i+1) for i in range(cols)]
        legends[-1]="Total"
        for i in range(cols):
            ax.plot(x,evaluation[:,i])


    ax.set_title("Evaluation Win Percentage")
    ax.grid()

    
    save_plot(save_path,format,title="Evaluation Win Percentage",legend=legends,xlabel='training iterations',ylabel='win %')
    
    
    plt.show()

def plot_train_individual(blockI,title,ylabel,trainDataBlocks, nr_q_players, data_fields_pr_player, trainIterations, save_path=None,format='eps'):

    fig, ax = plt.subplots(1,figsize = (6,4)) # type: tuple(plt.Figure ,List[plt.Axes])
    data =trainDataBlocks[blockI]
    rows,cols = data.shape

    legends = ["P"+str(i+1) for i in range(cols)]
    ax.plot(data)

    ax.set_title("Evaluation Win Percentage")
    ax.grid()
    
    save_plot(save_path,format,title=title,legend=legends,xlabel='training iterations',ylabel=None)
   
    plt.show()

def plot_all_data_combined(trainDataBlocks,evaluation):
    wins = trainDataBlocks[0]
    accumulated_reward = trainDataBlocks[1]
    qsize = trainDataBlocks[2]
    player_was_sent_home = trainDataBlocks[3]
    player_sent_someone_home = trainDataBlocks[4]
    stars_hit = trainDataBlocks[5]
    globes_hit = trainDataBlocks[6]
    nrTrainGames = 10

    fig, ax = plt.subplots(4,2,figsize = (12,8)) # type: tuple(plt.Figure ,List[plt.Axes])

    ax[0][0].plot(moving_average(wins,nrTrainGames*10)*100)
    ax[0][0].set_title("Training Win Percentage")

    #ax[1][0].plot(moving_average(accumulated_reward,nrTrainGames))
    ax[1][0].plot(moving_average(accumulated_reward,nrTrainGames*10))
    ax[1][0].set_title("Training Accumulated Reward")

    ax[2][0].plot(moving_average(qsize,nrTrainGames))
    ax[2][0].set_title("Training Q-tabel Size")
#    ax[2][0].legend(["Player  {}".format(i) for i in range(4)])

    ax[3][0].plot([i*30 for i in range(evaluation.shape[0])],evaluation*100)#[i*trainIterations for i in range(evaluation.shape[0])]
    # evaluation = moving_average(evaluation,5)
    # ax[3][0].plot(evaluation)
    

    ax[3][0].set_title("Evaluation Win Percentage")
    ax[3][0].grid()

    names = ["Was Sent Home","Sent Enemy Home","Stars Hit","Globes Hit"]
    cnt= [player_was_sent_home,player_sent_someone_home,stars_hit,globes_hit]
    for i in range(4):
        ax[i][1].plot(moving_average(cnt[i],100))
        ax[i][1].set_title(names[i])

    plt.tight_layout(pad=0.4, w_pad=0.5, h_pad=1.0)
    plt.show() 

if __name__ == "__main__":
    trainDataBlocks, nr_q_players, data_fields_pr_player, trainIterations,evaluation  = load_train_and_eval_data();
    plot_all_data_combined(trainDataBlocks,evaluation);
    plot_evaluation(evaluation, nr_q_players, data_fields_pr_player, trainIterations ,None)
    #plot_train_individual(2,"States Explored",None,trainDataBlocks,nr_q_players, data_fields_pr_player, trainIterations ,'./test')
 
 

 ## COMPOSITION
    # compNameExt = ['r','a','ar','q']
    # names = ['Random','Aggressive','Random and Aggressive','Self-play']
    # for i,ext in enumerate(compNameExt):
    #     eval_r_path ='./data_composition_test/eval_r_'+ext+'.txt'
    #     eval_a_path ='./data_composition_test/eval_a_'+ext+'.txt'
    #     print(eval_r_path,eval_a_path)
    #     eval_r = load_eval_data(eval_r_path)
    #     eval_a = load_eval_data(eval_a_path)
    #     print(eval_r[1],eval_a[1])
    #     train,nr_q_players, data_fields_pr_player, trainIterations = load_train_data(train_path='./data_composition_test/train_'+ext+'.txt')

    #     fig, ax = plt.subplots(1,figsize = (6,4)) # type: tuple(plt.Figure ,List[plt.Axes])

    #     x = [i*trainIterations for i in range(eval_r.shape[0])];

    #     ax.plot(x,eval_r[:,0],x,eval_a[:,0])
    #     title = names[i]+" Composition Evaluation"
    #     legends = ['Eval R','Eval A']
       
    #     ax.grid()
    #     save_plot('./data_composition_test/eval_'+ext,'eps',title=title,legend=legends,xlabel='training iterations',ylabel='win %')
        
    #     plt.show()
