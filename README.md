# Hungarian Letter of Frequency Calculator
700 lines, 10 days work

hlfc - Hungarian letter frequency counter.<br>
Knowledge:  Unicode, Codepage, C Language


Source code:  hlfc.c
Exe    file:  hlfc.exe

   Input:   File hlfcBookList.txt   - file list to be read (UTF8 BOM file)
            Files in the above lit will be automatically read, assumed code page = 1250 (Central Europe)
   Output:  File hlfcResult.txt     - result
   NOTE:    files have to be placed in the same location to the exe file
            Books file's code page is 1250 (Central Europe).  Not Unicode/UTF-8.

   Written: DQ4WX0 - Takahiro FUJIWARA 
            2022.10.28. Initial version
            2022.10.29  Ver 0.1     Add alphabet counter and the percent will be total in the alphabet count.
            2022.10.30  Ver 0.2     Calculate hungarian letter áéíóőöúűü total occurence.
            2022.10.31  Ver 0.3     Add typeing speed.  Support UTF-8 BOM header for the output file.
            2022.11.01  Ver 0.4     Add calculation of business hours in a year.
