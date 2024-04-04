n = 50
m = [[0]*n for _ in range(n)]
share = [0]*n
with open("./log0","r") as file:
    for line in file:
        if "receive sharechunk" in line:
            l = line.strip().split(" ")
            r = int(l[-3])
            c = int(l[-1])
            m[r][c]=1
        if "receive share from" in line:
            l = line.strip().split(" ")
            r = int(l[-1])
            share[r] = 1
        if "beacon" in line:
            break
for i in range(len(m)):
    print(i,sum(m[i]))
print("share",sum(share))