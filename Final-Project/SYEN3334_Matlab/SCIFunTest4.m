%Test function for the SCI communication
%%read a float point data from an addressed data memory
s1 = serial('COM4','BaudRate', 9600,'DataBits', 8);
% To connect the serial port object to the serial port:
load varmapdat_stm32f407 fTestData1 fTestData2
fopen(s1)
X=uint32(fTestData1);
%X=uint32(hex2dec('0000710b')); %read a data memory location
%X1=swapbytes(X);
addr=typecast(X,'uint8');
S=uint8(['@' 'C' addr]);
fwrite(s1,S); %in the test program with C8051 Lab10test.asm, we can only write one byte for starting the commuication!
%We should not use fprintf to send one character. It will send at lease two
%bytes.
%a = fscanf(s1)
%Op=fread(s1,1); %dummy read
[x, cnt]=fread(s1,5);
% To disconnect the serial port object from the serial port.
fclose(s1); 
DX=uint8([x(2) x(3) x(4) x(5)]);
y=typecast(DX,'single')
