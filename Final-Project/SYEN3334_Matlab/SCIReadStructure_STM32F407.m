%Test function for the SCI communication
%%Read a data struct.
%%The addresses of variables are read from varmapddat file. The file should
%%be updated whenever there is any change of variables in the firmware.
%%It is necessary to introduce a version number for the firmware so that
%%the program can verify the correct map file to be used.
%%Program to process the map file is Mappro.m
%%Original file is SCIFunTest11.m
%%Rename on 7/14/2015
load varmapdat_stm32f407 AdcMeanValueSet
s1 = serial('COM4','BaudRate', 9600,'DataBits', 8);
% To connect the serial port object to the serial port:
%%prepare the address of the struct
X=uint32(AdcMeanValueSet); %Set the address of a data variable
addr1=typecast(X,'uint8');
addr = [addr1];
%%prepare the number of bytes in the struct
bytenum = 28;
bnum=uint8([bytenum]);
%%define the writing function
wrmode = uint8([0]); %read/write mode: "0" for reading and "1" for writing

S=uint8(['@' 'X' addr bnum wrmode]);

fopen(s1)
fwrite(s1,S); %in the test program with C8051 Lab10test.asm, we can only write one byte for starting the commuication!
%We should not use fprintf to send one character. It will send at lease two
%bytes.
%a = fscanf(s1)
%Op=fread(s1,1); %dummy read
bytenum = bytenum + 1;
[x, cnt]=fread(s1,bytenum);
% To disconnect the serial port object from the serial port.
fclose(s1); 
DX=uint8([x(2) x(3) x(4) x(5)]);
SecCount=typecast(DX,'int32')

DX=uint8([x(6) x(7) x(8) x(9)]);
PA3=typecast(DX,'single')

DX=uint8([x(10) x(11) x(12) x(13)]);
PA5=typecast(DX,'single')

DX=uint8([x(14) x(15) x(16) x(17)]);
PB0=typecast(DX,'single')

DX=uint8([x(18) x(19) x(20) x(21)]);
PB1=typecast(DX,'single')

DX=uint8([x(22) x(23) x(24) x(25)]);
PC2=typecast(DX,'single')

DX=uint8([x(26) x(27) x(28) x(29)]);
PC3=typecast(DX,'single')