/*************************************************************************************************************

   hlfc - Hungarian letter frequency counter
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
            2022.11.03  Ver 0.5     Support typing error correction rate.  Bug fix for the slowest, 2nd slowest logic
 *************************************************************************************************************/
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#   include <windows.h>
#endif
#include <locale.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define PROGNAME "hlfc"
#define BOOKLIST (PROGNAME "BookList.txt")
#define OUTPUTFILE (PROGNAME "Result.txt")
#define COMMENTSYMBOL '#'                               // comment start symbol in the book list
#define LC_CTYPE_HUNGARY    "Hungarian_Hungary.1250"    // 2nd parameter for setlocale.  good value: "Hungarian_Hungary.1250"
                                                        // 1250 is the code page (central Europe, windows)
                                                        // isalpha() works with hungarian letters áéíóőöúűü ÁÉÍÓŐÖÚŰÜ
                                                        // other function need to check.  e.g. toupper(), ispunct()...
#define BOM_UTF8    "\xef\xbb\xbf"                      // BOM (Byte Order Mark) for UTF-8
#define NUM_OF_TYPINGMETHOD         3                   // number of typing Method to be estimated the time

// ------------------------ Letter Frequency
struct bookFrequency {
    char *bookTitle;
    struct letterFrequency {
        unsigned long long c[256];                      // frequency count of this letter in a book
        unsigned char sortIdx[256];                     // sort index
        unsigned long long totalAlphabets;              // count of total hungarian alphabet
        unsigned long long totalHungarian;              // count of total hungarian special letter (only áéíóőöúűü ÁÉÍÓŐÖÚŰÜ) 
        unsigned long long punctuation;                 // count of punctuation letters, using ispunct1250()
        unsigned long long digit;                       // count of digit numbers, using isdigit()
        unsigned long long totalLetters;                // count of total letters = without white space, using isspace()
        struct typingTimeforBook {
            double typingSecondForBook;                 // total typing time for this book
            int    sortIdx;                             // slow index of this method
        } typingMethod[NUM_OF_TYPINGMETHOD];
                                                        // calcuated speed in a program
    } lf;
};

struct totalFrequency {
    int books;
    struct letterFrequency lf;
} grandTotal = { 0 };

