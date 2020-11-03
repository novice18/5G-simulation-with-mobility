import csv
import sys
import os
import datetime

scripts = os.path.dirname(os.path.realpath(__file__))
trace_loc = scripts + "/../ns3-mmwave-new-handover/"
path = scripts + "/../mmwave_logs/"
numUe = 1
numEnb = 1
interval = 0.01
time = 0.0

if "-ue" in sys.argv:
    numUe = int(sys.argv[sys.argv.index("-ue") + 1]) + 1
if "-enb" in sys.argv:
    numEnb = int(sys.argv[sys.argv.index("-enb") + 1]) + 1
if "-i" in sys.argv:
    interval = float(sys.argv[sys.argv.index("-i") + 1])
if "-p" in sys.argv:
    path = str(sys.argv[sys.argv.index("-p") + 1])


RxPacketTraceFile_trace = trace_loc + "/RxPacketTrace.csv"
PositionsFile_trace = trace_loc + "/mmwavePositions"
orig_dest = path + "/Originals/"
RxPacketTraceFile = orig_dest + "/RxPacketTrace.csv"

os.system("mkdir -p '" + path + "'")

if len(os.listdir(path)) > 0:
    print("Moving Old Logs...")
    dt = datetime.datetime.now()
    os.system("mkdir -p '" + path + "/../mmwave_old_logs/'")
    os.system("mv '" + path + "' '" + path + "/../mmwave_old_logs/" + str(dt.year) +
              str(dt.month) + str(dt.day) + str(dt.hour) + str(dt.minute) + str(dt.second) + "'")
    os.system("mkdir -p '" + path + "'")

os.system("mkdir -p '" + path + "/Originals'")
os.system("mkdir -p '" + path + "/Isolated'")
os.system("mkdir -p '" + path + "/Complete'")
os.system("mkdir -p '" + path + "/Complete/UL'")
os.system("mkdir -p '" + path + "/Complete/DL'")

print("Isolating UE & ENB Logs Based on UL/DL")
if os.path.isfile(RxPacketTraceFile_trace):
    os.system("mv '" + RxPacketTraceFile_trace + "' '" + orig_dest + "'")
for i in range(0, numUe):
    if os.path.isfile(PositionsFile_trace + str(i) + ".csv"):
        os.system("mv '" + PositionsFile_trace +
                  str(i) + ".csv' '" + orig_dest + "'")
if os.path.isfile(path + "/../exec_log.txt"):
    os.system("mv '" + path + "/../exec_log.txt' '" + path + "'")

if os.path.isfile(trace_loc + "/DlRlcStats.txt"):
    os.system("mv '" + trace_loc + "/DlRlcStats.txt' '" + path + "'")
if os.path.isfile(trace_loc + "/UlRlcStats.txt"):
    os.system("mv '" + trace_loc + "/UlRlcStats.txt' '" + path + "'")
if os.path.isfile(trace_loc + "/UlPdcpStats.txt"):
    os.system("mv '" + trace_loc + "/UlPdcpStats.txt' '" + path + "'")
if os.path.isfile(trace_loc + "/DlPdcpStats.txt"):
    os.system("mv '" + trace_loc + "/DlPdcpStats.txt' '" + path + "'")

# rowcount=0

if os.path.isfile(RxPacketTraceFile):
    with open(RxPacketTraceFile, 'rb') as trace:
        # data = list(csv.reader(trace,delimiter=','))
        # print len(data[1])
        # print data[1][0]

        tracereader = csv.reader((x.replace('\0', '')
                                  for x in trace), delimiter=',')

        headings = str(next(tracereader))[1:-1]
        for rnti in range(1, numUe):
            uefile = open(path + "/Isolated/UE_" +
                          str(rnti) + "_isolated_DL.csv", "w+")
            uefile.write(headings)
            uefile.close()
            uefile = open(path + "/Isolated/UE_" +
                          str(rnti) + "_isolated_UL.csv", "w+")
            uefile.write(headings)
            uefile.close()

        for cellid in range(1, numEnb):
            enbfile = open(path + "/Isolated/ENB_" +
                           str(cellid) + "_isolated_DL.csv", "w+")
            enbfile.write(headings)
            enbfile.close()
            enbfile = open(path + "/Isolated/ENB_" +
                           str(cellid) + "_isolated_UL.csv", "w+")
            enbfile.write(headings)
            enbfile.close()

        for row in tracereader:
            # rowcount+=1
            if row[0] == "DL":
                rnti = row[7]
                cellid = row[6]
                enbfile = open(path + "/Isolated/ENB_" +
                               cellid + "_isolated_DL.csv", "a+")
                enbfile.write("\n" + str(row)[1:-1])
                uefile = open(path + "/Isolated/UE_" +
                              rnti + "_isolated_DL.csv", "a+")
                uefile.write("\n" + str(row)[1:-1])
            elif row[0] == "UL":
                rnti = row[7]
                cellid = row[6]
                enbfile = open(path + "/Isolated/ENB_" +
                               cellid + "_isolated_UL.csv", "a+")
                enbfile.write("\n" + str(row)[1:-1])
                uefile = open(path + "/Isolated/UE_" +
                              rnti + "_isolated_UL.csv", "a+")
                uefile.write("\n" + str(row)[1:-1])
            uefile.close()
            enbfile.close()
