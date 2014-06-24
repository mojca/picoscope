This SDK contains examples for the PicoScope 4000 series oscilloscopes. These examples all require the driver ps4000.dll

1. C
2. Excel
3. Labview
4. C#

1. ps4000con.c is a console-mode Windows application. It requires the following files to compile:
-ps4000Api.h
-ps4000.lib (or ps4000bc.lib for Borland C)
-picoStatus.h

2. ps4000.xls is the Microsoft Excel example (VBA). It requires ps4000wrap.dll.

3. There are two LabVIEW examples, PS4000_block.vi and PS4000_stream.vi. They demonstrate block mode and streaming mode acquisitions respectively. These LabVIEW examples also require ps4000wrap.dll.

4. The C# zip file in the SDK includes a Visual Studio project file, ps4000example.csproj and all other files required to compile this console-mode .net example.

The SDK also includes the source code for ps4000wrap.dll, ps4000wrap.c. The wrapper is intended for use in environments that do not support function pointers.