// ------------------------ Typing Speed
#define TYPINGSPEED_REGULARPOS      (60./(50.*0.95*5))   // 40-60wpm  (average 50 * accuracy(95%)     https://thenaturehero.com/type-with-two-fingers/
#define TYPINGSPEED_UNREGULARPOS    (60./(27.*0.85*5.))  // 27wpm (slow averag 27 * accuracy(85%)  in two-fingers typing speed. https://thenaturehero.com/type-with-two-fingers/
#define TYPINGSPEED_MOUSE           (2.0)           // 2 sec / letter.  this speed is introduced in https://www.linkedin.com/pulse/20140723182611-2483196-how-keyboard-shortcuts-could-double-gdp-growth/
#define LETTERS_PER_WORD            5               // 1 word = 5 letters is a common sense.
#define HUNGARIAN_LOWERLETTERS      "\xe1\xe9\xed\xf3\xf5\xf6\xfa\xfb\xfc"     // áéíóőöúűü
#define HUNGARIAN_UPPERLETTERS      "\xc1\xc9\xcd\xd3\xd5\xd6\xda\xdb\xdc"     // ÁÉÍÓŐÖÚŰÜ
#define KEYBOARD_LETTERS_JP         ("1234567890abcdefghijklmnopqrstuvwxyz" "!\"#$%&'()=~|`{+*}<>?_-^\\@[;:],./")   // the keyboard letter in Japan and US is the same.
#define KEYBOARD_LETTERS_US         ("1234567890abcdefghijklmnopqrstuvwxyz" "~`!@#$%^&*()_+{}|:\"<>?-=[]\\;',./")   // only the position is little bit different
#define KEYBOARD_REGLARPOS_JP       "123456789abcdefghijklmnopqrstuvwx,.\"%()" // means position is same as Hungarian Keyboard.
#define KEYBOARD_REGLARPOS_US
#define BUSINESS_WORKINGHOURS       8               // business working hours in a day
#define BUSINESS_TYPINGHOURS        4               // business typing hours in a day
#define BUSINESS_DAYS_IN_YEAR       254             // 2022 working business days in a year (in Hungary)
struct typingMethod {
    char *shortName10;                              // method short name (max 10 char)
    char *name;                                     // long name
    struct {
        char letter[256];
        double typeSpeed;
    } regularPosition;
    struct {
        double typeSpeed;
    } unregularPosition;
} typingMethod[NUM_OF_TYPINGMETHOD] = {
    {   // [0]
        "Method[a]", "Hungarian keyboard",
        //  if the hungarian letter is same position on the familiar keyboard, then the speed is TYPINGSPEED_REGULARPOS
        {   // regular position letters - regular position means the position is same as compared KEYBOARD (JP/US/...)
            KEYBOARD_REGLARPOS_JP,
            TYPINGSPEED_REGULARPOS
        },
        //  other keys - different position, then the speed will be TYPINGSPEED_UNREGULARPOS which means similar to two-fingers typing
        {   // unregular position letters
            TYPINGSPEED_UNREGULARPOS                // other letter speed is slow. = speed is almost same as unregular key.
        }
    },
    {   // [1]
        "Method[b]", "Use mouse",
        // This is the base - familiar keyboard. speed is fast which means TYPINGSPEED_REGULARPOS
        {   // regular position letters
            KEYBOARD_LETTERS_JP,                    //  This is the regular position 
            TYPINGSPEED_REGULARPOS
        },
        // however, other letter = unable to type by the regular key = need to use screen keyboard = need mouse and back to the keyboard.
        {   // unregular position letters           // The time of 'use mouse and back to the keyboard'
            TYPINGSPEED_MOUSE
        }
    },
    {   // [2]
        "Method[c]", "Use shortcut key",
        // This is also the base - familar keyboard.  speed is fast which means TYPINGSPEED_REGULARPOS
        // (there is no unfamiliar key on the keyboard.  A letter which is not on in the list here, that means hungarian special letter.)
        { 
            KEYBOARD_LETTERS_JP,
            TYPINGSPEED_REGULARPOS
        },
        // however, other letter = unable to type by the regular key = need to use shortcut key.
        { 
            TYPINGSPEED_REGULARPOS*2                // shortcut key.  E.g., Ctrl+' then a = á.  that means 2 key stroke.
        }
    }   
};

// -------------------------------- General libraries
FILE *spOutputFile;                                 // all output will be here

/**********************************************
    libraries
***********************************************/
char *ltrim(char *s) {
    while(isspace(*s)) {
        s++;
    }
    return s;
}

char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*(--back))) {
        // do nothing
    }
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s)); 
}

static unsigned char hungarianLowerLetters[] = HUNGARIAN_LOWERLETTERS;
static unsigned char hungarianUpperLetters[] = HUNGARIAN_UPPERLETTERS;
//                                      –   “   ’   ‘   …   „   ‚   «   °    
static unsigned char hungarianPunctuation[]  = "\x96\x93\x92\x91\x85\x84\x82\xab\xb0"
//                                      »   ×   ä   ç   ô
                                      "\xbb\xd7\xe4\xe7\xf4";
/*
    isHungarian() for CP1250
    true only for Hungarian special letter áéíóőöúűü ÁÉÍÓŐÖÚŰÜ
*/
int isHungarian(int c) {
    for ( int i=0; i<sizeof(hungarianLowerLetters)/sizeof(hungarianLowerLetters[0]); i++ ) {
        if (c == hungarianLowerLetters[i]) {
            return true;
        }
    }
    for ( int i=0; i<sizeof(hungarianUpperLetters)/sizeof(hungarianUpperLetters[0]); i++ ) {
        if (c == hungarianUpperLetters[i]) {
            return true;
        }
    }
    return false;
}
/*
    toupper() for CP1250
    also convert áúóíéüöűő to ÁÚÓÍÉÜÖŰŐ 
*/
int toupper1250(int c) {
    for ( int i=0; i<sizeof(hungarianLowerLetters)/sizeof(hungarianLowerLetters[0]); i++ ) {
        if (c == hungarianLowerLetters[i]) {
            return hungarianUpperLetters[i];
        }
    }
    return toupper(c);
}

