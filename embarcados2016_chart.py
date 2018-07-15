import matplotlib.pyplot as plt
import matplotlib.animation as animation
import random
import time
import numpy as np
#-----------------------------------------------------------------------#
                    #OBSERVAÇOES
#Falta colocar os ranges corretos das leituras dos sensores
#Defini que no maximo o grafico tera 10 amostras, mas pode ser alterado no if(tempo>10)
#Depois de dez dados plotados o codigo salva a primeira amostra num txt e a exclui do grafico
#6 txts sao criados para cada "sensor" depois do intervalo limite definido na entrada
#Sao salvos tempo atual e tempo da leitura seguido do valor da leitura
#Ao pressionar o mouse o grafico da pause
#Na linha 167 esta definido o tempo de duracao de plotagem, ao passar o tempo a tela do grafico fechara

#-----------------------------------------------------------------------#

fig = plt.figure()
ax1 = fig.add_subplot(212)#define tamanho do grafico (nos parametros)
ax2 = fig.add_subplot(212)
ax3 = fig.add_subplot(221)
ax4 = fig.add_subplot(221)
ax5 = fig.add_subplot(222)
ax6 = fig.add_subplot(222)
#-------------------------------------------------------------------
#       Para garantir que os arquivos fiquem vazios no inicio       #
#-------------------------------------------------------------------

def restartFile (nome):
    arquivo = open (nome, "w")
    arquivo.write("")
    arquivo.close

def onClick(event):
    global pause
    pause ^= True
    
#------------------------------------------------------------------
#                     Valores iniciais                            #
#------------------------------------------------------------------



def writeFile(nome, dadosSensor,dadoAtual , tempo, xs, constTempo):
    dadosSensor.append(dadoAtual)

    if (tempo>constTempo):
        arquivo = open (nome, "a")
        arquivo.write ("tempo: "+str(int(tempo))+'\n'+str(xs[0])+'\n'+str(dadosSensor[0])+"\n\n")
        arquivo.close()
        if (tempo > constTempo):
            del dadosSensor[0]

    return dadosSensor
###--------------------------------------------------------------------------###
###--------------------------------------------------------------------------###

def inputs(i):
    global xs
    global acc
    global giro
    global strain
    global piezo
    global umi
    global pluv
    global constTempo
    global pause
    global tempo_inicial
    global tempoFim

    if not pause: #Se o mouse nao foi pressionado
    #-------------------------------------------------------------------
    #                     acelerometro                                 #
    #-------------------------------------------------------------------

        y = random.randrange (0,20,1)
        global tempo #para poder verificar tempo total de execuçao do programa
        
        tempo = int(time.time() - tempo_inicial) #alteracao do tempo
        xs.append(int(tempo)) #armazena o tempo na lista xs

        acc = writeFile("acelerometro.txt", acc, y, tempo, xs, constTempo)

    #-------------------------------------------------------------------
    #                     giroscopio                                   #
    #-------------------------------------------------------------------

        y = random.randrange (0,30,1)
        giro = writeFile("giroscopio.txt", giro, y, tempo, xs, constTempo)
        
    #--------------------------------------------------------------------
    #                     strain gauge                                  #
    #--------------------------------------------------------------------

        y = random.randrange (0,40,1)
        strain = writeFile("strain.txt", strain, y, tempo, xs, constTempo)
        
    #--------------------------------------------------------------------
    #                           piezo                                    #
    #--------------------------------------------------------------------

        y = random.randrange (0,50,1)
        piezo = writeFile("piezo.txt", piezo, y,tempo, xs, constTempo)

    #--------------------------------------------------------------------
    #                     umidade solo                                   #
    #--------------------------------------------------------------------

        y = random.randrange (0,60,1)
        umi = writeFile("umidadeSolo.txt", umi, y,tempo, xs, constTempo)
        
    #--------------------------------------------------------------------
    #                       pluviometro                                  #
    #--------------------------------------------------------------------

        y = random.randrange (0,70,1)
        pluv = writeFile("pluviometro.txt", pluv, y,tempo, xs, constTempo)

        if (tempo>constTempo): 
            del xs[0]
            
    #----------------------------------------------------------------------
    #                       plotagem dos graficos                         #
    #----------------------------------------------------------------------     
        ax1.clear()
        ax2.clear()
        ax3.clear()
        ax4.clear()
        ax5.clear()
        ax6.clear()
        ax1.plot (xs,acc, label='acc', color='r')
        ax2.plot (xs,giro, label ='giro',color='c')
        ax3.plot (xs,strain, label ='strain',color='g')
        ax4.plot (xs,piezo, label ='piezo', color='y')
        ax5.plot (xs,umi, label ='umidadeSolo', color='b') 
        ax6.plot (xs,pluv,label='pluviometro', color='k')
            
        plt.xlabel("red:acc cyan:gir green:str yel:pie  blue:umi black:plu" )

        if (tempo> tempoFim):
            plt.close()
        
###--------------------------------------------------------------------------###
###--------------------------------------------------------------------------###

#------------------------------------------------------------------
#                           MAIN                                   #
#------------------------------------------------------------------

xs =[] #Tempos
acc =[]
giro =[]
strain =[]
piezo =[]
umi =[]
pluv =[]
auxTry =0
pause =False

while (auxTry==0):
    
    try:
        
        constTempo = int(raw_input("defina quantos intervalos deseja ver na tela:\n"))
        tempo_inicial = time.time() #Adquire tempo da maquina
        auxTry=1
        tempoFim = constTempo*10 #TEMPO PARA ACABAR O PROGRAMA EM SEGUNDOS
        restartFile("acelerometro.txt")
        restartFile("giroscopio.txt")
        restartFile("strain.txt")
        restartFile("piezo.txt")
        restartFile("umidadeSolo.txt")
        restartFile("pluviometro.txt")
            
        
        try:
            fig.canvas.mpl_connect('button_press_event', onClick) #permite executar 
            animacao = animation.FuncAnimation (fig, inputs, repeat = True,interval=1000,) #funcao responsavel por atualizar os graficos a cada segundo
            plt.show()
        except:
            print("Problemas devido a plotagem do grafico\n")
        
    except ValueError:
        print "O numero deve ser um inteiro\n"
    
