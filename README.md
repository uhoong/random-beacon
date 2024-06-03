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

# 环境配置
vim /etc/resolv.conf
    nameserver 8.8.8.8

cp /etc/apt/sources.list /etc/apt/sources.list.bak
vim /etc/apt/sources.list
    deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse
    deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse
    deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse
    deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse
    deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse
    deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse
    deb http://security.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse
    deb-src http://security.ubuntu.com/ubuntu/ jammy-security main restricted universe multiverse
    deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-proposed main restricted universe multiverse
    deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-proposed main restricted universe multiverse

apt update
apt install proxychains
    配置文件 /etc/proxychains.conf
    注释 dns
    端口改为 7891

sudo apt-get update && sudo apt-get upgrade
sudo apt install build-essential git libboost-all-dev cmake libgmp3-dev libssl-dev libprocps-dev pkg-config libsodium-dev

# libsodium
git clone https://github.com/algorand/libsodium.git && cd libsodium && sh ./autogen.sh && ./configure && make && sudo make install
git clone https://github.com/algorand/libsodium.git
cd libsodium
sh ./autogen.sh
./configure
make
sudo make install

# libff
git clone https://github.com/scipr-lab/libff.git && cd libff && git submodule init && git submodule update && mkdir build && cd build && cmake .. && make && sudo make install
git clone https://github.com/scipr-lab/libff.git
cd libff
apt install build-essential git libboost-all-dev cmake libgmp3-dev libssl-dev libprocps-dev pkg-config libsodium-dev
git submodule init && git submodule update
mkdir build && cd build
cmake ..
make
sudo make install

# libuv
git clone https://github.com/libuv/libuv.git && cd libuv && sh autogen.sh && ./configure && make && sudo make install
git clone https://github.com/libuv/libuv.git
cd libuv
sh autogen.sh
./configure
make
sudo make install

# salticidae
git clone https://github.com/Determinant/salticidae.git && cd salticidae && cmake . && make && sudo make install
git clone https://github.com/Determinant/salticidae.git
cd salticidae
cmake .
make
sudo make install

# GF-complete
git clone https://github.com/ceph/gf-complete.git && cd gf-complete && sh autogen.sh && ./configure && make && sudo make install
git clone https://github.com/ceph/gf-complete.git
cd gf-complete
sh autogen.sh
./configure
make
sudo make install

# Jerasure
git clone https://github.com/ceph/jerasure.git && cd jerasure && autoreconf --force --install && ./configure && make && sudo make install
git clone https://github.com/ceph/jerasure.git
cd jerasure
autoreconf --force --install
./configure
make
sudo make install

cd /usr/local/include 
sudo vim /usr/local/include/jerasure.h
"jerasure/gal"

sudo vim /etc/ld.so.conf
添加 /usr/local/lib
sudo ldconfig


git clone https://github.com/uhoong/random-beacon.git && cd random-beacon && git pull origin feat-scale  && mkdir pvssconf && mkdir log && mkdir build && cd build && cmake .. && make

# 节点1
python3 scripts/gen_conf.py --iter 8  --ips ips.txt --groupid i
./scripts/run_demo.sh i j
python3 scripts/read_log.py --number 32 --ips ips.txt --groupid 0 
./scripts/run_demo.sh 0 15
修改遍历元素为0到15
sh scripts/gen_conf.py

./client

# 节点1
python3 scripts/gen_conf.py --iter 16  --ips ips2.txt --clientip 3.88.30.223
修改遍历元素为16到31
sh scripts/gen_conf.py


54.89.13.231
3.107.9.72
54.64.237.159
13.53.111.77

ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDZPSIDUNSo2aX9N6z9EANc8iLIwqAdnyJLw6gL61IYTEQm0ybitjKX2508pSQ487mWrHWw6inofop7zafgQzH1kkQv8tmS+R3q6KLYCYiGTJPvF+O2/S52BJ8ksbMNieUv+cXRaX25mL6QYkWD2FLEQBT77RYb2kiVTvSAi09/+GDitnxmzIuspv7RHciMWrqJivWIED53i8GHzm25VRiVMX97C6qeIzY8q4XxYgDcxKYu5wReuHn9Xl+VpTNzy2Z3WnO46gLE/Vg69jx/tnmx1c5YERa9tunJdRQ3zE9f1H3sly2L8cx2/Dffy++huPtIAuHMfFiG/aqiH1LXUqrWUCrYfG7HczE97RrcWB/WV2RwsyS/OxDKsuOzyHCcZIAE4TIZyaCTJnJABFRnQgHgqW5UzkwE6Vf15sgxmq/mNXdL5Jx1b/VJFORXUqtvJFIvWbZBSA3wFt4JewFj7hBEvmiuBi+PqQ+smF5n4DqMn0kPwBDmK9yuVPjEUd/zbyU= uhong@uhongdeMacBook-Pro.local