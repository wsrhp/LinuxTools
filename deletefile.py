import os
#file1='run1.mac';
#file2='Bfield.dat';
#file3='TestEm14';
file4='rootelectron.c';
file5='rootpositron.c';
for i in range(1,51):
    base='f:/Geant4/MISSION/xraydetail/';
    no=0.2*i;
    pathname4=base+str(no)+"MeV/"+file4;
    pathname5=base+str(no)+"MeV/"+file5;
    os.remove(pathname4);
    os.remove(pathname5);