%Read map file and generate a table including all symbols and their
%addresses in both hexadecimal and decimal formats.
%Read all lines of a map file into a cell
%The variables including their symbols and addresses are saved in
%varmapdat.dat
%The text file is vartable.txt
clear
fid=fopen('SYEN3334.map');
text = textscan(fid,'%s', 'delimiter', '\n');
fclose(fid);
%Find the begin and end of the section including all variable symbols and
%their addresses.
sbegin ='Entry                      Address    Size  Type      Object';
Ibegin = 0;
linenum = length(text{1});
charnum0 = length(sbegin);
for i = 1:linenum
    s=text{1}{i};
    charnum = length(s);
    if charnum0 == charnum
        if s == sbegin
            Ibegin = i;
        end
    end
end
%pick the section with all variables and their addresses.
section = text{1}(Ibegin+4:linenum);
%clear variables during processing the map file
clear text charnum charnum0 charnum1 Ibegin linenum s sbegin send
varnum = length(section);
fidw=fopen('vartable.txt','w');
vari = 0;
for i = 1:varnum
    ss=strsplit(section{i,1});
    if length(ss) > 2
        vari = 0;           
        if ((ss{1}(1) >= 65) && (ss{1}(1) <= 90)) || ((ss{1}(1) >= 97) && (ss{1}(1) <= 122))
            for j = 2:length(ss{1})
                if ss{1}(j) == '$'
                    vari = 1;
                end
            end
        else
            vari=1;
        end
        if (length(ss{2})==10) && (ss{2}(2) == 'x') && (vari == 0)
              sd=hex2dec(ss{2}(3:10));
                sym=ss{1};
                sh=ss{2};
                fprintf(fidw, '%s %s %u\n', sym, sh, sd);
        end
    end
end
fclose(fidw)
clear
%prepare the data file
fida = fopen('vartable.txt'); 
%fid = fopen('LEDTestMap1a.xlsx', 'rt'); 
lines = textscan(fida,'%s%s%d'); %reads line by line 
%lines = textscan(fid,'%[NNeg ^\n]'); %reads line by line 
fclose(fida); 
%L = find(~cellfun(@isempty,strfind(lines, 'CmdA')),1,'first'); 
%if ~isempty(L) 
%fprintf('found in %s at line %d\n', 'LEDTest.map', L); 
%end 

for n = 1:numel(lines{:,1})
    string=lines{1,1}(n);
    s=string{1};
    vn = lines{1,3}(n);
    assignin('base',char(s),vn);
end
clear string str vn s n fida
save varmapdat_stm32f407
clear

