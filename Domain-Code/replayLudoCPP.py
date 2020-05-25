from tkinter import *
from time import sleep
from params import *
from math import sin,cos,pi
import numpy as np
import csv

class LudoApp():
    def __init__(self,gameSize=1200):
        
        self.gameSize  = gameSize
        self.cellSize = gameSize/15
        self.color = ['blue','yellow','green','red']
        self.root, self.c = self.initializeApp()
        self.drawPts = self.generateMap()
        self.replayData = self.loadReplayData()
        self.replayIdx = 0

        self.tempDrawnObj = None
        self.MoveNr = 1;
      

        self.nextStep('')
        self.prevStep('')
        self.c.bind('<Key-Right> ',self.nextStep)
        self.c.bind('<Key-Left> ',self.prevStep)
        print('test')
        self.root.mainloop()
    

    def initializeApp(self):
        root = Tk()
        root.title('Ludo Replay')
        root.geometry("{}x{}".format(self.gameSize,self.gameSize))
        f=Frame(root,width=self.gameSize,height=self.gameSize)
        c = Canvas(root,width=self.gameSize,height=self.gameSize,background='#DAA869')
        c.pack()
        c.focus_set()
        return root, c

    def nextStep(self, event):
        if self.replayIdx == len(self.replayData)-1: 
            print('At game end press left to go back')
        else:
            self.replayIdx+=1
            self.MoveNr +=1

        dataRow = list(map(int,self.replayData[self.replayIdx][0:19])) 
        
        playerI = dataRow[0]
        die = dataRow[1]
        pieceI = dataRow[2]
        playerPieces = [dataRow[3:7],dataRow[7:11],dataRow[11:15],dataRow[15:19]]

        # Delete old drawn pieces
        if self.tempDrawnObj is not None:
            for piece in self.tempDrawnObj:
                self.c.delete(piece)

        # Draw new peices
        self.tempDrawnObj=[self.drawPlayer(self.drawPts[i][0,piece],self.drawPts[i][1,piece],j,i) for i,pieces in enumerate(playerPieces) for j,piece in enumerate(pieces)]
        
        self.tempDrawnObj.append(self.c.create_text(200,30,fill="gray",font="Times 20 italic bold",
                        text="Player {}, Die {}, PieceChosen {}\nMoveNr {}".format(playerI+1,die,pieceI+1,self.MoveNr)))

    def prevStep(self, event):
        if self.replayIdx ==0: 
            print('At game start press right to go forward')
        else:
            self.replayIdx-=1
            self.MoveNr -=1;
        
        dataRow = list(map(int,self.replayData[self.replayIdx][0:19])) 
        
        playerI = dataRow[0]
        die = dataRow[1]
        pieceI = dataRow[2]
        playerPieces = [dataRow[3:7],dataRow[7:11],dataRow[11:15],dataRow[15:19]]

        # Delete old drawn pieces
        if self.tempDrawnObj is not None:
            for piece in self.tempDrawnObj:
                self.c.delete(piece)
        
        # Draw new peices
        self.tempDrawnObj=[self.drawPlayer(self.drawPts[i][0,piece],self.drawPts[i][1,piece],j,i) for i,pieces in enumerate(playerPieces) for j,piece in enumerate(pieces)]
        self.tempDrawnObj.append(self.c.create_text(200,30,fill="gray",font="Times 20 italic bold",
                        text="Player {}, Die {}, PieceChosen {}\nMoveNr {}".format(playerI+1,die,pieceI+1,self.MoveNr)))




    def drawRect(self,x,y,color='',scale=1):
        offSet = self.cellSize/2
        x1,y1,x2,y2 = x-offSet*scale,y-offSet*scale,x+offSet*scale,y+offSet*scale

        return self.c.create_rectangle(x1,y1,x2,y2,fill=color,outline='black')

    def drawCircle(self,x,y,color='',scale=1):
        offSet = self.cellSize/2
        x1,y1,x2,y2 = x-offSet*scale,y-offSet*scale,x+offSet*scale,y+offSet*scale

        return self.c.create_oval(x1,y1,x2,y2,fill=color,outline='black')

    def drawStar(self,x,y,scale=0.9):
        x = np.array((x,y))
        verts = np.array([ [10,40],[40,40],[50,10],[60,40],[90,40],[65,60],[75,90],[50,70],[25,90],[35,60]],dtype=float)
        verts-=50
        verts/=80/(self.cellSize*scale)
        verts+=x
        return self.c.create_polygon(list(verts.flatten()),fill='',outline='black')
        

    def drawGlobe(self,x,y,scale=0.9):
        offSet = scale*self.cellSize/2
        x1,y1,x2,y2 = x-offSet,y-offSet,x+offSet,y+offSet
        for i in range(6):
            s=i*self.cellSize/(5*2)   
            self.c.create_oval(x1+s,y1,x2-s,y2)
    
        self.c.create_oval(x1,y1+offSet*0.3,x2,y2-offSet*0.3)
        self.c.create_oval(x1,y1+offSet*0.6,x2,y2-offSet*0.6)
        self.c.create_line(x1,y,x2,y)


    def drawPlayer(self,x,y,pieceI,playerI,spread=0.50):
        offSet = self.cellSize/2
        x1,y1,x2,y2 = x-offSet*spread,y-offSet*spread,x+offSet*spread,y+offSet*spread

        if pieceI == 0:
                return self.drawCircle(x1,y1,color=self.color[playerI],scale=0.2)
        elif pieceI == 1:
                return self.drawCircle(x2,y1,color=self.color[playerI],scale=0.2)
        elif pieceI == 2:
                return self.drawCircle(x1,y2,color=self.color[playerI],scale=0.2)
        elif pieceI == 3:
                return self.drawCircle(x2,y2,color=self.color[playerI],scale=0.2)
        else:
            print("Cannot draw player with id {}".format(i))

    def generateMap(self):
        # Generate grid coordinates for player one
        drawPtsP1 = []
        drawPtsP1.append([3,12])
        x,y = 6,13
        for i in range(5): drawPtsP1.append([x,y-i])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x-1-i,last_y-1])
        last_x,last_y = drawPtsP1[-1]
        drawPtsP1.append([last_x,last_y-1])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x+i,last_y-1])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x+1,last_y-1-i])
        last_x,last_y = drawPtsP1[-1]
        drawPtsP1.append([last_x+1,last_y])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x+1,last_y+i])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x+1+i,last_y+1])
        last_x,last_y = drawPtsP1[-1]
        drawPtsP1.append([last_x,last_y+1])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x-i,last_y+1])
        last_x,last_y = drawPtsP1[-1]
        for i in range(6): drawPtsP1.append([last_x-1,last_y+1+i])
        last_x,last_y = drawPtsP1[-1]
        for i in range(7): drawPtsP1.append([last_x-1,last_y-i])

        drawPtsP1 = np.array(drawPtsP1,dtype=float).T
        drawPtsP1*=float(self.cellSize) # Scale to fit canvas
        drawPtsP1+=self.cellSize/2 # substract offset to get center of square

        # Rotation matrice
        rotate90 = np.array([[0, -1],[1, 0]],dtype=float)

        # Center Player 1 points
        drawPtsP1Centered=(drawPtsP1-self.gameSize/2.0)

        # Rotate to get centered player 2,3,4 pts
        drawPtsP2 = rotate90.dot(drawPtsP1Centered)
        drawPtsP3 = rotate90.dot(drawPtsP2)
        drawPtsP4 = rotate90.dot(drawPtsP3)

        # Undo centering
        drawPtsP2+=self.gameSize/2.0
        drawPtsP3+=self.gameSize/2.0
        drawPtsP4+=self.gameSize/2.0

        # Find all neutral pts on the map (pts with no player color)
        allPts, unique_indices, unique_counts = np.unique(np.concatenate((drawPtsP1,drawPtsP2,drawPtsP3,drawPtsP4),axis=1),axis=1,return_index=True,return_counts=True)
        neutralPts = allPts[:,np.argwhere(unique_counts>1).squeeze()]
        coloredPts = allPts[:,np.argwhere(unique_counts==1).squeeze()]

        # Generate star and globe points
        starPts = drawPtsP1[:,np.array(starI)]
        globePts = drawPtsP1[:,np.array(globeI)]

        # Draw coloured squares
        drawPtsP1Col = drawPtsP1[:,[0,1,52,53,54,55,56,57]]
        for x,y in drawPtsP1Col.T:
            self.drawRect(x,y,'blue')
        drawPtsP2Col = drawPtsP2[:,[0,1,52,53,54,55,56,57]]
        for x,y in drawPtsP2Col.T:
            self.drawRect(x,y,'yellow')
        drawPtsP3Col = drawPtsP3[:,[0,1,52,53,54,55,56,57]]
        for x,y in drawPtsP3Col.T:
            self.drawRect(x,y,'green')
        drawPtsP4Col = drawPtsP4[:,[0,1,52,53,54,55,56,57]]
        for x,y in drawPtsP4Col.T:
            self.drawRect(x,y,'red')

        # Draw neutral coloured squares
        for i in range(neutralPts.shape[1]):
            x,y= neutralPts[:,i]
            self.drawRect(x,y)
        # Draw Stars
        for i in range(starPts.shape[1]):
            x,y= starPts[:,i]
            self.drawStar(x,y)
        # Draw Globes
        for i in range(globePts.shape[1]):
            x,y= globePts[:,i]
            self.drawGlobe(x,y)

        return [drawPtsP1, drawPtsP2, drawPtsP3, drawPtsP4]


    def loadReplayData(self):
        data = []
        with open('./cmake-build-release/replayLudo.txt') as csv_file:
            csv_reader = csv.reader(csv_file, delimiter=' ')
            for row in csv_reader:
                if len(row)>18:
                    data.append(row[0:19])
        return data


if __name__ == "__main__":
    print('test')
    app = LudoApp()






