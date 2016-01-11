#! /usr/bin/env python
# coding:utf-8

import os,datetime

base_dir = 'f:/'
list = os.listdir(base_dir)

filelist = []
for i in range(0, len(list)):
    path = os.path.join(base_dir,list[i])
    if os.path.isfile(path):
        filelist.append(list[i])

for i in range(0, len(filelist)):
    path = os.path.join(base_dir, filelist[i])
    if os.path.isdir(path):
        continue
    timestamp = os.path.getmtime(path)
    print timestamp
    ts1 = os.stat(path).st_mtime
    print ts1
    
    date = datetime.datetime.fromtimestamp(timestamp)
    print list[i],' 最近修改时间是: ',date.strftime('%Y-%m-%d %H:%M:%S')