/*
    isalpha() for CP1250
    also true for áúóíéüöűő and ÁÚÓÍÉÜÖŰŐ
*/
int isalpha1250(int c) {
    if ( isHungarian(c) ) {
        return true;
    }
    return isalpha(c);
}

int ispunct1250(int c) {
    for ( int i=0; i<sizeof(hungarianPunctuation)/sizeof(hungarianPunctuation[0]); i++ ) {
        if ( c == hungarianPunctuation[i] ) {
            return true;
        }        
    }
    return ispunct(c);
}
/*
    Assume that the character is Code Page 1250 = Central Europe
    Change CP1200 character to UTF8 for able to print
*/
char *toPrintableChar1250(char c, char *unicodeStr, int maxLen) {
    if ( c==' ' || !isspace(c) ) {
        wchar_t wcStr[2] = { '\0', '\0' };
        unicodeStr[0] = c;
        unicodeStr[1] = '\0';
        MultiByteToWideChar(1250, 0, unicodeStr, 1, wcStr, sizeof(wcStr)/sizeof(wcStr[0])); // assume CP1250 and change it to widechar
        WideCharToMultiByte(CP_UTF8, 0, wcStr, sizeof(wcStr)/sizeof(wcStr[0]), unicodeStr, maxLen, NULL, NULL); // change widechar to utf8
        return unicodeStr;
    }
    return "_";             // unprintable character
}

#if defined(_WIN32) || defined(_WIN64)
/*
    able to open the file which name is UTF8
    NOTE:   here, UTF8 is not the content.  only the file name.
*/
FILE *fopenUtf8(char *fname, char *fmode) {
    wchar_t WCfname[512];
    wchar_t WCfmode[128];
    MultiByteToWideChar(CP_UTF8, 0, fname, -1, WCfname, sizeof(WCfname)/sizeof(WCfname[0]));    // convert UTF-8 string to wchar_t
    MultiByteToWideChar(CP_UTF8, 0, fmode, -1, WCfmode, sizeof(WCfmode)/sizeof(WCfmode[0]));    // convert UTF-8 string to wchar_t
    return _wfopen(WCfname, WCfmode);
} 
#else
/*
    in the Linux, fopenUtf8() is same as fopen()
*/
#   define fopenUtf8(fname, fmode) fopen(fname, fmode)
#endif

// ------------------------------------ Solution for the task
/*
    count how many books need to process.
    File = book.  Title = file name. the files are in the book list
*/
int step01_countBookList(char* inFName) {
    FILE *spIn;
    char linebuf[512];
    char *linebufp;
    int books = 0;
 
    if ( (spIn=fopen(inFName, "r")) == NULL ) {
        fprintf(stderr, "***Error line %d:  file read open error:  %s\n", __LINE__, inFName);
        return 0;
    }
    for (int i=0; fgets(linebuf, sizeof(linebuf), spIn); i++) {
        char *trimLine = trim(linebuf);
        if ( *trimLine != '\0' && *trimLine!=COMMENTSYMBOL ) {
            books++;            
        }
    }
    fclose(spIn);
    return books;
}

/*
    Initialize Letter Frequency table for a book.
    Input:  pointer of totalFrequency
            pointer of the top of bookFrequency table[]
            books:  number of books
    Ouput:  TotalFrequeny Table
            bookFrequeny Table
*/
void step02_initializeLf(struct totalFrequency *pGt, struct bookFrequency *pBf, int books) {
    memset(pGt, 0, sizeof(*pGt));
    for (int i=0; i<sizeof(pGt->lf.sortIdx)/sizeof(pGt->lf.sortIdx[0]); i++) {
        pGt->lf.sortIdx[i] = i;             // initialize sort index
    }
    memset(pBf, 0, sizeof(struct bookFrequency)*books);
    for (int i=0; i<books; i++) {
        for (int j=0; j<sizeof(pBf->lf.sortIdx)/sizeof(pBf->lf.sortIdx[0]); j++) {
            pBf[i].lf.sortIdx[j] = j;       // initialize sort index
        }
    }
}

