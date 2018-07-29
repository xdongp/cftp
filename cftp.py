#!/usr/bin/env python
# -*- coding: utf-8 -*-
###############################################
#Author: Panxiaodong
#Email:panxiaodong@baidu.com
#Date:2010-03-01
###############################################

import os
import sys
import getopt
import re
import commands
import time
import random

#参数选项
g_shortopts ="h:f:l:o:u:p:hr"
g_longopts = [  "host=",
                "file=",
                "limit=",
                "recursion",
                "output=",
                "user=",
                "password=",
                "vesion",
                "help"]

g_opts = {"host"     :   "",
          "file"    :   "",
          "limit"    :   False,
          "user"    :   "",
          "password"    :   "",
          "output"    :   "",}

#设置显示颜色          
g_color = { "black"     :"\033[30;49;1m",
            "red"       :"\033[31;49;1m",
            "green"     :"\033[32;49;1m",
            "brown"     :"\033[33;49;1m",
            "blue"      :"\033[34;49;1m",
            "purple"    :"\033[35;49;1m",
            "cyan"      :"\033[36;49;1m",
            "white"     :"\033[37;49;1m"}

#设置背景颜色            
g_bg_color = "\033[39;49;0m"
g_sshpass = "./sshpass" 
g_cftp_serv = "/tmp/cftp-serv" 

LINE_LENGHT = 85
START_PORT = 20000
MAX_TRY_TIME = 1
END_PORT = 50000

def draw_line(fd = sys.stdout, lenght = LINE_LENGHT):
    fd.write('-' * lenght)
    fd.write('\n')

def myprint(stri):
    print "[CFTP.PY] %s" % stri

def draw_str(wstr, color = "white", bgcolor = g_bg_color, fd =sys.stdout, lenght = LINE_LENGHT):
    fd.write("#")
    lenght = lenght -1
    fd.write(" "*4)
    lenght = lenght - 4
    fcolor = g_color[color]
    fd.write("%s %s %s"%(fcolor, wstr, bgcolor) )
    lenght = lenght - len(wstr)
    fd.write(" "* (lenght-1))
    fd.write("#")
    fd.write('\n')


def usage():
    draw_line()
    draw_str("cftp -h host -f file [-l limit(MB)] -u user -p password [-o local_file]")
    draw_str("-h, --host=romate host")
    draw_str("-f, --file=romate file")
    draw_str("-u, --user=romate host ssh user")
    draw_str("-l, --limit=limit rate")
    draw_str("-p, --create=romate host ssh password") 
    draw_str("-o, --output=local output file") 
    draw_str("-v, --version")
    draw_str("-s, --help")
    draw_str("eg: cftp jx-ecom-log09.jx -f /home/work/cftp/data -l 30 ")
    draw_line()
    sys.exit(1)
 
 
def version():
    draw_line()
    draw_str("cftp -h host -f file [-l limit(MB)] -u user -p password [-o local_file]")
    draw_str("version 1.0.0.0")
    draw_line()
    sys.exit(1)
 

