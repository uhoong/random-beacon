import argparse
import datetime

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='read log')
    parser.add_argument('--number', type=int)
    parser.add_argument('--ips', type=str, default=None)
    parser.add_argument('--groupid', type=int, default=0)
    args = parser.parse_args()

    if args.ips is None:
        ips = ['127.0.0.1']
    else:
        ips = [l.strip() for l in open(args.ips, 'r').readlines()]

    groups = len(ips)
    lists = list(range(int(args.number/groups*args.groupid),int(args.number/groups*(args.groupid+1))))
    print(f"集群{args.groupid}")
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
                print(f'节点{i}用时：{timediff.total_seconds()}')
            else:
                print(f"节点{i}失败")