/*
    read the book title from the book list.
    Input:  inFName:    file name of the book list
            pBF:        pointer for the struct bookFrequency
            books:      number of the books in the book list, this number is already got from function step01_countBookList();
*/
int step03_readBookList(char* inFName, struct bookFrequency *pBf, int books) {
    FILE *spIn;
    char linebuf[512];
    char *linebufp;

    if ( inFName ) {
        grandTotal.books = 0;
        if ( (spIn=fopen(inFName, "r")) == NULL ) {
            fprintf(stderr, "***Error line %d:  file read open error:  %s\n", __LINE__, inFName);
            return 0;
        }
        for (int i=0; fgets(linebuf, sizeof(linebuf), spIn); i++) {
            char *trimLine = trim(linebuf);
            if ( *trimLine != '\0' && *trimLine!=COMMENTSYMBOL ) {
                int len=strlen(trimLine);
                pBf[grandTotal.books].bookTitle = malloc(sizeof(char)*(strlen(trimLine)+1) );
                strncpy(pBf[grandTotal.books].bookTitle, trimLine, sizeof(char)*(strlen(trimLine)+1));
                grandTotal.books++;
            }
        }
        fclose(spIn);
        return (grandTotal.books==books? books: 0);
    } else { // terminate procedure
        for ( int i=0; i<books; i++ ) {
            free(pBf[i].bookTitle);
        }
    }
}

/*
    memory free for 03_readBookList
*/
int terminate03_readBookList(char* inFName, struct bookFrequency *pBf, int books) {
    return step03_readBookList(NULL, pBf, books);
}

#define BARCHART_LEN_PERCENT_NULL   7       // value of strlen("00.0% ") + 1 (for null terminate)   default: 7
#define BARCHART_BARLEN             13      // length of bar char (max bar length)                  default: 13
#define BARCAHRT_SATURATION         12.0    // percentage of saturation (double)                    default: 12.0
#define BARCHART_HOW_MANY_IN_LINE   3       // 3 bar char in one line                               need to be: 3
/*
    barChar function
    sortLF: soft index only for the Letter Frequency Table.    
*/
void sortLf(struct letterFrequency *pLf) {
    for (int i=0; i<sizeof(pLf->c)/sizeof(pLf->c[0])-1; i++) {
        for (int j=i+1; j<sizeof(pLf->c)/sizeof(pLf->c[0]); j++) {
            if ( pLf->c[ pLf->sortIdx[i] ] < pLf->c[ pLf->sortIdx[j] ] ) {
                int tmp = pLf->sortIdx[i];
                pLf->sortIdx[i] = pLf->sortIdx[j];
                pLf->sortIdx[j] = tmp;
            }
        }
    }
}

/*
    barChart function
    static character table for the chartBar 
*/
static char *barChart_xStr = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
static char *barChart_xSpc = "                                                                           "
                             "                                                                           ";
static char *barChart_head = "---------------------------------------------------------------------------"
                             "---------------------------------------------------------------------------";
static char *barChart_foot = "---------------------------------------------------------------------------"
                             "---------------------------------------------------------------------------";                             
/*
    barChart function
    create a table header for one table
*/
char *barChartHeader(char *pStr, int maxLen, double saturat) {
    snprintf(pStr, maxLen+BARCHART_LEN_PERCENT_NULL, "%.*s%.*s",7, barChart_head, maxLen, barChart_head); 
    return pStr;
}

/*
    barChart function
    create a table footer for one table
*/
char *barChartFooter(char *pStr, int maxLen, double saturat) {
    snprintf(pStr, maxLen+BARCHART_LEN_PERCENT_NULL, "%.*s%.*s",7, barChart_head, maxLen, barChart_head); 
    return pStr;
}

/*
    barChart function
    create a table content for one table
*/
char* barChart(char *pStr, int maxLen, double saturat, double value) {
    int numOfSmallx;    // number of Large X
    double lastx_value;    // value of last X
    char *lastStr;
    numOfSmallx = (int)(value / saturat * maxLen);
    lastx_value = value - (double)numOfSmallx / maxLen * saturat;
    if ( saturat+(double)saturat/maxLen/3 < value ) {
        numOfSmallx = maxLen - 1;
        lastStr = "*";
    } else {
        if ( lastx_value <= (double)saturat/maxLen/3. ) {
            lastStr = "";
        } else if ( (double)saturat/maxLen/3. < lastx_value && lastx_value < (double)saturat/maxLen*2./3. ) {
            lastStr = ".";
        } else {
            lastStr = ":";
        }
    }
    if ( value==0) {
        snprintf(pStr, maxLen+BARCHART_LEN_PERCENT_NULL, "       %.*s%s%.*s", numOfSmallx, barChart_xStr, lastStr, maxLen-numOfSmallx-strlen(lastStr), barChart_xSpc);
    } else {
        snprintf(pStr, maxLen+BARCHART_LEN_PERCENT_NULL, "%4.1lf%% %.*s%s%.*s", value, numOfSmallx, barChart_xStr, lastStr, maxLen-numOfSmallx-strlen(lastStr), barChart_xSpc);
    }
    return pStr;
}

