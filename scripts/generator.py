import os
import subprocess
from time import sleep

myfile = open("scrape.csv","w")
myfile.write("UE,ENB,Realtime\n")
myfile.close()
for ue in range(8,11):
    for enb in range(1,11):
        if not (ue == 2 and enb==1):
            if enb>5 or ue>7:
                myfile=open("scrape.csv","a")
                os.system("clear;python start_mmwave.py -ue %s -enb %s"%(ue,enb))
                sleep(2)
                os.system("tail -n 1 ../mmwave_logs/exec_log.txt")
                tail = subprocess.check_output(["tail -n 1 ../mmwave_logs/exec_log.txt"], shell=True)
                time = tail.split(" ")[2]
                myfile.write(str(ue)+","+str(enb)+","+str(time))
                myfile.close()