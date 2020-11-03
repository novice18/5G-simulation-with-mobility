import os
import sys

logs = "../mmwave_logs/"

if "-p"in sys.argv:
	logs=str(sys.argv[sys.argv.index("-p")+1])

dl = logs+"/Complete/DL/"
ul = logs+"/Complete/UL/"

for filename in os.listdir(dl):
    tmpname = dl+filename+"_tmp.txt"
    tmpfile = open(tmpname,"w+")
    targfile = dl+filename

    with open(targfile,"r") as file:
        for row in file:
            tmpfile.write(row.replace('"','').replace("'",""))

    os.system("rm '"+targfile+"'")
    os.system("mv '"+tmpname+"' '"+targfile+"'")

for filename in os.listdir(ul):
    tmpname = ul+filename+"_tmp.txt"
    tmpfile = open(tmpname,"w+")
    targfile = ul+filename

    with open(targfile,"r") as file:
        for row in file:
            tmpfile.write(row.replace('"','').replace("'",""))

    os.system("rm '"+targfile+"'")
    os.system("mv '"+tmpname+"' '"+targfile+"'")