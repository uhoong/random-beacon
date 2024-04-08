# 文件传输命令

在本地主机 git bash 中运行

先从 fjl 服务器下载文件到本地
scp -r -P 22222 fjl@123.157.213.104:/home/fjl/xshproject/random-beacon/pvssconf /c/39local/random-beacon
scp -P 22222 fjl@123.157.213.104:/home/fjl/xshproject/random-beacon/client /c/39local/random-beacon
scp -P 22222 fjl@123.157.213.104:/home/fjl/xshproject/random-beacon/drg /c/39local/random-beacon
scp -P 22222 fjl@123.157.213.104:/home/fjl/xshproject/random-beacon/drg.conf /c/39local/random-beacon

再将本地文件传输到新服务器
scp -r -P 2223 /c/39local/random-beacon/pvssconf root@123.157.213.104:/root/random-beacon
scp -P 2223 /c/39local/random-beacon/client root@123.157.213.104:/root/random-beacon
scp -P 2223 /c/39local/random-beacon/drg root@123.157.213.104:/root/random-beacon
scp -P 2223 /c/39local/random-beacon/drg.conf root@123.157.213.104:/root/random-beacon