#检查输入的参数是否正确        
def check_args(argvs):
    try:
        opts,args = getopt.gnu_getopt(argvs, g_shortopts, g_longopts)
        #print opts
    except getopt.GetoptError:
        #print opts
        print "check args false"
        usage()
        
    draw_line()
    draw_str("CFTP is a Tool that is Dedicated Transmission Speed")
    host_opt = 0
    for opt in opts:
        if opt[0] == "-h" or opt[0] == "--host":
            host = opt[1]
            if not re.search('\d+\.\d+\.\d+\.\d+', host):
                host = get_ip(host)
                if  host == False:
                    draw_str("romate host error: %s"%host)
            draw_str("romate host: %s"%host)
            g_opts["host"] = host    
            draw_str("romate host: %s"%opt[1])
        elif opt[0] == "-f" or opt[0] == "--file":
            g_opts["file"] = opt[1]
            draw_str("Host file: %s"%opt[1])
        elif opt[0] == "-l" or opt[0] == "--limit":
            g_opts["limit"] = opt[1]
            draw_str("limit: %s"%opt[1])
        elif opt[0]== "-h" or opt[0] == "--help":
            usage()
        elif opt[0]== "-v" or opt[0] == "--version":
            version()
        elif opt[0] == "-u" or opt[0] == "--user":
            g_opts["user"] = opt[1]
            draw_str("user: %s"%opt[1])
        elif opt[0] == "-p" or opt[0] == "--password":
            g_opts["password"] = opt[1]
           # draw_str("password: %s"%opt[1])
        elif opt[0] == "-o" or opt[0] == "--output":
            g_opts["output"] = opt[1]
            draw_str("output: %s"%opt[1])
            
    if not g_opts['host'] or not g_opts['file'] or not g_opts['user'] or not g_opts['password']:
        usage()
     

def get_ip(hostName):
    xpin = commands.getoutput("ping -c1 %s 2>/dev/null" %hostName)
    ms='\d+\.\d+\.\d+\.\d+'
    ip = re.search(ms,xpin)
    if not ip:
        ret = False
    else:
        ret = ip.group()
    return ret

 
def start_server():
    tied_up_ports = []
    #首先判断cftp-serv是否存在
    host = g_opts['host']
    user = g_opts['user']
    passwd = g_opts['password']
    limit = g_opts['limit']
    filename = g_opts['file'] 
    outfile = g_opts['output']
    if not outfile:
        outfile = os.path.basename(filename)

    cmd = "%s -p %s ssh %s@%s 'ls %s >/dev/null'" % (g_sshpass, passwd, user, host, g_cftp_serv)
    ret = os.system(cmd)
    if ret != 0:
        myprint("cftp not exist in %s, send it!" % host)
        cmd = "%s -p %s scp ./cftp-serv %s@%s:%s" % (g_sshpass, passwd, user, host, g_cftp_serv)
        if os.system(cmd) != 0:
            myprint("send cftp-serv to %s failed, eixt!" % host)
            sys.exit(1)
        else:
            cmd = "%s -p %s ssh %s@%s 'chmod u+x %s'" % (g_sshpass, passwd, user, host, g_cftp_serv)
    
    #判断文件是否存在
    cmd = "%s -p %s ssh %s@%s 'ls %s >/dev/null'" % (g_sshpass, passwd, user, host, filename)
    ret = os.system(cmd)
    if ret != 0:
        myprint("file: %s not exist in %s!" % (filename, host))
        sys.exit(2)
    
    try_time = 3
    while True:
        if try_time == MAX_TRY_TIME:
            myprint("Have no port to use,exit")
            sys.exit(3)

        now_port =int( random.Random().random() * 30000 + START_PORT )
        draw_str( "Try Port: %s!" % now_port )
     
        if limit:
            cmd_serv = """%s -p %s ssh %s@%s 'killall -9 cftp-serv &>/dev/null; /tmp/cftp-serv -f %s -p %s -l %s &> /tmp/cftp.log &'\
                """ % (g_sshpass, passwd, user, host, filename, now_port, limit)

        else:
            cmd_serv = """%s -p %s ssh %s@%s 'killall -9 cftp-serv &>/dev/null; /tmp/cftp-serv -f %s -p %s &> /tmp/cftp.log &'\
                  """ % (g_sshpass, passwd, user, host,filename, now_port)
    
        cmd_recv = "./cftp-recv -i %s -p %d  -f %s" % (host, now_port, outfile)
        cmd = "%s && %s" %(cmd_serv, cmd_recv)
        draw_str(cmd)
        ret = os.system(cmd)
        if ret == 0:
            return True
        else:
            myprint("Try time: %d" % try_time)
            try_time += 1
    

if __name__ == "__main__":
    check_args(sys.argv)
    start_server()