/*
    calculate the letter frequeny for a book
*/
int step10_calcBookFrequency(struct bookFrequency *pBf) {
    FILE *spIn;
    int c;
    unsigned long lc=0;      // line count
    unsigned long cc=0;      // character count including newline

    if ( (spIn=fopenUtf8(pBf->bookTitle, "r")) == NULL ) {                      //  (not the content.  only the fine name)
        fprintf(stderr, "***Error line %d:  file read open error:  %s\n", __LINE__, pBf->bookTitle);
        return 0;
    }
    for ( ; (c=fgetc(spIn)) != EOF; cc++) {
        char unicodeStr[8];
        // count ecah letter
        pBf->lf.c[(unsigned char)toupper1250((unsigned char)c)]++;
        grandTotal.lf.c[(unsigned char)toupper1250((unsigned char)c)]++;
        // count punctuation letters
        pBf->lf.punctuation += ispunct1250((unsigned char) c)? 1: 0;
        grandTotal.lf.punctuation += (ispunct1250((unsigned char) c)? 1: 0);
        // count [0-9] digit numbers
        pBf->lf.digit += (isdigit((unsigned char)c)? 1: 0);
        grandTotal.lf.digit += (isdigit((unsigned char)c)? 1: 0);
        // count Hungarian special letters
        if ( isHungarian(c) ) {
            (pBf->lf.totalHungarian)++;
            grandTotal.lf.totalHungarian++;
        }
        // count alphabet letters
        if ( isalpha(c) ) {
            (pBf->lf.totalAlphabets)++;
            grandTotal.lf.totalAlphabets++;
        }
        // count all letters
        if ( !isspace(c) ) {
            (pBf->lf.totalLetters)++;
            grandTotal.lf.totalLetters++;
        }
        // test
        // fprintf(spOutputFile, "%s", toPrintableChar1250(c, unicodeStr, sizeof(unicodeStr)/sizeof(unicodeStr[0])) );
        // count lines (not used)
        if ( c=='\n' ) {
            lc++;
        }
    }
    fclose(spIn);
    return cc;
}

double getLetterSpeed(char c, int method) {
    for ( int i=0; i<sizeof(typingMethod[method].regularPosition.letter)/sizeof(typingMethod[method].regularPosition.letter[0]); i++) {
        if ( toupper(c)==toupper1250(typingMethod[method].regularPosition.letter[i]) ) {
            return typingMethod[method].regularPosition.typeSpeed;
        }
    }
    return typingMethod[method].unregularPosition.typeSpeed;
};

int speedToWpm(double speedPerLetter) {
    return (int)(60. / (speedPerLetter * 5.) +0.5);
}

void step20_calcTypingSpeed(char *bookName, struct letterFrequency *pLf) {
    fprintf(spOutputFile, "[Typing Speed]\n");
    for ( int method=0; method<sizeof(typingMethod)/sizeof(typingMethod[0]); method++ ) {
        double typingSecondForBook = 0.;
        for ( int i=0; i<sizeof(pLf->c)/sizeof(pLf->c[0]); i++) {
            typingSecondForBook += getLetterSpeed(i, method) * pLf->c[i];
        }
        pLf->typingMethod[method].typingSecondForBook = typingSecondForBook;
        fprintf(spOutputFile, "  %-10s: %7.1lf hours - %s (using %d to %dwpm)\n",
            typingMethod[method].shortName10, typingSecondForBook/(60*60), 
            typingMethod[method].name, 
            speedToWpm(typingMethod[method].unregularPosition.typeSpeed),
            speedToWpm(typingMethod[method].regularPosition.typeSpeed) );
    }
}

