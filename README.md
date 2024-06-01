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

# libsodium
git clone https://github.com/algorand/libsodium.git
cd libsodium
sh ./autogen.sh
./configure
make
sudo make install

# libff
git clone https://github.com/scipr-lab/libff.git
cd libff
apt install build-essential git libboost-all-dev cmake libgmp3-dev libssl-dev libprocps-dev pkg-config libsodium-dev
git submodule init && git submodule update
mkdir build && cd build
cmake ..
make
sudo make install

# libuv
git clone https://github.com/libuv/libuv.git
cd libuv
sh autogen.sh
./configure
make
sudo make install

# salticidae
git clone https://github.com/Determinant/salticidae.git
cd salticidae
cmake .
make
sudo make install

# GF-complete
git clone https://github.com/ceph/gf-complete.git
cd gf-complete
sh autogen.sh
./configure
make
sudo make install

# Jerasure
git clone https://github.com/ceph/jerasure.git
autoreconf --force --install
./configure
make
sudo make install

cd /usr/local/include
sudo vim jerasure.h
"jerasure/gal"


git clone https://github.com/uhoong/random-beacon.git
cd random-beacon
git pull origin feat-scale

mkdir pvssconf
mkdir log

# 节点1
python3 scripts/gen_conf.py --iter 16  --ips ips1.txt
修改遍历元素为0到15
sh scripts/gen_conf.py

./client

# 节点1
python3 scripts/gen_conf.py --iter 16  --ips ips1.txt --clientip 34.227.113.9
修改遍历元素为16到31
sh scripts/gen_conf.py