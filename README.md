# Hungarian Letter of Frequency Calculator
700 lines, 10 days work

hlfc - Hungarian letter frequency counter.<br>
Knowledge:  Unicode, Codepage, C Language

```
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
            2022.11.01  Ver 0.4     Add calculation of business hours in a year
```

# Output which is in the file "hlfcResult.txt"
```
---------[Grand Total]
 /---------------------\   /---------------------\   /---------------------\
 |E|10.2% xxxxxxxxxx:  |   |Á| 3.1% xxx.         |   |Ó| 0.8% :            |
 |A| 9.0% xxxxxxxxx:   |   |É| 2.7% xx:          |   |Ő| 0.7% :            |
 |T| 8.1% xxxxxxxx:    |   |Y| 2.5% xx:          |   |Ü| 0.5% .            |
 |N| 6.3% xxxxxx:      |   |D| 2.4% xx.          |   |Í| 0.2%              |
 |L| 5.7% xxxxxx       |   |H| 2.0% xx           |   |Ú| 0.2%              |
 |S| 5.6% xxxxxx       |   |B| 1.9% xx           |   |W| 0.1%              |
 |O| 4.7% xxxxx        |   |V| 1.9% xx           |   |Ű| 0.1%              |
 |I| 4.5% xxxx:        |   |U| 1.3% x.           |   |X| 0.0%              |
 |K| 4.5% xxxx:        |   |J| 1.2% x            |   |Q| 0.0%              |
 |R| 4.3% xxxx.        |   |F| 1.1% x            |   |Ä| 0.0%              |
 |M| 4.0% xxxx         |   |C| 1.1% x            |   |Ô| 0.0%              |
 |Z| 3.7% xxxx         |   |P| 1.0% x            |   |Ç| 0.0%              |
 |G| 3.6% xxx:         |   |Ö| 1.0% x            |   | |                   |
 \--------+-+-+-+-+-+-*/   \--------+-+-+-+-+-+-*/   \--------+-+-+-+-+-+-*/
          0 2 4 6 8 10              0 2 4 6 8 10              0 2 4 6 8 10    
          % % % % % % 12%+          % % % % % % 12%+          % % % % % % 12%+
Total letters                          :  3771703
 - Punctuations    in Total letters    :   185366 ( 4.9%)
 - [0-9] numbers   in Total letters    :     5651 ( 0.1%)
 - Total Alphabets in Total letters    :  3580794 (94.9%)
    -  Hungarian áéíóőöúűü in Alphabets:   332149 ( 9.3%)
[Typing Speed]
  Method[a] :   416.0 hours - Hungarian keyboard (using 23 to 48wpm)
  Method[b] :   837.5 hours - Use mouse (using 6 to 48wpm)
  Method[c] :   391.0 hours - Use shortcut key (using 24 to 48wpm)
If 50% of business hours need to type whole in a year,
  Method[b] is the slowest, able to type 4696107991962213867 words in a year.
  Method[a] reduces 511.3 hours (127.8 business days 4h typing) than Method[b]
  Method[c] reduces 541.6 hours (135.4 business days 4h typing) than Method[b]
  Method[c] reduces  30.3 hours (  7.6 business days 4h typing) than Method[a]
-----------------------------------------------------------------------------------
[Configuration]
  Typing Speed (same as familiar keyboard       :   47.5 [wpm] (0.252632 sec/letter)
  Typing Speed (different from familiar keyboard:   22.9 [wpm] (0.522876 sec/letter)
  Typing Speed (using mouse back to the keyboard:    6.0 [wpm] (2.000000 sec/letter)
  Hungarian business days in a year, 2022       :  254   [days]
  Business typing hours in a day                :    4   [hours]
  wpm:  word per minute (common sense)          :    5   [letters]
```