else:
    print("ERROR: Log File " + RxPacketTraceFile +
          " has not been generated. Cannot Parse.")
    os.system("clear;cat ../mmwave_logs/exec_log.txt")

# print "Row :"+str(rowcount)

for i in range(0, numUe - 1):
    with open(path + "/Originals/mmwavePositions" + str(i) + ".csv") as ueposlog:
        with open(path + "/Isolated/UE_" + str(i + 1) + "_isolated_DL.csv", 'rb') as uerxlog:
            rxreader = csv.reader(uerxlog, delimiter=',')
            posreader = csv.reader(ueposlog, delimiter=',')
            headings = str(next(rxreader))[
                1:-1] + "," + str(next(posreader))[1:-1] + "\n"
            dlfile = open(path + "/Complete/DL/UE_" + str(i) +
                          "_interval_" + str(interval) + ".csv", 'w+')
            dlfile.write(headings)

            getNext = True
            endofrx = False

            for row in ueposlog:
                posTime = float(row.split(",")[0])
                if endofrx:
                    dlfile.write(
                        "DL,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA," + row)
                else:
                    if getNext:
                        rxRow = str(next(uerxlog, 'eof'))
                        if rxRow == 'eof':
                            endofrx = True
                            dlfile.write(
                                "DL,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA," + row)
                            break
                        rxTime = float(rxRow.split(",")[1].replace("'", ""))

                    while posTime > rxTime and abs(posTime - rxTime) > interval:
                        rxRow = str(next(uerxlog, 'eof'))
                        if rxRow == 'eof':
                            endofrx = True
                            dlfile.write(
                                "DL,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA," + row)
                            break
                        rxTime = float(rxRow.split(",")[1].replace("'", ""))

                    if posTime < rxTime and abs(posTime - rxTime) > interval:
                        dlfile.write(
                            "DL,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA," + row)
                        getNext = False

                    elif posTime > rxTime and abs(posTime - rxTime) < interval:
                        nextRx = str(next(uerxlog, 'eof'))
                        if nextRx == 'eof':
                            dlfile.write(rxRow.replace("\n", ",") + row)
                            getNext = False
                            endofrx = True
                        else:
                            nextRxTime = float(nextRx.split(",")[
                                               1].replace("'", ""))

                            if abs(posTime - rxTime) < abs(posTime - nextRxTime):
                                dlfile.write(rxRow.replace("\n", ",") + row)
                                rxTime = nextRxTime
                                getNext = False

                            else:
                                dlfile.write(nextRx.replace("\n", ",") + row)
                                getNext = True

    with open(path + "/Originals/mmwavePositions" + str(i) + ".csv") as ueposlog:
        time = 0.0
        last_time = 0.0
        last_row = []
        with open(path + "/Isolated/UE_" + str(i + 1) + "_isolated_UL.csv", 'rb') as uerxlog:
            rxreader = csv.reader(uerxlog, delimiter=',')
            posreader = csv.reader(ueposlog, delimiter=',')
            headings = str(next(rxreader))[
                1:-1] + "," + str(next(posreader))[1:-1]
            ulfile = open(path + "/Complete/UL/UE_" + str(i) +
                          "_interval_" + str(interval) + ".csv", 'w+')
            ulfile.write(headings)
            for row in rxreader:
                row_time = float(row[1].replace("'", ""))
                if last_row == []:
                    ulfile.write(
                        "\n" + str(row)[1:-1] + "," + str(next(posreader, None))[1:-1])
                    time = time + interval
                elif row_time == time:
                    ulfile.write(
                        "\n" + str(row)[1:-1] + "," + str(next(posreader, None))[1:-1])
                    time = time + interval
                elif row_time > time:
                    if(last_time < time):
                        if(row_time - time <= time - last_time):
                            ulfile.write(
                                "\n" + str(row)[1:-1] + "," + str(next(posreader, None))[1:-1])
                            time = time + interval
                        else:
                            ulfile.write(
                                "\n" + str(last_row)[1:-1] + "," + str(next(posreader, None))[1:-1])
                            time = time + interval
                last_time = row_time
                last_row = row
            ulfile.close()

print "Logs successfully isolated"

# for i in range(0,numUe):
# Following should serve as a guide, copied from my python scratchpad
##time = 0.0
##interval = 0.01
# last_line=[]
# row_time=0.0
# last_time=0.0
# this_diff=0
# last_diff=0
##
# with open("RxPacketTrace.csv",'rb') as file:
##	filereader = csv.reader((x.replace('\0','') for x in file),delimiter=',')
##	headings = str(next(filereader))[1:-1]
# print headings[1]
# for row in filereader:
# row_time=float(row[1])
# if last_line==[]:
# print row
# time=time+interval
# elif row_time==time:
# print row
# time=time+interval
# elif row_time>time:
# if last_time<time:
##				this_diff = row_time - time
##				last_diff = last_time - time
# if(this_diff<=last_diff):
# print row[1]+" "+last_line[1]
# else:
# print last_line[1]+" "+row[1]
# time=time+interval
# last_line=row
# last_time=row_time
