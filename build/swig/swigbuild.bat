echo on
cd %2
%1\swig.exe -c++ -I%3 -csharp -Fmicrosoft -fvirtual -module %4 -namespace Vixen -outdir VixenCS/Sources Vixen.swg