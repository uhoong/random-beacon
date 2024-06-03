import argparse
import datetime

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='read log')
    parser.add_argument('--start', type=int)
    parser.add_argument('--end', type=int)
    args = parser.parse_args()

    times = []
    lists = list(range(args.start,args.end))
    for i in lists:
        with open("./log/log"+str(i),'r') as logfile:
            timestart = None
            timeend = None
            for line in logfile:
                if "start" in line:
                    timestart = datetime.datetime.strptime(line.split(" ")[1], "%H:%M:%S.%f")
                if "reconstruct" in line:
                    timeend = datetime.datetime.strptime(line.split(" ")[1], "%H:%M:%S.%f")
            if(timestart!=None and timeend!=None):
                timediff = timeend-timestart
                times.append(timediff.total_seconds())
                print(f'节点{i}用时：{timediff.total_seconds()}')
            else:
                print(f"节点{i}失败")
    print(f"最小用时：{min(times)}")