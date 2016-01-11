import os,shutil
os.mkdir(r'f:/data/');

#for i in range(25,26):
i=10;
while i < 61:
    base='f:/data/';
    no=i;
    pathname=base+str(no)+"MeV"
    os.makedirs(pathname);
    i=i+10