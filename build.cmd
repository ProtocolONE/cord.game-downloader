echo off
@rem call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86_amd64

msbuild.exe build.proj /t:BuildStaticRelease_x86,BuildStaticDebug_x86
pause