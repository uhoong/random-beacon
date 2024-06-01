import os, re
import subprocess
import itertools
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate configuration file for a batch of replicas')
    parser.add_argument('--prefix', type=str, default='drg')
    parser.add_argument('--evil-notSharing', type=int, default=0)
    parser.add_argument('--evil-notForward', type=int, default=0)
    parser.add_argument('--clientip', type=str, default="127.0.0.1")
    parser.add_argument('--ips', type=str, default=None)
    parser.add_argument('--iter', type=int, default=5)
    parser.add_argument('--pport', type=int, default=20000)
    # parser.add_argument('--keygen', type=str, default='./hotstuff-keygen')
    parser.add_argument('--pvss-setup', type=str, default='./pvss-setup')
    # parser.add_argument('--tls-keygen', type=str, default='./hotstuff-tls-keygen')
    parser.add_argument('--nodes', type=str, default='nodes.txt')
    # parser.add_argument('--block-size', type=int, default=1)
    # parser.add_argument('--pace-maker', type=str, default='rr')
    args = parser.parse_args()


    if args.ips is None:
        ips = ['127.0.0.1']
    else:
        ips = [l.strip() for l in open(args.ips, 'r').readlines()]
    prefix = args.prefix
    iter = args.iter
    base_pport = args.pport
    # keygen_bin = args.keygen
    # tls_keygen_bin = args.tls_keygen
    pvss_setup_bin = args.pvss_setup

    # 恶意节点设置
    evil_notSharing = args.evil_notSharing
    evil_notForward = args.evil_notForward

    main_conf = open("{}.conf".format(prefix), 'w')
    # nodes = open(args.nodes, 'w')
    replicas = ["{}:{}".format(ip, base_pport + i)
                for ip in ips
                for i in range(iter)]
    # p = subprocess.Popen([keygen_bin, '--num', str(len(replicas))],
    #                     stdout=subprocess.PIPE, stderr=open(os.devnull, 'w'))
    # keys = [[t[4:] for t in l.decode('ascii').split()] for l in p.stdout]
    # tls_p = subprocess.Popen([tls_keygen_bin, '--num', str(len(replicas))],
    #                     stdout=subprocess.PIPE, stderr=open(os.devnull, 'w'))
    # tls_keys = [[t[4:] for t in l.decode('ascii').split()] for l in tls_p.stdout]

    subprocess.Popen([pvss_setup_bin, "--num", str(len(replicas))], stdout =subprocess.PIPE, stderr=open(os.devnull, 'w'))

    # if not (args.block_size is None):
    #     main_conf.write("block-size = {}\n".format(args.block_size))
    # if not (args.pace_maker is None):
    #     main_conf.write("pace-maker = {}\n".format(args.pace_maker))
    # main_conf.write("stat-period = -1\n")
    main_conf.write("nworker = 2\n")
    for r in zip(replicas, itertools.count(0)):
        main_conf.write("replica = {}\n".format(r[0]))
        # r_conf_name = "{}-sec{}.conf".format(prefix, r[1])
        # nodes.write("{}:{}\t{}\n".format(r[1], r[0], r_conf_name))
        # r_conf = open(r_conf_name, 'w')
        # r_conf.write("idx = {}\n".format(r[1]))
        # r_conf.write("pvss-ctx = pvss-sec{}.conf\n".format(r[1]))
        # r_conf.write("pvss-dat = pvss-setup.dat\n".format(r[1]))
    for i in range(iter-evil_notSharing,iter):
        main_conf.write("evil-notSharing = {}\n".format(i))
    for i in range(iter-evil_notForward,iter):
        main_conf.write("evil-notForward = {}\n".format(i))
    main_conf.write("client = {}:30000\n".format(args.clientip))
