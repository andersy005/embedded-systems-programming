%Test function for the SCI communication
%%Set for data set reading
%%The addresses of variables are read from varmapddat file. The file should
%%be updated whenever there is any change of variables in the firmware.
%%It is necessary to introduce a version number for the firmware so that
%%the program can verify the correct map file to be used.
%%Program to process the map file is Mappro.m
load varmapdat
s1 = serial('COM9','BaudRate', 9600,'DataBits', 8);
% To connect the serial port object to the serial port:
fopen(s1)
dnum = uint8(7); %Set the number of data in the set
X=uint32(ILEDOffset); %Set the address of a data variable
X1=swapbytes(X);
addr1=typecast(X1,'uint8');
X=uint32(VLEDOffset); %Set the address of a data variable
X1=swapbytes(X);
addr2=typecast(X1,'uint8');
X=uint32(CurSetMax); %Set the address of a data variable
X1=swapbytes(X);
addr3=typecast(X1,'uint8');
X=uint32(VLEDFb); %Set the address of a data variable
X1=swapbytes(X);
addr4=typecast(X1,'uint8');
X=uint32(Pwrfb); %Set the address of a data variable
X1=swapbytes(X);
addr5=typecast(X1,'uint8');
X=uint32(PwrPAcc); %Set the address of a data variable
X1=swapbytes(X);
addr6=typecast(X1,'uint8');
X=uint32(PwrNAcc); %Set the address of a data variable
X1=swapbytes(X);
addr7=typecast(X1,'uint8');

addr = [addr1 addr2 addr3 addr4 addr5 addr6 addr7];
wnum=uint8([1 1 1 2 2 2 2]);
delay=int8(-1); %negative delay for the sampling of a data set
S=uint8(['@' 'S' delay dnum addr wnum]);
fwrite(s1,S); %in the test program with C8051 Lab10test.asm, we can only write one byte for starting the commuication!
%We should not use fprintf to send one character. It will send at lease two
%bytes.
%a = fscanf(s1)
%Op=fread(s1,1); %dummy read
[x, cnt]=fread(s1,1)
% To disconnect the serial port object from the serial port.
fclose(s1); 
%DX=uint8([x(5) x(4) x(3) x(2)]);
%y=typecast(DX,'single')

