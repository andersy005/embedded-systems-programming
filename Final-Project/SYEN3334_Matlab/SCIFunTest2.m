%Test function for the SCI communication
%Test function to write an 16-bit integer
load varmapdat_stm32f407 SecNum iSysCtrlWrd
s1 = serial('COM4','BaudRate', 9600,'DataBits', 8);
 
% To connect the serial port object to the serial port:
fopen(s1)
X=uint32(SecNum); %VLEDOffset
%X1=swapbytes(X);
addr=typecast(X,'uint8');
X2=uint16(0); %Disable battery operration
%X2=uint16(2); %Enable battery operation
data=typecast(X2,'uint8');
S=uint8(['@' 'B' addr data]);
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