void step21_calcBusinessHours(char *bookName, struct letterFrequency *pLf) {
    fprintf(spOutputFile, "If %.lf%% of business hours need to type whole in a year,\n",
        (double)BUSINESS_TYPINGHOURS / BUSINESS_WORKINGHOURS * 100. ) ;
    // initialize sort index
    for ( int i=0; i<sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0]); i++) {
        pLf->typingMethod[i].sortIdx = i;
    }
    // index sort
    for ( int i=0; i<sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0])-1; i++ ) {
        for ( int j=i+1; j<sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0]); j++ ) {
            if ( pLf->typingMethod[i].typingSecondForBook < pLf->typingMethod[j].typingSecondForBook ) {
                int tmp = pLf->typingMethod[i].sortIdx;
                pLf->typingMethod[i].sortIdx = pLf->typingMethod[j].sortIdx;
                pLf->typingMethod[j].sortIdx = tmp;
            }
        }
    }
    // get slower order
    int slowerIdx[sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0])];
    for ( int i=0; i<sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0]); i++ ) {
        slowerIdx[pLf->typingMethod[i].sortIdx] = i;
    }
    // print
    double lettersPerYear0 = (double)(BUSINESS_DAYS_IN_YEAR * BUSINESS_TYPINGHOURS) * 60 * 60 
                           / (pLf->typingMethod[slowerIdx[0]].typingSecondForBook / pLf->totalLetters);
    double reduceSeconds[sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[slowerIdx[0]])];
    fprintf(spOutputFile, "  %s is the slowest, able to type %llu words in a year.\n",
        typingMethod[slowerIdx[0]].shortName10,
        lettersPerYear0 / LETTERS_PER_WORD );
    for (int i=1; i<sizeof(pLf->typingMethod)/sizeof(pLf->typingMethod[0]); i++ ) {
        double needSeconds = lettersPerYear0 * (pLf->typingMethod[slowerIdx[i]].typingSecondForBook) / (double)pLf->totalLetters;
        reduceSeconds[slowerIdx[i]] = (double)(BUSINESS_DAYS_IN_YEAR * BUSINESS_TYPINGHOURS * 60 * 60) - needSeconds;
        fprintf(spOutputFile, "  %s reduces %5.1lf hours (%5.1lf business days %dh typing) than %s\n",
            typingMethod[slowerIdx[i]].shortName10,
            reduceSeconds[slowerIdx[i]]/(60*60),
            reduceSeconds[slowerIdx[i]]/(60*60)/BUSINESS_TYPINGHOURS,
            BUSINESS_TYPINGHOURS,
            typingMethod[slowerIdx[0]].shortName10
        );
    }
    fprintf(spOutputFile, "  %s reduces %5.1lf hours (%5.1lf business days %dh typing) than %s\n",
        typingMethod[slowerIdx[2]].shortName10,
        (reduceSeconds[slowerIdx[2]]-reduceSeconds[pLf->typingMethod[1].sortIdx])/(60*60),
        (reduceSeconds[slowerIdx[2]]-reduceSeconds[pLf->typingMethod[1].sortIdx])/(60*60)/BUSINESS_TYPINGHOURS,
        BUSINESS_TYPINGHOURS,
        typingMethod[slowerIdx[1]].shortName10
    );
}

