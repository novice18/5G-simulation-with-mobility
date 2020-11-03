import sys
import os

logs = "../mmwave_logs/"
interval = 0.01

if "-p"in sys.argv:
	logs=str(sys.argv[sys.argv.index("-p")+1])
if "-i" in sys.argv:
    interval=float(sys.argv[sys.argv.index("-i")+1])

logfile=logs+"/exec_log.txt"
fulllog=logs+"/Complete/DL/"

ids=[]
counts = []

with open(logfile) as execlog:
   for line in execlog:
       if "CQI" in line:
            components = line.split(' ')
            if not components[0] in ids:
               ids.append(components[0])
               counts.append(1)
            else:
                counts[ids.index(components[0])] += 1

simTimeFound=False
found=[]
cqis=[]
simTimes=[]

with open(logfile) as execlog:
    for line in execlog:
        if simTimeFound:
            components = line.split(' ')
            if components[0] in ids and not components[0] in found:
                found.append(components[0])
                cqis[len(cqis)-1][ids.index(components[0])]=components[4]
            if len(found)==len(ids):
                simTimeFound=False
                found=[]
        if "SimTime" in line:
            if simTimeFound:
                cqis[len(cqis)-1] = ['NA\n']*len(ids)
                components = line.split(' ')
                simTimes.append(components[1])
                cqis.append([0]*len(ids))
            else:
                components = line.split(' ')
                simTimeFound=True
                simTimes.append(components[1])
                cqis.append([0]*len(ids))

print len(ids)
for i in range(0,len(ids)):
    logname = fulllog+"UE_"+str(i)+"_interval_"+str(interval)+".csv"
    tmpfilename = fulllog+"UE_"+str(i)+"_tmp.csv"
    tmpfile = open(tmpfilename,"a")
    time = 0
    with open(logname,'r') as rfile:
        for line in rfile:
            if "\n" in line:
                line = line.rstrip("\n")
            if "time" in line:
                tmpfile.write(line+",'CQI'\n")
            else:
                tmpfile.write(line+","+str(cqis[time][i]))
                time+=1
    tmpfile.close()
    os.system("rm '"+logname+"'")
    os.system("mv '"+tmpfilename+"' '"+logname+"'")
    

#for i in range(0,len(simTimes)):
#    print "Time = "+simTimes[i]
#    for j in range(0,len(ids)):
#        print ids[j] + " CQI = " + cqis[i][j]

#for i in range(0,len(ids)):
#    print "ID\t" + ids[i] + "occurs\t" + str(counts[i]) + "\ttimes."