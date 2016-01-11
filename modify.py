import os,re

#for i in range(15,26):
i=500;
while i < 601:
    base='f:/data/';
    no=i;
    pathname=base+str(no)+"MeV";
    filename=base+str(no)+"MeV/"+"run1.mac";
    #n_filename=base+str(no)+"MeV/"+"run.mac";
    new_content="/gps/energy "+str(no)+" MeV";
    f=open(filename,'r+');
    data=f.read();
    n_data=re.sub(r'/gps/energy 350 MeV',new_content, data)
    f.close;
    f=open(filename,'w+');
    f.write(n_data);
    #open(n_filename, 'w').write(re.sub(r'/gps/energy 10 MeV',new_content, f.read()));
    f.close();
    i=i+10;