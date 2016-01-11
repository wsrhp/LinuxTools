import os,shutil
f_base='f:/Geant4/compton_tem14_102_1/buildtem14/';
#f_base='h:/';
file1=f_base+'run1.mac';
file2=f_base+'Bfield.dat';
file3=f_base+'TestEm14';
#file4=f_base+'rootelectron.c';
#file5=f_base+'rootpositron.c';
#for i in range(15,26):
i=500;
while i < 601:
    base='f:/data/';
    no=i;
    pathname=base+str(no)+"MeV"
    shutil.copy(file1, pathname);
    shutil.copy(file2, pathname);
    shutil.copy(file3, pathname);
    #shutil.copy(file4, pathname);
    #shutil.copy(file5, pathname);
    i=i+10;