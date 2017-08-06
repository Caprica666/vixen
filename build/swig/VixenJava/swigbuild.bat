echo on
cd %2
%1\swig.exe -c++ -I%3 -java -module %4 -package Vixen -outdir VixenJava/Sources VixenLite.swg