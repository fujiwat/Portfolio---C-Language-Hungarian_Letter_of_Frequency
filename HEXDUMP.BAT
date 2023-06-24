@chcp 1250		&rem 1250: Central Europe Windows
@del $hexdump.txt 1>nul 2>nul
@certutil -encodehex %1 $hexdump.txt &more $hexdump.txt
@del $hexdump.txt
@pause