/*
    print the letter frequency for a book
*/
int step11_printBookFrequency(char *bookName, struct letterFrequency *pLf) {
    int idx;
    int printCount;
    int lineCount;
    unsigned char dispMatrix[(256+BARCHART_HOW_MANY_IN_LINE-1)/BARCHART_HOW_MANY_IN_LINE][BARCHART_HOW_MANY_IN_LINE] = { 0 }; // display matrix
    fprintf(spOutputFile, "\n---------%s\n", bookName);
    sortLf(pLf);
    // step 1. get printCount and lineCount
    printCount = 0;
    for ( idx=0; idx<sizeof(pLf->c)/sizeof(pLf->c[0]); idx++) {
        int c = pLf->sortIdx[idx];                      // get a letter which has the biggest percentage
        if ( pLf->c[c] ) {                              // check the value of it percentage
            if ( !ispunct1250(c) ) {                    // if the character is not punctuation (=alpha numeric)
                if ( !isspace(c) ) {                    // if the character is not white space
                    if ( !isdigit(c) ) {                // if the characger is not digit numbers
                        printCount++;
                    }
                }
            }
        }
    }
    lineCount = (printCount + BARCHART_HOW_MANY_IN_LINE - 1)  / BARCHART_HOW_MANY_IN_LINE; 

    // step2. set char to the position of the matrix
    printCount = 0;
    for ( idx=0; idx<sizeof(pLf->c)/sizeof(pLf->c[0]); idx++) {
        int c = pLf->sortIdx[idx];                      // get a letter which has the biggest percentage
        if ( pLf->c[c] ) {                              // check the value of it percentage
            if ( !ispunct1250(c) ) {                    // if the character is not punctuation (=alpha numeric)
                if ( !isspace(c) ) {                    // if the character is not white space
                    if ( !isdigit(c) ) {                // if the character is not digit numbers
                        int row = printCount % lineCount;
                        int col = printCount / lineCount;
                        dispMatrix[row][col] = c;   // set a letter
                        printCount++;
                    }
                }
            }
        }
    }

    // step 3. print the bar chart
    for (int section=0; section<3; section++) {             // section: 1=header, 2=bar chart, 3=footer
        for ( int row=0; row<lineCount; row++ ) {
            if ( (section==0 && row==0) || (section==1) || (section==2 && row==0)) {
                fprintf(spOutputFile, " ");
            }
            for ( int col=0; col<BARCHART_HOW_MANY_IN_LINE; col++ ) {
                int c = dispMatrix[row][col];
                char unicodeStr[8];
                char barString[3][BARCHART_BARLEN+BARCHART_LEN_PERCENT_NULL];
                if ( section==0 ) {
                    if ( row==0 ) {                         // print the header
                        if ( col!=0 ) {                     // before the 2nd, 3rd bar chart in a line
                            fprintf(spOutputFile, "   ");               
                        }
                        fprintf(spOutputFile, "/--%s\\",
                            barChartHeader(&(barString[col][0]), BARCHART_BARLEN, (double)BARCAHRT_SATURATION));
                    }
                } else if ( section==1 ) {                  // print the bar chart content
                    if ( col!=0 ) {                         // before the 2nd, 3rd bar chart in a line
                        fprintf(spOutputFile, "   ");               
                    }
                    fprintf(spOutputFile, "|%s|%s|",
                        toPrintableChar1250(c?c:' ', unicodeStr, sizeof(unicodeStr)/sizeof(unicodeStr[0])),
                        barChart(&(barString[col][0]), BARCHART_BARLEN, (double)BARCAHRT_SATURATION, 100.*(pLf->c[c]) / pLf->totalAlphabets)
                        );
                    if ( c!='\0' && !isalpha1250(c) ) fprintf(stderr, "Line %7d: Found %s(%02x) -- need to implement this.\n", __LINE__, unicodeStr, c );
                } else if ( section==2 ) {
                    if ( row==0 ) {                         // print the footer
                        if ( col!=0 ) {                     // before the 2nd, 3rd bar chart in a line
                            fprintf(spOutputFile, "   ");               
                        }
                        fprintf(spOutputFile, "\\--------+-+-+-+-+-+-*/");
                    } else if ( row==1 ) {                         // print the footer
                        fprintf(spOutputFile, "          0 2 4 6 8 10    ");
                    } else if ( row==2 ) {                         // print the footer
                        fprintf(spOutputFile, "          %% %% %% %% %% %% 12%%+");
                    }                         
                } 
            }
            if ( (section==0 && row==0) || (section==1) || (section==2 && row < 3)) {
                fprintf(spOutputFile, "\n");
            }
        }
    }
    fprintf(spOutputFile, "Total letters                          : %8llu\n", pLf->totalLetters);
    fprintf(spOutputFile, " - Punctuations    in Total letters    : %8lu (%4.1lf%%)\n", pLf->punctuation, 100.*pLf->punctuation/pLf->totalLetters);
    fprintf(spOutputFile, " - [0-9] numbers   in Total letters    : %8lu (%4.1lf%%)\n", pLf->digit, 100.*pLf->digit/pLf->totalLetters);
    fprintf(spOutputFile, " - Total Alphabets in Total letters    : %8llu (%4.1lf%%)\n", pLf->totalAlphabets, 100.*pLf->totalAlphabets/pLf->totalLetters);
    fprintf(spOutputFile, "    -  Hungarian áéíóőöúűü in Alphabets: %8llu (%4.1lf%%)\n", pLf->totalHungarian, 100.*pLf->totalHungarian/pLf->totalAlphabets);
    step20_calcTypingSpeed(bookName, pLf);
    step21_calcBusinessHours(bookName, pLf);
}

