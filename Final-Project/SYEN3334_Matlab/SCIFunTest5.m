%Test function for the SCI communication
%Test function to write a float data to an addressed data memory
load varmapdat_stm32f407 fTestData1 fTestData2
s1 = serial('COM4','BaudRate', 9600,'DataBits', 8);
% To connect the serial port object to the serial port:
fopen(s1)
X=uint32(fTestData2);
%X1=swapbytes(X);
addr=typecast(X,'uint8');
X2=single(-2*3.14156);
data=typecast(X2,'uint8');
S=uint8(['@' 'D' addr data]);
fwrite(s1,S); %in the test program with C8051 Lab10test.asm, we can only write one byte for starting the commuication!
%We should not use fprintf to send one character. It will send at lease two
%bytes.
%a = fscanf(s1)
%Op=fread(s1,1); %dummy read
[x, cnt]=fread(s1,1);
% To disconnect the serial port object from the serial port.
fclose(s1); 
DX=uint8(x(1));
%y=typecast(DX,'int16')