void step30_printConfiguration() {
    fprintf(spOutputFile, "-----------------------------------------------------------------------------------\n" );
    fprintf(spOutputFile, "[Configuration]\n" );
    fprintf(spOutputFile, "  Typing Speed (same as familiar keyboard       : % 6.1lf [wpm] (%lf sec/letter)\n", 60./TYPINGSPEED_REGULARPOS/LETTERS_PER_WORD, TYPINGSPEED_REGULARPOS);
    fprintf(spOutputFile, "  Typing Speed (different from familiar keyboard: % 6.1lf [wpm] (%lf sec/letter)\n", 60./TYPINGSPEED_UNREGULARPOS/LETTERS_PER_WORD, TYPINGSPEED_UNREGULARPOS);
    fprintf(spOutputFile, "  Typing Speed (using mouse back to the keyboard: % 6.1lf [wpm] (%lf sec/letter)\n", 60./TYPINGSPEED_MOUSE/LETTERS_PER_WORD, TYPINGSPEED_MOUSE);
    fprintf(spOutputFile, "  Hungarian business days in a year, 2022       : % 4d   [days]\n", BUSINESS_DAYS_IN_YEAR );
    fprintf(spOutputFile, "  Business typing hours in a day                : % 4d   [hours]\n", BUSINESS_TYPINGHOURS );
    fprintf(spOutputFile, "  wpm:  word per minute (common sense)          : % 4d   [letters]\n", LETTERS_PER_WORD );
}

/*
    main() entry.
    usage:  no parameter.   Just run the program.
    Input:  BOOKLIST (PROGNAME "BookList.txt")
    Ouput:  OUTPUTFILE (PROGNAME "Result.txt")
    Error:  stderr
*/
int main(int argc, char* argv[]) {
    struct bookFrequency *pBookFrequency;
    setlocale(LC_CTYPE, LC_CTYPE_HUNGARY);              // enable hungarian letters áéíóőöúűü
    if ( (spOutputFile=fopen(OUTPUTFILE, "w")) == NULL ) {
        fprintf(stderr, "***Error line %d: file open error: %s\n", __LINE__, OUTPUTFILE);
        return 2;
    }
    fprintf(spOutputFile, "%s", BOM_UTF8);              // write BOM header to the UTF-8 file.
/*
    fprintf(spOutputFile, "isalpha(0xe1)=%d\n", isalpha(0xe9));
    char barString[3][BARCHART_BARLEN+BARCHART_LEN_PERCENT_NULL];
    printf("%s\n", barChart(&(barString[0][0]), BARCHART_BARLEN, (double)BARCAHRT_SATURATION), 0.0 );
return 0;
*/
    int books = step01_countBookList(BOOKLIST);                         // read the book list and get the count of books (only the count)
    if ( books ) {
        pBookFrequency = (struct bookFrequency*)
                                 malloc(sizeof(struct bookFrequency)*books);
        if ( pBookFrequency ) {
            step02_initializeLf(&grandTotal, pBookFrequency, books);    // Initialize table of the Letter Frequency for grand total
            step03_readBookList(BOOKLIST, pBookFrequency, books);       // read each book title (in UTF-8) from the book list
            for (int i=0; i<books; i++) {
                if ( step10_calcBookFrequency(&pBookFrequency[i]) ) {   // calculate letter frequency for a book
                    // print letter frequency for a book (if you do not need it then you can comment out the following line)
                    step11_printBookFrequency(pBookFrequency[i].bookTitle, &pBookFrequency[i].lf);
                }
            }
            // pint letter frequency from every books
            step11_printBookFrequency("[Grand Total]", &grandTotal.lf);
        }
    }
    step30_printConfiguration();
    terminate03_readBookList(NULL, pBookFrequency, books);                   // terminate procedure, free()
    fclose(spOutputFile);
    free(pBookFrequency);
}