 /*
Edytor Textové Grafüky
vytvořil Jakub Hroník
*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <dos.h>

/*
text, který se vypisuje v Changelogu, rovnou šoupnutý do příkazu, aby bylo možné použít i znaky s diakritikou (Code:Blocks používá jiné kódování horní části ASCII než Příkazová řádka - nejjednodušší je tedy zjistit číslo potřebných znaků experimentálně a potom ho dosadit do zdrojového kódu
zdá se, že pro 1 řetězec je z nějakého důvodu příliš dlouhý - když jsem to tak
*/
#define CHANGELOG_PART_1 puts("Changelog:\n\nv1.7.2   (3.3.2019)\n-nekolik zmen na zdrojovem kodu, jejichz ucelem je veskrze zmirneni nasledku, jez by jeho pripadne cteni mohlo zanechat na nahodnem ctenari\n\nv1.7.1   (24.2.2019)\n-opraveno mnoho chyb, z nichz vetsina tu byla uz velmi dlouho\n-pridana funkce Rychleho ulozeni\n\nv1.7   (18.2.2019)\n-opravena spusta chyb, ktere byly v RC5 - autorovi se jiz nedari nalezt dalsi\n\nv1.7 RC5   (17.2.2019)\n-pridano subnastaveni pro specielni jidla v Hadovi\n\nv1.7 RC4   (12.2.2019)\n-opraveno nekolik zanedbatelnych drobnosti\n-zdrojovy kod byl obecne trochu zkultivovan\n\nv1.7 RC3   (12.12.2018)\n-nove jiz program pri vypnuti nepada")
#define CHANGELOG_PART_2 printf("\nv1.7 RC2   (9.12.2018)\n-nove program na konci vzdy spadne\n-pridana moznost pri umistovani vybrane oblasti nebo retezce ignorovat bile znaky v nem - ridi se nastavenim Dutosti ctvercu\n-optimalnejsi zpusob barevneho kresleni - nove se nepouziva externi program ColourText.exe\n\nv1.6.1   (25.11.2018)\n-pridany instrukce pro interakci s Nastavenim\n\nv1.6   (19.9.2018)\n-pridan changelog\n\nv1.5   (7.9.2018)\n-pridano Nastaveni\n-moznost pohybu hada i vyhradne po uhloprickach\n-odstranen nefunkcni system pro rychlejsi kresleni cervenou barvou na Windows 10\n\nv1.4  (19.7.2018)\n-pridan Had!\n-zmena zkratky z nespravneho \"EyTGu\" na \"EyTG%c\"\n\nv1.3  (12.7.2018)\n-pridana funkce pro Orez\n-pridana schranka pro funkci Kopirovani oblasti\n\nv1.2 (13.6.2018)\n-pridana Funkce pro vyber a kopirovani oblasti\n\nv1.2 (22.1.2017)\n-tato informace je tajna\n\nv1.1 (18.1.2017)\n-zmeny nezname\n\nv1.0 (davna historie)\n-bude predmetem budouciho badani ", 251)


#define CONFIG_FILE_NAME "eytgu.ini" //název konfiguračního souboru

#define abs(x) (((x)<0)?(-(x)):(x)) //makro vracející absolutní hodnotu

#define MAX_XY 512 //maximum jak pro výšku, tak pro šířku plátna; nemám žádný pořádný důvod, proč bylo stanoveno zrovna takto
#define MIN_X 1
#define MIN_Y 30//pro důvod viz Uživatelský manuál - část 2.0
#define ODSAZENI_Y_OTAZEK 8 //offset, který se přičítá k dolní souřadnici konce plátna, aby se odvodilo místo, kde začíná řádka, ve které probíhá většina systémových dialogů
#define MAX_STRING 60 //hezká hodnota, která stačí většině nekonstantních řetězců, které je v tomto programu potřeba použít
#define MAX_RYCHLA_DELKA (Y_MAX/2*3) //maximální poloměr rychlého čtverce (viz UM-2.8)
#define POCET_K_VYBILENI 120 //počet mezer, které se zpravidla vytisknou při zamazávání Dialogové řádky potom, co bylo odpovězeno na otázku
#define DEF_X_MAX 80 //výchozí hodnota pro šířku obrazovky
#define DEF_Y_MAX 30 //výchozí hodnota pro výšku obrazovky
#define POCET_RADKU_V_NAPOVEDE 32 // kolik řádků zabírá celkem Klávesová nápověda

#define kurzorZaujmoutMisto  (gotoxy(xn,yn), putchar(kresliciZnak), gotoxy(xn,yn)) //jednoduchý zpsůob, jak zajistit, aby byl kurzor na svém místě a byl i vytištěn jeho znak, ale nakonec se zdá, že jsem to použil jen na jediném místě v programu; ale jednou se to třeba bude hodit
int MIN_XY[2] = {MIN_X,MIN_Y}; //minimum pro šířku a pro výšku soupnuté takto do pole, aby se s tím lépe pracovalo v místě programu, kde se to využívá
int pouzivaWin10 = 0;  //dříve mělo význam, ten se však v současné verzi vytratil; třeba se pro to však najde využití např. v případě portu pro Linux

//úvodní hodnoty, které se při spuštění přepíšou
int X_MAX = -1; //80
int Y_MAX = -1; //24
int Y_OTAZEK = DEF_Y_MAX+ODSAZENI_Y_OTAZEK;

//int *sledovani;

int editRezim = 0; //0...Pohyb; 1...Editace; -1...Guma (možná by bývalo bylo přehlednější použít výčtový typ, ale takto se mi s tím lépe pracuje
int cyklPohyb = 1;    //používá se jako čistý boolean
int duteCtverce = 0;  //používá se jako čistý boolean
int natoceni = 0;    //při umísťování textového řetězce či oblasti
int delkaRychleCary = 10;
int polomerRychlehoCtverce = 2;
int vyznacenyVyznBody = 0;//používá se jako čistý boolean; pro funkci vyvolanou stiskem klávesy 'Z'
int rychlostPohybu = 1;

int priNejblizsiPrilezitostiVybilitSpodniRadku = 0; //pokud == true, program při následujícím stisknutí nějaké klávesy (pokud jsme v základním editačním režimu) vybílí Dotazníkovou řádku

char kresliciZnak = 'X'; //znak kurzoru

//samotné dvojrozměrné pole, které uchovává celý obsah pracovního plátna; první rozměr je bohužel X-ová souřadnice a druhý je Y. Nemohu tedy plátno vypisovat po celých řádkách (každý jako 1 řetězec), nýbrž jedině znak po znaku; uznávám, že to ode mě bylo velmi nerozvážné, avšak překopávat kvůli této chvbě celý program se mi již nechce
char **zn = NULL; //nulty řádek a sloupec ponechávám prázdný, abych nemusel od souřadnic všude odečítat jedničku (protože nultý řádek a sloupec je zabrán rámečkem, který není součaátí samotného plátna)

//používají funkce pro Výběr a kopírování a pro Ořez plátna
char **vybrObl = NULL;
int oblX = 0, oblY = 0;



char puvTlOvladani[47] = "wsadqeyc789461230,/*-+ ;jknmplutz5h.v=)o\\|@";//výčet všech znaků, které se používají k orientaci někde v programu
char nastaveneTlOvladani[47];//uchovává nové, uživatelem konfigurované rozložení kláves, které se po přečtení převedou na prvek z pole původních znaků se stejným indexem a ten se pak porovnává s hodnotami ve Switch-case

char jmenoSouboru[MAX_STRING] = ""; //jméno posledního editovaného či otevřeného souboru - pro funkci Rychlého uložení
#define MAX_DELKA_JMENA_PROGRAMU 40
char nazevProgramu[MAX_DELKA_JMENA_PROGRAMU];
char programTitle[MAX_STRING+MAX_DELKA_JMENA_PROGRAMU]; //pro titulek, který se zobrazuje nahoře na okně s programem
#define nastavitJmenoAktualnihoSouboru(co) (strcpy(jmenoSouboru, co), strcpy(programTitle+strlen(nazevProgramu), jmenoSouboru), SetConsoleTitle(programTitle))

int naZacatkuNacteniSOrezem = 0; //boolean; pravdivé v případě, že byl program otevřen tak, že se rovnou načetl soubor, a ten se nevešel do limitu velikosti plátna a došlo tudíž k ořezu - aby program mohl uživateli vypsat příslušné hlášení
int xn=1,yn=1;//souřadnice, na kterých se aktuálně nalézá kurzor

/*!had!*/
//bílé znaky, pod kterými se interně ukládají prvky, které používá had, aby se daly rozlišit od znaků obyčejných; přiřazeny víceméně náhodně
#define C_JIDLA 6
#define C_JIDLA_S 5
#define C_HAD 15
#define C_NA_NULA_NULA 3

//výchozí hodnoty znaků, které se na místě znaků výše skutečně vykreslují
#define DEFAULT_ZN_JIDLA '*'
#define DEFAULT_ZN__HLAVA 'O'
#define DEFAULT_ZN__TELO '#'

#define DEFAULT_HAD_CHODI_SIKMO 1 //výchotí hodnota pro HAD_CHODI_SIKMO
#define DEFAULT_PKOST_JIDLA_S 22000

#define DRUHU_SPEC_JIDEL 3  //výchozí počet specielních jídel
#define MAX_DRUHU_SPEC_JIDEL 18
#define MIN_DRUHU_SPEC_JIDEL 0

//znaky, které se na místě prvků v hadovi ve skutečnosti vykreslují
char ZN_JIDLA = DEFAULT_ZN_JIDLA;
char ZN_HLAVA = DEFAULT_ZN__HLAVA;
char ZN_TELO = DEFAULT_ZN__TELO;

int pkostJidlaS = DEFAULT_PKOST_JIDLA_S; /*pokud rand() <= toto, objeví se Specielní jídlo   pozn. rand() vrací číslo do 32767*/


/*vlastnosti každého z jídel; indexuje se pořadím daného specielního jídla*/
char *ZN_JIDLA_S = NULL; //inicializuje se při spuštění ve funkci inicializovatHada()
int *trvaniJidlaS = NULL; //čas v milisekundách
int *hodnotyJidlaS = NULL; //v bodech



int hadiRekord = 0; //maximální dosažené skóre

int HAD_CHODI_SIKMO = DEFAULT_HAD_CHODI_SIKMO; //1..pouze rovně; -1...pouze šikmo; 0...všechny směry povoleny
int ZDRZENI_HADA = 100; //čas v milisekundách
int povolenoSpecialniJidlo = DRUHU_SPEC_JIDEL; //počet druhů specielních jídel; může být i záporné - potom se počet jídel daný absolutní hodnotou udržuje v paměti, aby při případném opětovném zapnutí nemusel uživatel vše konfigurovat odznova, ale žádné specielní jídlo se ve hře neobjeví
int puvodniPovolenoSpecialniJidlo = 0; //kolik bylo druhů specielních jídel naposledy - pomůcka při alokování paměti pro další jídla při změně jejich počtu


int alokovatPametProSpecJidlo(){
    if(povolenoSpecialniJidlo){
        if((ZN_JIDLA_S = (char *) realloc(ZN_JIDLA_S,abs(povolenoSpecialniJidlo)*sizeof(char)))==NULL) return -1;
        if((trvaniJidlaS = (int *) realloc(trvaniJidlaS,abs(povolenoSpecialniJidlo)*sizeof(int)))==NULL) return -1;
        if((hodnotyJidlaS = (int *) realloc(hodnotyJidlaS,abs(povolenoSpecialniJidlo)*sizeof(int)))==NULL) return -1;
    } else{
        free(ZN_JIDLA_S); free(trvaniJidlaS); free(hodnotyJidlaS); ZN_JIDLA_S = NULL; trvaniJidlaS = hodnotyJidlaS = NULL;
    }
    return 0;
}
inicializovatDefaultSpecielniJidlo(int alokovatPamet){
    int t;

    if(abs(puvodniPovolenoSpecialniJidlo) == abs(povolenoSpecialniJidlo)) return 0; //pokud již je alokována paměť přesně pro počet znaků, jaký máme aktuálně, není třeba alokovat žádnou další

    if(alokovatPamet && alokovatPametProSpecJidlo()) { //funkce pro alokaci paměti měla být a byla zavolána, ale vrátila chybu - není dostatek paměti
        printf("Chyba alokace pameti pro S_Jidlo!"), getchar(); //#debug
        //zkusí, jestli zkrátka není druhů specielních jídel příliš mnoho
        povolenoSpecialniJidlo = DRUHU_SPEC_JIDEL;
        if(alokovatPametProSpecJidlo()) return -1; //nepovedlo se
    }
    for(t=abs(puvodniPovolenoSpecialniJidlo);t<abs(povolenoSpecialniJidlo);t++){ //povedlo se alokovat paměť - zbývá jen inicializovat hodnoty
        ZN_JIDLA_S[t] = '$'+t; //za '$' je nekolik pro specielni jidlo docela hezkych znaku
        trvaniJidlaS[t] = 8000;
        hodnotyJidlaS[t] = 2;
    }

    puvodniPovolenoSpecialniJidlo = povolenoSpecialniJidlo;
    return 0;
}

inicializovatHada(){ //nastaví Hada kompletně na výchozíá hodnoty
    povolenoSpecialniJidlo = DRUHU_SPEC_JIDEL;
    puvodniPovolenoSpecialniJidlo = 0;
    ZN_JIDLA = DEFAULT_ZN_JIDLA;    ZN_HLAVA = DEFAULT_ZN__HLAVA;   ZN_TELO = DEFAULT_ZN__TELO;
    HAD_CHODI_SIKMO = DEFAULT_HAD_CHODI_SIKMO;

    return inicializovatDefaultSpecielniJidlo(1);
}
/*!had!*/


HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE); //aby to funkce pro přesun kurzoru na dané souřadnice nemusela volat při každém svém běhu

/*nejdůležitější funkce celého programu - umožňuje přeskočit kurzorem na vybrané souřadnice*/
void gotoxy (int x, int y){
    COORD coord; // coordinates
    coord.X = x; coord.Y = y; // X and Y coordinates
    SetConsoleCursorPosition(hConsole, coord);
}

#define zacatekKresleniCervene() (SetConsoleTextAttribute(hConsole, 4))
#define zacatekKresleniCerneNaBilem() (SetConsoleTextAttribute(hConsole, 240))
#define konecKresleniCervene() (SetConsoleTextAttribute(hConsole, 7))
/*#define zacatekKresleniCervene() (SetConsoleTextAttribute(hConsole, 244))  //
#define zacatekKresleniCerneNaBilem() (SetConsoleTextAttribute(hConsole, 7))
#define konecKresleniCervene() (SetConsoleTextAttribute(hConsole, 240))*/

char malePismeno(char c){   //převede VELKÉ písmeno na malé
    return (c>='A' && c<='Z')?c+('a'-'A'):c;
}

char velkePismeno(char c){   //převede malé písmeno na VELKÉ
    return (c>='a' && c<='z')?c+('A'-'a'):c;
}

najitZnak(char str[], char c){//vrati nejvyssi poradi zadaneho znaku v retezci nebo -1 kdyz tam znak vubec neni. Skenuje i ukoncovyci symbol '\0'
    int t;
    for(t = strlen(str);t>=0;t--) if(str[t]== c) return t;
    return -1;
}

prelozitZadanyZnak(char c){ //převede znak z pole uživatelem konfigurovaných kláves pro ovládání na programu srozumitelný znak z pole výchozích znaků
    int t;
    if((t=najitZnak(nastaveneTlOvladani,c))==-1){ return 0;}
    else {return puvTlOvladani[t];}
}

vybilit(int x,int y, int delka){ //zaplní daný řádek mezerami -> smaže text který tam byl
    if(y==Y_OTAZEK){
        priNejblizsiPrilezitostiVybilitSpodniRadku = 0; //Dotazníková řádka "již byla vybílena" - není třeba to hned při příští příležitosti dělat znovu
    }
    gotoxy(x,y);    //skočí na počadované místo a...
    for(;delka>0;delka--) putchar(' '); //... a vypíše požadovaný počet mezer
    gotoxy(x,y);    //vrátí kurzor na začátek místa, které právě vybílila

}

//K čemu to je? - viz UM - 3.2
int anoNe(int x, int y, char text[], int odpoved){
    char ch;
    int delkaTextu = strlen(text); //aby se nemuselo počítat pokaždé znovu
    vybilit(x,y,POCET_K_VYBILENI);
    //gotoxy(x,y); //není potřeba - to již provedla funkce vybilit
    printf("%s %s ",text, odpoved?"[ANO/ne]":"[ano/NE]");
    gotoxy(x+delkaTextu+(odpoved?2:6),y); //kurzor skočí buď na ANO, nebo na NE - podle toho, které je volajícím vyžádáno jako výchozí vybrané
    while(1){
        if(kbhit()){ //pokud byl stisknut znak na klávesnici...
            switch(ch = prelozitZadanyZnak(malePismeno(getch()))){ //... přečte ho to a převede na srozumitelnou hodnotu
                case('a'): //pohyb doleva ve výběru
                    if(odpoved) break;
                    odpoved = 1;
                    gotoxy(x+delkaTextu,y);
                    printf(" [ANO/ne]");
                    gotoxy(x+delkaTextu+(odpoved?2:6),y);
                    break;
                case('d'): //pohyb doprava
                    if(!odpoved) break;
                    odpoved = 0;
                    gotoxy(x+delkaTextu,y);
                    printf(" [ano/NE]");
                    gotoxy(x+delkaTextu+(odpoved?2:6),y);
                    break;
                case(' '): //výběr potvrzen
                    vybilit(x,y, delkaTextu+50);
                    return odpoved;
            }
        }
    }

}

/*vybílí celou obrazovku a potom vypíše na dané souřadnice zprávu; případně vyčká na stisknutí enteru*/
void vypsatClsZpravu(int x, int y, char *str, int pressEnter){
    system("cls"); //příkaz pro Příkazovou řádku, aby smazala vše, co je na obrazovce
    gotoxy(x,y);
    puts(str);
    pressEnter&&getchar(); //drobné zneužití zkráceného vyhodnocování logických výrazů, aby se nemusel rozepisovat s ifem
}

/*vypíše zprávu do Dialogové řádky*/
zprava(char *str){
    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
    gotoxy(0, Y_OTAZEK);
    puts(str);
}

/*nastaví ovládání na výchozí hodnoty*/
#define inicializovatPoleNovychZnaku()  (strcpy(nastaveneTlOvladani, puvTlOvladani))


/*uloží příslušné proměnné do konfiguračního souboru*/
ulozitConfig(){
    FILE *f;
    int t;

    inicializovatDefaultSpecielniJidlo(1);

    if((f=fopen(CONFIG_FILE_NAME,"w"))==NULL) return 1;
    fputs(nastaveneTlOvladani, f);
    fprintf(f, "\n%d", pouzivaWin10);
    fprintf(f, "\n%d", hadiRekord);
    fprintf(f, "\n%d", HAD_CHODI_SIKMO);
    fprintf(f, "\n%c%c%c\n%d\n%d", ZN_JIDLA, ZN_HLAVA, ZN_TELO, povolenoSpecialniJidlo, pkostJidlaS);

    for(t=0;t<abs(povolenoSpecialniJidlo);t++){
        fprintf(f, "\n%c %d %d", ZN_JIDLA_S[t], trvaniJidlaS[t], hodnotyJidlaS[t]);
    }

    fclose(f);
    return 0;
}

/*načte příslušné proměnné z konfiguračního souboru*/
nacteniConfigu(){
    FILE *f;
    char c;
    int t, pl;

    #define CHYBA(i) fclose(f); return (i) //pokud došlo k chybě (rozbitý konfigurační soubor), zavře konfigurační soubor a navrátí danou hodnotu; Pozor! - jsou to 2 příkazy a tudíž se vždy musí uzavřít do složených závorek

    if((f=fopen(CONFIG_FILE_NAME,"r"))==NULL) return 1;

    fgets(nastaveneTlOvladani, strlen(nastaveneTlOvladani)+1,f);
    if(strlen(nastaveneTlOvladani)!=strlen(puvTlOvladani)) {CHYBA (-1);}
    if(fscanf(f,"%d", &pouzivaWin10)<1) {CHYBA(-2);}
    if(fscanf(f,"%d", &hadiRekord) <1) {CHYBA(-3);}
    if(fscanf(f,"%d", &HAD_CHODI_SIKMO) <1 || abs(HAD_CHODI_SIKMO)>1) {CHYBA(-4);}

    if(fscanf(f,"\n%c%c%c", &ZN_JIDLA, &ZN_HLAVA, &ZN_TELO) <3) {CHYBA(1);}
    if((fscanf(f,"\n%d", &povolenoSpecialniJidlo) <1)||alokovatPametProSpecJidlo()) {CHYBA(4);} //pokud se pro specielní jídlo nepodařilo alokovat dost paměti, také se to bere za chybu

    if(fscanf(f,"\n%d", &pkostJidlaS) <1) {CHYBA(5);}

    for(t=0;t<abs(povolenoSpecialniJidlo);t++){
        if((fscanf(f,"\n%c%d%d", ZN_JIDLA_S+t, trvaniJidlaS+t, hodnotyJidlaS+t) <1) || (((unsigned char)ZN_JIDLA_S[t])<' ')) {CHYBA(6+t);} //pokud byl pro Specielní jídlo přečten bílý znak, také se to považuje za chybu
    }
    puvodniPovolenoSpecialniJidlo = povolenoSpecialniJidlo;

    fclose(f);
    return 0;
}

/*funkce, která se volá při spuštění programu, a má za úkol zorganizovat načtení konfiguračního souboru a informovat uživatele o případných chybách, ke kterým došlo*/
nacistConfig(){
    int t;
    t = nacteniConfigu();
    if(t){
        if(t>0) inicializovatHada();
        if(t==-1) inicializovatPoleNovychZnaku();
        if(t==-2) (pouzivaWin10 = anoNe(0, 0, "Pouzivate Windows 10 nebo jiny OS, ktery umi pracovat s barevnymi znaky?", 0)), system("cls");

        vypsatClsZpravu(0,0,"Veskere nekonzistentnosti v konfiguracnim souboru byly napraveny. \n***PRESS ENTER***", 1);
        printf("Config load error: %d", t), getchar();  /*#debug*/
        inicializovatPoleNovychZnaku();
        ulozitConfig();
    }

    return 0;
}


/*přeloží znak z pole výchozích znaků na znak, který je skutečně třeba zadat*/
prelozitNaZnakKZadani(char c){
    int t;
    if((t=najitZnak(puvTlOvladani,c))==-1||nastaveneTlOvladani[t]<32) return 1;
    return nastaveneTlOvladani[t];
}


int vrn(char str[],char text[]){ //vypsat radek v Napovede
    #define NAZEV_PRO_MEZERNIK  "Mezernik"
    int t, u,pom,delka=strlen(text);
    //text...řetězec, který se vypisuje; když narazí na znak '_', nahradí ho opravdovým znakem k příslušnému výchozímu znaku ze str[] (čte se po pořadí, podobně jako argumenty u printf)

    for(t=0,u=0, pom=0;t<delka;t++){
        if(text[t]=='_'){
            if(velkePismeno(prelozitNaZnakKZadani(str[u]))==' '){printf(NAZEV_PRO_MEZERNIK);u++;pom+=strlen(NAZEV_PRO_MEZERNIK);continue;}
            //if(velkePismeno(prelozitNaZnakKZadani(str[u]))=='\e'){printf("Escape");u++;continue;} //!nefunguje - zjistit proc
            //if(velkePismeno(prelozitNaZnakKZadani(str[u]))=='\n'){printf("Enter");u++;continue;}
            putchar(velkePismeno(prelozitNaZnakKZadani(str[u])));
            u++;
            continue;
        }
        putchar(text[t]);
    }
    return t+pom-1; //vrací počet znaků, ktré vypsala
}

/*přiřadí každému prvku pole výchozí hodnotu ' ' - inicializace prázdného plátna*/
znGen(){
    int x, y;
    for(x = 0;x<=X_MAX;x++){
        for(y=0;y<=Y_MAX;y++)
            zn[x][y] = ' ';
    }
}

/*zjistí, zda řetězec sestává pouze z číslic*/
int strObsahujeJenCislice(char str[]){
    int t, delka = strlen(str);
    for(t=0;t<delka;t++){
        if(str[t]>'9'||str[t]<'0') return 0;
        if(str[t]=='\0') if(t==0)return 0; else return 1;
    }
}

/*přiměje Příkazový řádek, aby příslušně upravil velikost svého okna*/
prizpusobitVelikostOknaRozliseni(int x, int y){
    char str[30];
    snprintf(str, 30, "mode con:cols=%d lines=%d", x, y);
    system(str); //zavolá příkaz v Příkazovém řádku
}

/*vykreslí rámeček okolo daného obdélníku - používá se pro nakreslení rámečku okolo plátna nebo okolo oblasti určené k Ořezu*/
vykreslitOhraniceni(int x0, int y0, int xMax, int yMax){
    vyznacenyVyznBody = 0;
    int x = --x0, y = --y0;
    gotoxy(x,y);
    putchar(201);
    for(x = x0;x<xMax;x++) putchar(205);
    putchar(187);
    for(y=y0+1;y<=yMax;y++){
        gotoxy(x0,y);
        putchar(186);
        gotoxy(xMax+1,y);
        putchar(186);
    }
    x=x0;
    gotoxy(x,y);
    putchar(200);
    for(x=x0;x<xMax;x++) putchar(205);
    putchar(188);
}

/*vykreslí zářezy na rámečku ohraničujícím plátno, případně je zamaže*/
vyznacitVyznMiry(){
    vyznacenyVyznBody = 1;
    if(X_MAX%2==0){
        gotoxy(X_MAX/2,0);
        putchar(203);putchar(203);
        gotoxy(X_MAX/2,Y_MAX+1);
        putchar(202);putchar(202);
    }
    else{
        gotoxy(X_MAX/2+1,0);
        putchar(203);
        gotoxy(X_MAX/2+1,Y_MAX+1);
        putchar(202);
    }
    gotoxy(0,Y_MAX/2+1);
    putchar(204);
    gotoxy(X_MAX+1,Y_MAX/2+1);
    putchar(185);
    if(Y_MAX%2==0){
        gotoxy(0,Y_MAX/2);
        putchar(204);
        gotoxy(X_MAX+1,Y_MAX/2);
        putchar(185);
    }
}

/*nakreslí čtverec - viz UM - 2.8*/
fillSquare(int zx, int zy,int polomer, char znak, int duty){
    int hx = ((zx-polomer)<1)?1:(zx-polomer),hy = ((zy-polomer)<1)?1:(zy-polomer),dx = ((zx+polomer)>X_MAX)?X_MAX:(zx+polomer),dy = ((zy+polomer)>Y_MAX)?Y_MAX:(zy+polomer),x,y;
    for(x=hx, y = hy;y<=dy;){
        if((!duty)||(x==hx||x==dx||y==hy||y==dy)){
            zn[x][y] = znak;
            gotoxy(x,y);
            putchar(znak);
        }
        x++;
        if(x>dx) {
            x = hx;
            y++;
        }
    }
    gotoxy(zx,zy);
}

/*posune kurzor daným směrem - očekává se, že x a y nabývají hodnot od -1 do 1*/
posunKurzoru(int x, int y){
    xn += x*rychlostPohybu;
    yn += y*rychlostPohybu;
    //pohyb přes okraj plátna
    while(yn < 1)     yn += (cyklPohyb)?Y_MAX: -y*rychlostPohybu;
    while(xn < 1)     xn += (cyklPohyb)?X_MAX: -x*rychlostPohybu;
    while(yn > Y_MAX) yn += (cyklPohyb)?(-Y_MAX): -y*rychlostPohybu;
    while(xn > X_MAX) xn += (cyklPohyb)?(-X_MAX): -x*rychlostPohybu;
}

/*ošetří , že kurzor není mimo plátno*/
proPresahHranice(){
    while(yn < 1)     yn = (cyklPohyb)?yn+Y_MAX: 1;
    while(xn < 1)     xn += (cyklPohyb)?xn+X_MAX: 1;
    while(yn > Y_MAX) yn += (cyklPohyb)?1: Y_MAX;
    while(xn > X_MAX) xn += (cyklPohyb)?1: X_MAX;

}

/*uklidí na místě, které kurzor právě opustil*/
smazatKurzor(int x, int y){
    if(editRezim>0)
        zn[x][y] = kresliciZnak;
    else if(editRezim<0)
        zn[x][y] = ' ';
    putchar(zn[x][y]);
}


/*vypíše Nápovědu za okrajem plátna a vrátí kurzor na dané souřadnice*/
vypsatNapovedu(int puvodniX, int puvodniY, int dolni/*boolean*/, int pravou/*boolean*/){
    char ohledneOblVZasobniku[30];
    if(dolni){ //aby se nemusely vždy zbytečně vypisovat obě Nápovědy
        if(vybrObl==NULL)sprintf(ohledneOblVZasobniku, "Zasobnik...#####");
        else sprintf(ohledneOblVZasobniku, "Zasobnik...%d x %d", oblX, oblY);
        gotoxy(0,Y_MAX+2);
        /*asi nejdelsi jednoradkovy prikaz co jsem kdy napsal :-)*/printf("Aktualni znak: '%c' (dec:%3d) (hexadec:%3X)                           \n\nRezim: %s    Pohyb: %s        Ctverce: %s    Rychlost pohybu: %2d    \nDelka rychle cary: %3d    Polomer: %3d    Natoceni: %s                \n%s\t\t\t\t\t\nRozliseni: %3d x %3d \n", kresliciZnak,(unsigned char)kresliciZnak,(unsigned char)kresliciZnak, (!editRezim)?"pohyb  ":(editRezim>0)?"editace":"guma   ", (cyklPohyb)?"cyklicky  ":"ohraniceny", (duteCtverce)?"dute":"plne", rychlostPohybu,delkaRychleCary,polomerRychlehoCtverce,!natoceni?"Vychod      ":(natoceni==1?"JihoVychod  ":(natoceni==2?"Jih         ":(natoceni==3?"JihoZapad   ":(natoceni==4?"Zapad       ":(natoceni==5?"SeveroZapad ":(natoceni==6?"Sever       ":"SeveroVychod")))))), ohledneOblVZasobniku, X_MAX,Y_MAX);
        //gotoxy(0, Y_MAX+3);        printf("DEBUG: r4 = %d", (povolenoSpecialniJidlo>3)?trvaniJidlaS[3]:-1);
    }
    if(pravou){
        int odsazeni = X_MAX+3,radek = 0;
        gotoxy(odsazeni,radek++);
        printf("OVLADANI:           ");
        gotoxy(odsazeni,radek++);
        vrn("wsad","_,_,_,_...pohyb rovne           ");
        gotoxy(odsazeni,radek++);
        vrn("qeyc","_,_,_,_...pohyb sikmo           ");
        gotoxy(odsazeni,radek++);
        vrn("*","_...vypnout program           ");
        gotoxy(odsazeni,radek++);
        vrn("123","_,_,_...prepinani rezimu kresleni           ");
        gotoxy(odsazeni,radek++);
        vrn("p","_...prepinani rezimu pohybu           ");
        gotoxy(odsazeni,radek++);
        vrn(",","_...prepinani dutosti ctvercu           ");
        gotoxy(odsazeni,radek++);
        vrn("0","_...zmena natoceni Textu a car           ");
        gotoxy(odsazeni,radek++);
        vrn("4","_...rychle vytvori ctverec           ");
        gotoxy(odsazeni,radek++);
        vrn("6","_...rychle vytvori dve rovnobezne cary s predvolenym natocenim           ");
        gotoxy(odsazeni,radek++);
        vrn("8","_...vytvoreni ctverce s vybranym polomerem           ");
        gotoxy(odsazeni,radek++);
        vrn("9","_...vytvoreni rovnobeznych car s vybranou delkou a predvolenym natocenim           ");
        gotoxy(odsazeni,radek++);
        vrn("jk","_,_...zmena polomeru rychleho ctverce a car           ");
        gotoxy(odsazeni,radek++);
        vrn("mn","_,_...zmena delky rychle cary           ");
        gotoxy(odsazeni,radek++);
        vrn("t","_...vlozit textovy retezec           ");
        gotoxy(odsazeni,radek++);
        vrn("+-/","_,_,_...vyber kresliciho znaku           ");
        gotoxy(odsazeni,radek++);
        vrn("z","_...(od)vyznacit stredy stran           ");
        gotoxy(odsazeni,radek++);
        vrn(";","_...prekreslit obrazovku (pouzivat jen kdyz je nezbytne nutne!)           ");
        gotoxy(odsazeni,radek++);
        vrn(" ","_...potvrdit vyber nebo umisteni Textu, promazat Dialogovou radku           ");
        gotoxy(odsazeni,radek++);
        printf("Enter...potvrdit pri zadavani textu %s",(pravou==2)?"(tuto klavesu nelze zmenit)":"                           ");
        gotoxy(odsazeni,radek++);
        vrn("7","_...smazat vse ( + zmenit rozliseni)           ");
        gotoxy(odsazeni,radek++);
        vrn("u","_...ulozit do souboru           ");
        gotoxy(odsazeni,radek++);
        vrn("@","_...rychle ulozit           ");
        gotoxy(odsazeni,radek++);
        vrn("l","_...nacist ze souboru           ");
        gotoxy(odsazeni,radek++);
        vrn("5","_...zmenit ovladani           ");
        gotoxy(odsazeni,radek++);
        vrn("h","_...vratit vychozi ovladani           ");
        gotoxy(odsazeni,radek++);
        vrn(".","_...zmenit znak kresleni na znak, na kterem se prave naleza kurzor           ");
        gotoxy(odsazeni,radek++);
        vrn("v","_...kopirovat           ");
        gotoxy(odsazeni,radek++);
        vrn("o","_...orez           ");
        gotoxy(odsazeni,radek++);
        vrn("=)","_,_...zvysit/snizit rychlost pohybu           ");
        gotoxy(odsazeni,radek++);
        vrn("|", "_...Nastaveni           ");
        gotoxy(odsazeni,radek++);
        vrn("\\", "_...HAD!           ");


    }
    gotoxy(puvodniX, puvodniY);
}

/*viz UM - 2.5*/
prohoditKlavesy(int x, int y /*souřadnice, kam píše zprávy*/){
    char c[2];
    int t, cp;
    vypsatNapovedu(x,y,0,2); //vypíše Klávesovou nápovědu, navíc s tím, že u Enteru bude upozornění, že ten nelze vyměnit za jinou klávesu
    //gotoxy(x,y);
    printf("Stisknete klavesu, jejiz funkci chcete priradit necemu jinemu!");
    while(1){
        if(kbhit()){
            c[0]=malePismeno(getch());
            if((t=najitZnak(nastaveneTlOvladani,c[0]))!=-1) break; //je spokojené jen když přečte klávesu, která již má nějakou funkci a ta může tudíž být převedena na nějaký jiný znak
        }
    }
    c[1]='\0';
    vybilit(x,y,POCET_K_VYBILENI);
    printf("Nyni stisknete klavesu, ktere chcete priradit funkci klavesy %c!", c[0]);
    while(1){
        if(kbhit()){
            c[1]=malePismeno(getch()); //stačí jakýkoliv znak - i ten původní, pokud si to uživatel mezitím stihl rozmyslet
            break;
        }
    }
    vybilit(x,y,POCET_K_VYBILENI);
    if((cp=najitZnak(nastaveneTlOvladani,c[1]))==-1){ //pokud se vybraný znak dosud nepoužívá k ničemu jinému, pouze se mu přiřadí nová funkce
        nastaveneTlOvladani[t]=c[1];
    }
    else {  //pokud vybraný znak již má nějakou funkci, ta se přiřadí znaku, jehož funkci právě převzal
        nastaveneTlOvladani[cp]=c[0];
        nastaveneTlOvladani[t]=c[1];
    }

    ulozitConfig();
    vypsatNapovedu(xn,yn,0,1);
}


/*!Funkce pro práci se soubory*/

/*alokuje paměť pro plátno o určitých rozměrech*/
realokovatProRozliseni(int x, int y, int puvX){

    if(x<=0 || y<=0) return -1; //žádost nedává smysl

    if(x>puvX){ //přidáváme k plátnu další sloupce
        zn = ( char **) realloc(zn, x*sizeof( char *));
        for(x--;x>=0;x--) zn[x] = ( char*) ((x>puvX)?malloc(y):realloc(zn[x], y));
    }
    else{ //sloupce z plátna ubíráme
        for(puvX--;puvX>=0;puvX--) (((puvX>x)&&(free(zn[puvX]), 1)) || (zn[puvX] = ( char*)realloc(zn[puvX], y)));
        zn = ( char **) realloc(zn, x*sizeof( char *));
    }

    return 0;
}

/*Má na starosti kompletní organizaci a komunikaci s uživatelem při změně rozlišení plátna*/
zmenitRozliseni(int sx, int sy/*souřadnice, kam vypisuje zprávy*/, int sVysvetlivkami/*boolean*/){
    int xy[2], vybranyPredmet = 0;
    char c;
    char otazka[MAX_STRING];
    xy[0] = (X_MAX>=MIN_XY[0])?X_MAX:DEF_X_MAX;
    xy[1] = (Y_MAX>=MIN_XY[1])?Y_MAX:DEF_Y_MAX;
    vybilit(sx,sy,POCET_K_VYBILENI);
    //gotoxy(sx,sy);
    printf("Vyberte rozliseni obrazku... Sirka: %3d    Vyska: %3d ", xy[0],xy[1]);

    if(sVysvetlivkami){gotoxy(sx,sy+1); vrn("wsad ","(Vyber pomoci _,_,_,_, pro potvrzeni stisknete _)");}

    if(vybranyPredmet) gotoxy(sx+strlen("Vyberte rozliseni obrazku... Sirka: 100    Vyska: 1"),sy); //kam skočí kurzor
    else    gotoxy(sx+strlen("Vyberte rozliseni obrazku... Sirka: 1"),sy);

    while(1){
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){
            case('w'):
            case('+'):
                if(xy[vybranyPredmet]<MAX_XY)xy[vybranyPredmet]++;
                break;
            case('s'):
            case('-'):
                if(xy[vybranyPredmet]>MIN_XY[vybranyPredmet])xy[vybranyPredmet]--;
                break;
            case('a'):
                vybranyPredmet = 0;
                break;
            case('d'):
                vybranyPredmet = 1;
                break;
            case(' '):
                if(sVysvetlivkami) vybilit(sx,sy+1,POCET_K_VYBILENI);
                snprintf(otazka, MAX_STRING, "Opravdu chcete vytvorit soubor s rozlisenim %d x %d", xy[0],xy[1]);
                if(anoNe(sx,sy,otazka,1)){
                    if(realokovatProRozliseni(xy[0]+1, xy[1]+1, X_MAX)) printf("!!!"), getchar();
                    X_MAX = xy[0];  Y_MAX = xy[1]; Y_OTAZEK = Y_MAX+ODSAZENI_Y_OTAZEK;
                    if(xn>X_MAX)xn=X_MAX;   if(yn>Y_MAX)yn=Y_MAX;
                    prizpusobitVelikostOknaRozliseni(X_MAX+90,Y_MAX+30);
                    return 0;
                }
                else c = 1;
                break;
            }
            gotoxy(sx,sy);
            printf("Vyberte rozliseni obrazku... Sirka: %3d    Vyska: %3d ", xy[0],xy[1]);
            if(vybranyPredmet) gotoxy(sx+strlen("Vyberte rozliseni obrazku... Sirka: 100    Vyska: 1"),sy);
            else    gotoxy(sx+strlen("Vyberte rozliseni obrazku... Sirka: 1"),sy);
        }
    }
}

/*smaže celé plátno*/
smazat(int rovnou/*boolean - neptá se uživatele a maže hned*/){
    if(rovnou || anoNe(0,Y_OTAZEK,"Opravdu si prejete vse smazat?", 0)){
        if((!rovnou) && anoNe(0,Y_OTAZEK,"Nechcete pro novy obrazec zmenit rozliseni ?", 0)){
            zmenitRozliseni(0,Y_OTAZEK,0);
        }

        system("cls"); //vyčistí obrazovku
        znGen(); // inicializuje celé plátno mezerami
        vykreslitOhraniceni(1,1,X_MAX, Y_MAX);
        vypsatNapovedu(xn,yn,1,1);
        if(rovnou){xn=1;yn=1;}
        gotoxy(xn,yn);
        putchar(kresliciZnak);
        gotoxy(xn,yn);
    }
}

/*vyčistí celou obrazovku a vše vykreslí odznova*/
prekreslit(int cele){
    int x,y;
    if(cele){
        system("cls");
        vykreslitOhraniceni(1,1,X_MAX, Y_MAX);
        vypsatNapovedu(xn,yn,1,1);
    }
    for(y = 1;y<=Y_MAX;y++){
        for(x = 1;x<=X_MAX;x++){
            gotoxy(x,y);
            putchar(zn[x][y]);
        }
    }

    //gotoxy(xn,yn);  putchar(kresliciZnak);  gotoxy(xn,yn);
}


/*samotný proces přepisování obsahu plátna do souboru*/
otisknoutPlatnoDoSouboru(FILE *f){
    int x,y;
    for(y=1;y<=Y_MAX;y++){
        for(x=1;x<=X_MAX;x++){
            putc(zn[x][y], f);
        }
        putc('\n',f);
    }

}

/*má za úkol zorganizovat veškeré náležitosti při ukládání plátna do souboru*/
ulozitDoSouboru(int x, int y/*souř. na kterých probíhá dialog s uživatelem*/){
    char nazev[MAX_STRING];
    char zprava[MAX_STRING+50] = "Soubor ";
    /*char zacatekZpravy[8] = "Soubor "; char konecZpravy[39] = " jiz existuje. Prejete si jej prepsat?";*/
    FILE *f;
    vybilit(x,y, POCET_K_VYBILENI);
    gotoxy(x,y);
    printf("Zadejte nazev souboru: ");
    if(scanf("%s", &nazev)<1) return 1;//nepodařilo se přečíst žádný název
    while(getchar()!='\n');
    if(strchr(nazev, '.')==NULL) strcat(nazev, ".txt"); //když chybí koncovka, doplní se ".txt"

    if((f=fopen(nazev, "r"))!=NULL){ //soubor se podařilo otevřít pro čtení ~ soubor již existuje
        strcat(zprava, nazev);
        strcat(zprava, " jiz existuje. Prejete si jej prepsat?");
        if(!anoNe(x,y,zprava,0)){
            fclose(f);
            return 1;}
    }

    fclose(f);

    if((f = fopen(nazev, "w"))==NULL){
        //soubor se nepodařilo otevřít
        vybilit(x,y, POCET_K_VYBILENI/2*3);
        gotoxy(x,y);
        printf("Ukladani do souboru %s nebylo uspesne!", nazev);
        return 1;
    }

    otisknoutPlatnoDoSouboru(f);

    fclose(f);
    vybilit(0,Y_OTAZEK, POCET_K_VYBILENI/2*3);
    gotoxy(0,Y_OTAZEK);
    printf("Uspesne ulozeno do souboru %s!", nazev);
    nastavitJmenoAktualnihoSouboru(nazev); //pro funkci Rychlého uložení
    return 0;
}


int rychleUlozitDoSouboru(char *nazev){
    FILE *f;
    if(!strcmp(nazev, "")) return 1;
    if((f=(fopen(nazev, "w")))==NULL) return -1;
    otisknoutPlatnoDoSouboru(f);
    fclose(f);
    return 0;

}
/*má nastarosti kompletní organizaci rychlého uložení*/
int celaProceduraRychlehoUlozeni(int x, int y/*souř., kde probíhá dialog s uživatelem*/){
    int t = rychleUlozitDoSouboru(jmenoSouboru);

    if(t) return t; //když došlo k chybě, nevypíše se zpráva o úspěchu
    vybilit(x,y, POCET_K_VYBILENI);
    gotoxy(x,y);
    printf("Rychle ulozeni probehlo uspesne");
    priNejblizsiPrilezitostiVybilitSpodniRadku = 1; //aby zpráva o úspěchu zbytečně neotravovala
    return t;
}


/*přečte soubor a nastaví rozlišení tak, aby se do plátna soubor dal načíst pokud možno bez ořezu*/
nacistRozliseniZeSouboru(FILE *f){
    int x=0,y=0, maxX=1, maxY=1, ret = 1;
    char c;

    while(1){
        c=getc(f);
        if(c=='\n'){
            if(maxX<x) maxX = x; //najdeme nejdelší řádku
            x=0;
            y++;
            continue;
        }
        if(c==EOF){
            if(maxY<y) maxY=y;//počet řádek
            break;
        }
        x++;
    }
    rewind(f);//přetočíme soubor zase na začátek - zavře ho až volající funkce

    if(maxX > MAX_XY) (maxX = MAX_XY), ret-=2; //na souřadnici X došlo k ořezu
    else if(maxX<MIN_XY[0]) maxX = MIN_XY[0]; //natažení, aby byly dodrženy limity na min. šířku
    if(maxY > MAX_XY) (maxY = MAX_XY), ret-=3;  //na souřadnici Y došlo k ořezu
    else if(maxY<MIN_XY[1]) maxY = MIN_XY[1];//natažení, aby byly dodrženy limity na min. výšku
    realokovatProRozliseni(maxX+1, maxY+1, X_MAX); //1 znak se přidává navíc, aby se nemuselo indexovat přes nulu a všude přičítat jednička

    X_MAX = maxX;
    Y_MAX = maxY;
    Y_OTAZEK = Y_MAX+ODSAZENI_Y_OTAZEK;
    if(xn>X_MAX)xn=X_MAX;   if(yn>Y_MAX)yn=Y_MAX;
    prizpusobitVelikostOknaRozliseni(X_MAX+90,Y_MAX+20);

    return ret;
}

int nacistSoubor(FILE *f, int vynutitNacteniRozliseni, int px, int py){
    int x,y, ret;
    char c = 0;
    if(vynutitNacteniRozliseni || anoNe(px, py, "Prejete si prizpusobit rozliseni nacitanemu souboru?", 1)) ret = nacistRozliseniZeSouboru(f);
    else    zmenitRozliseni(px, py, 0);

    for(x=1,y=1;1;){
        if(x>X_MAX) {x=1;y++;   while(c!='\n'&&c!=EOF) c=getc(f);} //pokud je na řádce více znaků, než se vejde na plátno, zbytek řádky se přečte a zahodí
        if(y>Y_MAX) {
                if(c=='\n'&&(getc(f)!=EOF)) ret = -1; //pokud přečuhuje nějaká řádka, není třeba číst dál, ale rovnou se navrátí hodnota značící, že došlo k chybě
                break;
        }
        c=getc(f);
        if(c=='\n'&&x<=X_MAX){ //pokud řádka končí dřív, než je konec plátna, zbytek řádky se doplní mezerami
            while(x<=X_MAX)zn[x++][y]=' ';
            continue;
        }
        if(c==EOF && y<Y_MAX){  //pokud je načítaný soubor kratší, než plátno, celý zbytek plátna se doplní mezerami
            while(y<=Y_MAX){
                zn[x][y] = ' ';
                x++;
                if(x>X_MAX){x=1;y++;}
            }
            continue;
        }
        if(x>X_MAX && c!='\n'){
            while((c=getc(f))!='\n')
            ret = -1;
            continue;
        }
        zn[x][y] = c;   //pokud nedošlo k žádné z výjímek, přečtený znak se uloží na své místo a x se zinkrementuje (k tomu ale nemusí dojít při některé z výjímek - proto x++ není v hlavičce fóru, ale až zde)
        x++;
    }
    return ret;
}

/*má na starosti kompletní organizaci načítání textového souboru do programu*/
int celaProceduraProNacteniSouboru(int px,int py, int pocetOpakovani/*kolikkrát nanejvýš si vyžádá zadání názvu souboru, než to vzdá*/, int vynutitNacteniRozliseni){
    int x,y;

    #define PRECIST_NAZEV_SOUBORU (scanf("%s", nazev),  ((najitZnak(nazev, '.')==-1) && strcpy(&nazev[strlen(nazev)], ".txt")))  //přečte název souboru; pokud nemá koncovku, doplní ".txt
    /*zneužívá zkráceného vyhodnocování logických výrazů a operátoru čárky, aby nemuselo obsahovat if a vešlo se tudíž do jednoho příkazu*/

    char nazev[MAX_STRING];
    char zprava[MAX_STRING+50];
    FILE *f;
    vybilit(px,py,POCET_K_VYBILENI);
    printf("Zadejte nazev souboru: ");
    PRECIST_NAZEV_SOUBORU;
    while(getchar()!='\n');
    for(x=0;x<=pocetOpakovani;x++){
        if((f = fopen(nazev, "r"))!=NULL)
            break;
        else{
            if(x>=pocetOpakovani){  fclose(f); return 0;}
            else{
                fclose(f);
                vybilit(px,py,POCET_K_VYBILENI);
                printf("Chyba! Zadejte jiny nazev souboru: ");
               PRECIST_NAZEV_SOUBORU;
                while(getchar()!='\n');
            }
        }
    }
    nastavitJmenoAktualnihoSouboru(nazev); //pro Rychle ulozeni
    return nacistSoubor(f, vynutitNacteniRozliseni, px, py);
}
/*!Funkce pro práci se soubory - konec*/



/*!Funkce pro výběr a umísťování oblastí plátna a textových řetězců*/


typedef enum {NOVY, ZAMAZAT, PUVODNI} moznVykresleniZnaku; //značí režim, jak se znak bude vykreslovat - NOVY...vykreslí znak, který funkce dostala k vykreslení; ZAMAZAT...vykreslí mezeru; PUVODNI...vykreslí znak, který se nalézá na daných souřadnicích na plátně

/*vykreslí na daných souřadnicích daný znak*/
vykreslitZnakZTextu(int x, int y, char c, int prepisovat/*bool; zda se má znak obtisknout do plátna*/, moznVykresleniZnaku rezim, int vykreslit /*bool; zda má být znak skutečně vykreslen na obrazovku, či se jen interně zpracuje v paměti*/){

    if(duteCtverce&& ((unsigned char)c<=32)) return 2; //když jsou aktivní Duté čtverce a vykreslen má být bílý znak, přeskočí ho to - viz UM - 5.0

    if(cyklPohyb){  //ošetření přesahu přes okraj plátna
        while(x>X_MAX) x-=X_MAX;
        while(x<1) x+=X_MAX;
        while(y>Y_MAX) y-=Y_MAX;
        while(y<1) y+=Y_MAX;
    }
    if(x<=X_MAX&&y<=Y_MAX&&x>0&&y>0){ //pokud není Režim cyklického pohybu a znak přesahuje, bude ignorován
        if(rezim!= PUVODNI){
            if(rezim==ZAMAZAT) c=' ';
            if(prepisovat) zn[x][y] = c;
        } else        c = (zn[x][y]);

        if(vykreslit) gotoxy(x,y),  putchar(c);

    }
}


/*vykreslí daný textový řetězec z dané souřadnice s přísl. natočením*/
vykreslitText(int x, int y, int delka, char text[], /*násl. argumenty akorát předává funkci pro vykreslení znaku*/int prepisovat, moznVykresleniZnaku rezim, int vykreslit){
    int t;
    //gotoxy(x,y);
    switch(natoceni){
        case(0)://normální natočení
            for(t=0;t<delka;t++)    {vykreslitZnakZTextu(x+t,y, text[t],prepisovat, rezim, vykreslit);}//časem bych mohl upravit, aby v případě, že nejsou Duté čtverce,použilo jednoduše  puts()
            break;
        case(1)://napravo dolů
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x+t,y+t, text[t],prepisovat, rezim, vykreslit);
            break;
        case(2)://seshora dolů
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x,y+t, text[t],prepisovat, rezim, vykreslit);
            break;
        case(3)://doleva dolů
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x-t,y+t, text[t],prepisovat, rezim, vykreslit);
            break;
        case(4)://zprava doleva - pozpátku
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x-t,y, text[t],prepisovat, rezim, vykreslit);
            break;
        case(5)://doleva nahoru - pozpátku
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x-t,y-t, text[t],prepisovat, rezim, vykreslit);
            break;
        case(6)://zezdola nahoru - pozpátku
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x,y-t, text[t],prepisovat, rezim, vykreslit);
            break;
        case(7)://doprava nahoru
            for(t=0;t<delka;t++)    vykreslitZnakZTextu(x+t,y-t, text[t],prepisovat, rezim, vykreslit);
            break;
    }
}

/*umístění textu na patřičné místo na obrazovce*/
umistitText(int x, int y, char text[]){ //dodelat
    char c = 0;
    int delka = strlen(text);

    editRezim = 0;

    //gotoxy(xn,yn);
    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
    printf("Umisteni textoveho retezce potvrdte stiskem Mezerniku");
    gotoxy(xn,yn);
    vykreslitText(xn, yn, delka,text, (c==' ')?1:0, NOVY, 1);   //úvodní vykreslení umísťovaného textu
    gotoxy(xn,yn);

    #define smazatKurzory vykreslitText(xn, yn, delka,text, (c==' '||editRezim)?1:0, PUVODNI, 1); //smaže po sobě místo, se kterého se právě přesunuje pryč

    do{
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){
                case('d'):		// do prava
                    smazatKurzory;
                    posunKurzoru(1, 0);
                    break;
                case('a'):		// do leva
                    smazatKurzory;
                    posunKurzoru(-1, 0);
                    break;
                case('s'):		// dolu
                    smazatKurzory;
                    posunKurzoru(0, 1);
                    break;
                case('w'):		// nahoru
                    smazatKurzory;
                    posunKurzoru(0, -1);
                    break;
                case('q'):      //SZ
                    smazatKurzory;
                    posunKurzoru(-1, -1);
                    break;
                case('e'):      //SV
                    smazatKurzory;
                    posunKurzoru(1, -1);
                    break;
                case('y'):      //JZ
                    smazatKurzory;
                    posunKurzoru(-1, 1);
                    break;
                case('c'):      //JV
                    smazatKurzory;
                    posunKurzoru(1, 1);
                    break;
                case('0'): //změna Natočení
                    smazatKurzory;
                    if(natoceni>=7)natoceni = 0;
                    else natoceni++;
                    vypsatNapovedu(xn,yn,1,0); //přepsat Dolní nápovědu, aby uživatel věděl, co a jak se změnilo
                    break;
                case('p'):  //přepne Režim cykl. pohybu
                    smazatKurzory;
                    cyklPohyb = !cyklPohyb;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('='):  //zvýšení Rychlosti pohybu
                    rychlostPohybu++;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(')'):  //snížení Rychlosti pohybu
                    rychlostPohybu--;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(','):  //přepne Dutost čtverců
                    smazatKurzory;
                    duteCtverce=!duteCtverce;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case('1'):  //Režim kreslení...guma
                    editRezim=-1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('2'):  //Režim kreslení...pohyb
                    editRezim=0;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('3'):  //Režim kreslení...editace
                    editRezim=1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('*'):
                case(' '):
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    break;

            }
            gotoxy(xn,yn);
            if(editRezim<0) vykreslitText(xn, yn, delka,text, 1, ZAMAZAT, 0);
            vykreslitText(xn, yn, delka,text, (c==' '||editRezim>0)?1:0, NOVY, 1);
            gotoxy(xn,yn);
        }

    }while(c!=' '&&c!='*');
    if(c=='*') smazatKurzory;

    editRezim=0;

}

/*vykreslí 2 rovnoběžné čáry - viz UM - 2.8*/
nakreslitCary(char c, int delka){
    int t;
    char str[delka+1]; //ve skutečnosti, aby bylo jednodušší to naprogramovat, jde pouze o převlečené umísťování textového řetězce
    for(t=0;t<delka;t++)str[t]=c;
    str[delka]='\0';
    vykreslitText((natoceni%4==0)?xn:(xn+polomerRychlehoCtverce), (natoceni%4==2)?yn:(natoceni%4==1)?yn-polomerRychlehoCtverce:yn+polomerRychlehoCtverce, delka,str, 1, NOVY, 1);
    if(polomerRychlehoCtverce) //pokud je vzdálenost mezi oběma čarami nulová, není třeba čáru vykreslovat dvakrát
        vykreslitText((natoceni%4==0)?xn:(xn-polomerRychlehoCtverce), (natoceni%4==2)?yn:(natoceni%4==1)?yn+polomerRychlehoCtverce:yn-polomerRychlehoCtverce, delka,str, 1, NOVY, 1);
}


int zvyseneX(int oKolik){ /*vrátí X-ovou souřadnici zvýšenou o příslušný počet znaků tak, aby se nalézala v plátně*/
    int x = xn;
    x+=oKolik;
    if(x>X_MAX) (cyklPohyb&&(x -= X_MAX))||(x = X_MAX);
    else if(x<1) (cyklPohyb&&(x += X_MAX))||(x = 1);
    return x;
}
int zvyseneY(int oKolik){  /*vrátí Y-ovou souřadnici zvýšenou o příslušný počet znaků tak, aby se nalézala v plátně*/
    int y = yn;
    y+=oKolik;
    if(y>Y_MAX) (cyklPohyb&&(y -= Y_MAX))||(y = Y_MAX);
    else if(y<1) (cyklPohyb&&(y += Y_MAX))||(y = 1);
    return y;
}


int * vybratOblastDef(); //viz níže

/*výběr oblasti v plátně pomocí dvou červených křížků - viz 5.1*/
int * vybratOblast(int *xy/*čtyřprvkové pole obsahující souřadnice obou vrcholů vybraného obdélníku*/){
    //#define nakreslitKrizek (pouzivaWin10?printf("\x1b[31mX\x1b[0m"): /*putchar('X'))*/ system("ColourText 00 04"), gotoxy(xn, yn), putchar('X'), system("ColourText 00 07"))
    #define nakreslitKrizek (zacatekKresleniCervene(), putchar('X'), konecKresleniCervene()) //nakreslí červenou barvou 'X' a hned zase přepne na barvu bílou
    #define vybrX (xy[vybrHr?0:2]) //souřadnice aktuálně vybraného z obou křížků
    #define vybrY (xy[vybrHr?1:3])
    #define nevybrX (xy[vybrHr?2:0])
    #define nevybrY (xy[vybrHr?3:1])
    #define zahladit ((xn!=nevybrX || yn != nevybrY)&&putchar(zn[xn][yn]))   //uklidí na místě, které právě vybraný křížek opouští; pokud je zrovna náhodou na místě, které opouštíme, druhý z křížků není třeba nic mazat
    int vybrHr = 0;
    char c = 0;

    if(xy==NULL) return vybratOblastDef(); //pokud nemáme počáteční souřadnice pro křížky, zavoláme funkci, která si připraví výchozí souřadnice křížků a s těmi opět zavolá tuto funkci

    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
    vrn(" 0","Cervenym krizkem oznacte okraj obdelniku, ktery chcete vybrat. Pro potvrzeni pouzijte _. Pro prepnuti na druhy hranicni krizek stisknete _."); //vypsat instrukce
    gotoxy(nevybrX, nevybrY);
    nakreslitKrizek;
    xn = vybrX; yn = vybrY;
    if(xn != nevybrX || yn != nevybrY){ //pokud se oba křížky překrývají, je hloupé kreslit je dvakrát
        gotoxy(xn,yn);
        nakreslitKrizek;
    }
    gotoxy(xn, yn);

    do{
        if(kbhit()){
            switch(c=prelozitZadanyZnak(malePismeno(getch()))){
                case('d'):		// pohyb do prava
                    zahladit;
                    posunKurzoru(1, 0);
                    break;
                case('a'):		// do leva
                    zahladit;
                    posunKurzoru(-1, 0);
                    break;
                case('s'):		// dolu
                    zahladit;
                    posunKurzoru(0, 1);
                    break;
                case('w'):		// nahoru
                    zahladit;
                    posunKurzoru(0, -1);
                    break;
                case('q'):      //SZ
                    zahladit;
                    posunKurzoru(-1, -1);
                    break;
                case('e'):      //SV
                    zahladit;
                    posunKurzoru(1, -1);
                    break;
                case('y'):      //JZ
                    zahladit;
                    posunKurzoru(-1, 1);
                    break;
                case('c'):      //JV
                    zahladit;
                    posunKurzoru(1, 1);
                    //proPresahHranice();
                    break;
                case('0'):  //přepnutí na druhý kurzor
                    vybrHr = !vybrHr;
                    nevybrX = xn; nevybrY = yn;
                    xn = vybrX; yn = vybrY;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('p'):  //Režim cyklického pohybu
                    cyklPohyb = !cyklPohyb;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('='):
                    rychlostPohybu++;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(')'):
                    rychlostPohybu--;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(','):
                    duteCtverce=!duteCtverce;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case('*'): //výběr zamítnut
                case(' '): //výběr potvrzen
                    gotoxy(nevybrX, nevybrY); //vše po sobě z obrazovky uklidíme
                    putchar(zn[nevybrX][nevybrY]);
                    vybrX = xn; vybrY  = yn;
                    gotoxy(xn, yn);
                    putchar(zn[xn][yn]);
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    gotoxy(xn, yn);
                    goto k; //přeskočíme příkazy k nakreslení křížku

            }
            gotoxy(xn,yn);
            nakreslitKrizek;
            gotoxy(xn,yn);
        }

    }while(c!=' '&&c!='*');

    k://na jména návěstí pro goto nemám žádnou fantasii
    if(c=='*'){ //na znamení, že byl výběr zamítnut vrátíme NULL
        free(xy);
        return NULL;
    }

    //upravíme souřadnice tak, aby první vždy ukazovala na levý horní a druhá na pravý dolní roh vybraného obdélníku - s tím se pak lépe pracuje
    if(xy[0] > xy[2]) (vybrHr = xy[0]), (xy[0] = xy[2]), (xy[2] = vybrHr);
    if(xy[1] > xy[3]) (vybrHr = xy[1]), (xy[1] = xy[3]), (xy[3] = vybrHr);

    return xy;
}

/*zavolá funkci vybratOblast začínajíce s oběma ukazateli na aktuální souřadnici kurzoru*/
int *vybratOblastDef(){
    int *xy = (int *) malloc(4*sizeof(int));
    xy[0] = xy[2] = xn; xy[1] = xy[3] = yn;
    return vybratOblast(xy);
}


/*jako vykreslitText(...), akorát pro umísťování oblasti*/
int vykreslitKopirovanouOblast(/*odkud začíná vykreslování*/int x, int y, char **obl, /*jak velká je daná oblast*/ int lx, int ly,/*násl. argumenty akorát předává funkci pro vykreslení znaku*/ int prepisovat, moznVykresleniZnaku rezim, int vykreslit){
    int t, u;

    //gotoxy(x,y);
    switch(natoceni){
        case(0): //normálně
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    {vykreslitZnakZTextu(x+t,y+u, obl[t][u],prepisovat, rezim, vykreslit);}}
            break;
        case(1)://zrcadlově
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    {vykreslitZnakZTextu(x+t,y+u, obl[lx-t-1][u],prepisovat, rezim, vykreslit);}}
            break;
        case(2):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x-u,y+t, obl[t][u],prepisovat, rezim, vykreslit);}
            break;
        case(3):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x-u,y+t, obl[lx-t-1][u],prepisovat, rezim, vykreslit);}
            break;
        case(4):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x-t,y-u, obl[t][u],prepisovat, rezim, vykreslit);}
            break;
        case(5):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x-t,y-u, obl[lx-t-1][u],prepisovat, rezim, vykreslit);}
            break;
        case(6):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x+u,y-t, obl[t][u],prepisovat, rezim, vykreslit);}
            break;
        case(7):
            for(t=0;t<lx;t++){ for(u=0;u<ly;u++)    vykreslitZnakZTextu(x+u,y-t, obl[lx-t-1][u],prepisovat, rezim, vykreslit);}
            break;
    }
}

/*umístění oblasti na patřičné místo na obrazovce*/
umistitKopirovanouOblast(char **obl, int lx, int ly){
    char c = 0;
    editRezim = 0;
    vypsatNapovedu(xn,yn,1,0);
    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
    printf("Umisteni shluku znaku potvrdte stiskem Mezerniku");
    gotoxy(xn,yn);    if(duteCtverce && obl[0][0]==' ') putchar(zn[xn][yn]); //kdyby tu toto nebylo, tak by pri splneni techto podminek na miste, kde byl kurzor pred zacatkem umistovani oblasti, znak kurzoru zustal jako falesny znak az do prekresleni obrazovky
    vykreslitKopirovanouOblast(xn, yn, obl, lx, ly, (c==' ' || editRezim)?1:0, NOVY, 1);//úvodní vykreslení oblasti
    gotoxy(xn,yn);

    #define PROCEDURA (vykreslitKopirovanouOblast(xn, yn, obl, lx, ly, (editRezim<0)?1:0, ((editRezim<0)?ZAMAZAT:PUVODNI), 1)) //uklidí na místě, odkud se oblast chystá přesunout

    do{
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){
                case('d'):		// posun do prava
                    PROCEDURA;
                    posunKurzoru(1,0);
                    break;
                case('a'):		// do leva
                    PROCEDURA;
                    posunKurzoru(-1,0);
                    break;
                case('s'):		// dolu
                    PROCEDURA;
                    posunKurzoru(0,1);
                    break;
                case('w'):		// nahoru
                    PROCEDURA;
                    posunKurzoru(0,-1);
                    //proPresahHranice();
                    break;
                case('q'):      //SZ
                    PROCEDURA;
                    posunKurzoru(-1, -1);
                    break;
                case('e'):      //SV
                    PROCEDURA;
                    posunKurzoru(1, -1);
                    break;
                case('y'):      //JZ
                    PROCEDURA;
                    posunKurzoru(-1, 1);
                    break;
                case('c'):      //JV
                    PROCEDURA;
                    posunKurzoru(1, 1);
                    break;
                case('0'):  //změna Natočení
                    PROCEDURA;
                    if(natoceni>=7)natoceni = 0;
                    else natoceni++;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('p'):
                    PROCEDURA;
                    cyklPohyb = !cyklPohyb;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('='):
                    rychlostPohybu++;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(')'):
                    rychlostPohybu--;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(','):
                    PROCEDURA;
                    duteCtverce=!duteCtverce;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case('1'):
                    editRezim=-1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('2'):
                    editRezim=0;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('3'):
                    editRezim=1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('*'):
                case(' '):
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    break;

            }
            gotoxy(xn,yn);
            if(editRezim<0) vykreslitKopirovanouOblast(xn, yn, obl, lx, ly, 1, ZAMAZAT, 0); //interně zamaže znaky, které se nalézají na místech, kde se ve skutečnosti vykreslí daná oblast
            vykreslitKopirovanouOblast(xn, yn, obl, lx, ly, (c==' '||editRezim>0)?1:0, NOVY, 1); //vykreslí oblast na místě, kde se právě nachází; případně ji rovnou i obtiskne do plátna
            gotoxy(xn,yn);
        }

    }while(c!=' ' && c!='*');
    if(c=='*') vykreslitKopirovanouOblast(xn, yn, obl, lx, ly, 0, PUVODNI, 1); //při násilném ukončení programu po sobě uklidíme

    editRezim=0;
}

/*naalokuje paměť a nahraje do ní daný výřez z plátna*/
int zkopirovatVybrOblDoZasobniku(int *xy, char ***kam, int *x, int *y){
    int t, u;
    *x = xy[2] - xy[0]+1;
    *y = xy[3] - xy[1]+1;
    gotoxy(xn = xy[0], yn = xy[1]);
    free(*kam); //asi by stálo za to trochu vylepšit, aby nedocházelo k únikům paměti
                                                                                                                                                                                                                                                                           /*printf("Zadejte X a Y rozmer obdelniku, ktery chcete zkopirovat. (Pocitano od kurzoru na JV): ");     scanf("%d %d", &x, &y);*/
    *kam = (char **) malloc(*x*sizeof(char*));
    for(t = 0;t<*x;t++){
        (*kam)[t] = (char *) malloc(*y);
        for(u=0;u<*y;u++){
            (*kam)[t][u] = zn[zvyseneX(t)][zvyseneY(u)];
        }
    }
}

/*má nas starosti kompletní organizaci výběru a následného umísťování oblasti z plátna*/
int kopirovat(){
    int *xy;
    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
    if(vybrObl == NULL /*pokud žádnou oblast ještě nemáme, nemá smysl se ptát, zda ji uživatel nechce použít*/|| !anoNe(0, Y_OTAZEK, "Prejete si znovu umistit minule vybrany shluk znaku?" , 0)/*pokud uživatel chce umístit již vybranou oblast, nebudeme ho obtěžovat s opětovným výběrem*/){
        if((xy = vybratOblastDef())==NULL) return -1; //pokud byl výběr neúspěšný, vrátíme se zpět do volající funkce
        zkopirovatVybrOblDoZasobniku(xy, &vybrObl, &oblX, &oblY);
    }
    umistitKopirovanouOblast(vybrObl, oblX, oblY);
}

/*má na starosti provedení uživatele celým procesem ořezu plátna*/
int orez(){
    char **kus; /*pole, kam se nahraje výřez z plátna*/
    int *xy = NULL, x, y, pom;
    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);

    do{
        if((xy=vybratOblast(xy))==NULL){ //pokud byl výběr oblasti neúspěšný, uklidíme po sobě a vrátíme se do volající funkce
            prekreslit(0);
            return 1;
        }
        vykreslitOhraniceni(xy[0],xy[1], xy[2], xy[3]);  //pro větší názornost kolem vybrané oblasti vykreslíme ohraničení
    } while((pom=anoNe(0, Y_OTAZEK, "Skutecne chcete obrazek seriznout takto?", 0)), prekreslit(0), !pom); //zeptá se uživatele, zda je s výběrem spokojený, až dostane odpověď, smaže ohraničení, a pak se, podle odpovědi, buď vrátí zpět na výběr oblasti, nebo pokračuje dál, k oříznutí aktuálního výběru

    xy[0]--;//zvýší velikost vybrané oblasti o 1 bod doleva a nahoru - tato část se potom namapuje na body s nulovou jednou ze souřadnic, které program ignoruje, ale být zde musí
    xy[1]--;

    zkopirovatVybrOblDoZasobniku(xy, &kus, &x, &y); //zkopíruje vybranou část z plátna do našeho pole

    free(zn); //zbavíme se celého plátna
    zn = kus; //na místo plátna dáme pole, do kterého jsme provedli náš výřez

    X_MAX = x-1;//nastavíme okrajové souřadnice
    Y_MAX = y-1;
    Y_OTAZEK = Y_MAX + ODSAZENI_Y_OTAZEK;

    xn = yn = 1; //umístíme kurzor do levého horního rohu
    prekreslit(1); //smažeme obrazovku a vše vykreslíme odznova

    kurzorZaujmoutMisto;
}
/*!Funkce pro výběr a umísťování oblastí plátna a textových řetězců - konec*/


/*přečte z klávesnice znak či jeho pořadí v ASCII*/
char zadatNovyKresliciZnak(int x, int y, int kVybileni){
    char znak, text[8];
    vybilit(x,y, kVybileni);
    printf("Zadejte znak nebo jeho dec. poradi v ASCII tabulce: ");
    while(1){
            if(((*text)=getchar())==' '||(*text)=='\n'){ //přeskočíme bílé znaky
                    while((*text)!='\n')(*text)=getchar();
            }
            else{
                    ungetc((*text),stdin); //vrátíme do výchozího vstupu poslední znak, který jsme přečetli - ten už je totiž součástí dat, která chceme
                    scanf("%s", text);
                    while(getchar()!='\n'); //zbavíme se otravných enterů, které by nám někde jinde mohly překážet
                    if(text[1]=='\0'){  //pokud vstup sestává jen z jednoho znaku, je zcela jisté, že uživatel chtěl zadat přímo znak (povolené znaky začínají až od ASCII 33)
                    return text[0];
                    //break;
                    }
                    if(strObsahujeJenCislice(text)){ //pokud je vstup není jednoznakový a je jím číslo, jde o ASCII kód
                        text[0] = atoi(text); //převede řetězec na číslo
                        if(((unsigned char)text[0])>32&&((unsigned char)text[0])<255) return text[0];
                    }
                }
                //byl zadán neplatný vstup
                vybilit(x,y, kVybileni);
                printf("Chyba! Zadejte platny znak nebo jeho dec. poradi v ASCII tabulce: ");
            }
}

/*přečte od uživatele celé číslo*/
int zadatCislo(int x, int y, int kVybileni){
    int cislo;
    vybilit(x,y, kVybileni);
    printf("Zadejte cislo: ");
    while(scanf("%d", &cislo)<1){
        vybilit(x, y, kVybileni);
        printf("Chyba! Zadejte platne cislo: ");
    }
    return cislo;
}





/*!HAD*/

/*had je ukládán jako spojový seznam*/
typedef struct hadClanek{
    int x, y; //souřadnice na plátně
    struct hadClanek *dalsi, *predchozi;
}   HADCL; //HADi CLanek


/*projede celé plátno a zjistí, kolik prázdných míst, kde se může hat nalézat, v něm je - podle toho stanoví maximální počet bodů, kterého když had dosáhne, hra se ukončí s hláškou o vítězství*/
int zjistitMaxPBoduVHadovi(){
    int t, u, p;
    for(t=1, p=0;t<=X_MAX;t++){
        for(u=1;u<=Y_MAX;u++){
            if(zn[t][u]==' ')p++;
        }
    }
    return p;
}

/*nastaví čas, který proběhne mezi dvěma pohyby hada*/
definovatZdrzeniHada(){
    /*náhodný předpis, který mě zrovna napadl a zdá se nést jakž-takž rozumné výsledky*/
    ZDRZENI_HADA = 100 + (-rychlostPohybu+1)*((abs(rychlostPohybu-1)<5)?10:((abs(rychlostPohybu-1)>9)?2:5));
    if(ZDRZENI_HADA<1) ZDRZENI_HADA = 1;
}

/*vygeneruje náhodnou pozici na plátně a umístí na ni dané jídlo*/
int genPoziciJidla(int *xp, int *yp, int s/*zda jde o obyčejné, či Specielní jídlo*/){
    #define genX (*xp = (rand()%X_MAX) + 1)
    #define genY (*yp = (rand()%Y_MAX) + 1)
    //if(zn[*xp][*yp]==C_JIDLA)   (zn[*xp][*yp]=' ');
    if(HAD_CHODI_SIKMO<0){ /*když se chodí výhradně po diagonálách - lze se dostat pouze na každý druhý bod*/
        genX;
        *yp = yn +(rand()%(Y_MAX/2))*2+(*xp-xn)%2;
        while(*yp>Y_MAX) *yp -=Y_MAX;
        while(zn[*xp][*yp]!=' '){
            genX;
            *yp = yn +(rand()%(Y_MAX/2))*2+(*xp-xn)%2;
            while(*yp>Y_MAX) *yp -=Y_MAX;
        }
    }
    else{
        genX;
        genY;
        while(zn[*xp][*yp]!=' '){
            if(naZacatkuNacteniSOrezem/*první volná proměnná, na kterou jsem narazil*/) genX;
            else genY;  /*v případě, že vygenerované souřadnice nevyhovují, generují se střídavě pouze Xová nebo pouze Yová souřadnice - optimalizace*/
            naZacatkuNacteniSOrezem = !naZacatkuNacteniSOrezem;
        }
    }

    zn[*xp][*yp] = (s<0)?C_JIDLA:C_JIDLA_S; /*do pole náležícího plátnu se umístí interní bílý znak symbolizující jídlo (buď obyčejné, nebo Specielní)*/
}

/*nakreslí na daných souřadnicích znak daného jídla*/
int vykreslitJidlo(int xp, int yp, int s/*zda se jedná o obyčejné, či o Specielní jídlo*/){
    gotoxy(xp, yp);
    putchar((s<0)?ZN_JIDLA:ZN_JIDLA_S[s]);
    return 1;
}

/*s určitou pravděpodobností vygeneruje a vykreslí Specielní jídlo*/
int specielniJidlo(int *xps, int *yps, int *s){
    if((povolenoSpecialniJidlo>0)&&rand()<=pkostJidlaS){
        //rozhodnuto, že se vygeneruje nějaké specielní jídlo
        *s = rand()%povolenoSpecialniJidlo; // výběr, které ze specielních jídel se objeví
        genPoziciJidla(xps, yps, *s);
        vykreslitJidlo(*xps, *yps, *s);
        return trvaniJidlaS[*s];
    }
    return 0;
}

/*k úklidu Specielního jídla, jehož časový limit vypršel*/
int ukliditProsleSjidlo(int xps, int yps){
    zn[xps][yps] = ' ';
    gotoxy(xps, yps);
    putchar(' ');
    vybilit(0, Y_OTAZEK+1, POCET_K_VYBILENI);
}
/*vypíše pod Dialogovou řádkou, kolik zbývá času pro aktuální Specielní jídlo*/
int vypsatCasovacProSjidlo(int s, int cas){
    vybilit(0, Y_OTAZEK+1, POCET_K_VYBILENI);
    printf("%c... zbyva %.2fds", ZN_JIDLA_S[s], cas/1000.);
}

/*vrátí Xovou souřadnici, na které se had ocitne při svém dalším posunu*/
int dalsiXdleNatoceni(int x){
    /*natočení funguje stejně jako při umísťování textového řetězce*/
    if(natoceni<0) return x;
    x+=((natoceni<2||natoceni==7)?1:((natoceni>2&&natoceni<6)?-1:0));
    if(!cyklPohyb&&(x>X_MAX||x<1)) return -1; //Režim cykl. pohybu neaktivní a had se chystá vejít do okraje
    while(x>X_MAX) x-=X_MAX; //v případě, že cykl. pohyb platí, či nehraje roli
    while(x<1) x+=X_MAX;
    return x;
}
int dalsiYdleNatoceni(int y){  /*vrátí Yovou souřadnici, na které se had ocitne při svém dalším posunu*/
    y+=((natoceni>0&&natoceni<4)?1:((natoceni>4)?-1:0));
    if(!cyklPohyb&&(y>Y_MAX||y<1)) return -1;
    while(y>Y_MAX) y-=Y_MAX;
    while(y<1) y+=Y_MAX;
    return y;
}

/*zpravidla se používá k přesunutí koncového bodu hada do místa, kde bude nově jeho hlava*/
int posunoutBodDleNatoceni(HADCL *zaklad, HADCL *ulozitDo){
    if(natoceni<0) return -1; //pokud toto nastalo, tak se děje něco divného
    ulozitDo->x = dalsiXdleNatoceni(zaklad->x);
    ulozitDo->y = dalsiYdleNatoceni(zaklad->y);
    if(ulozitDo->x<0||ulozitDo->y<0) {ulozitDo->x = ulozitDo->y = 0; return -1;} //nelze jít do záporu
    zn[ulozitDo->x][ulozitDo->y] = C_HAD; //na nové pozici vyznačíme, že tam je had
    return 0;
}

/*vykreslí 1. dva znaky hada - stejně se může v 1 okamžiku posunout nanejvýš o 1 bod, takže vše funguje a vliv na výkon je výrazný*/
int vykreslitHada(HADCL *zacatek){
    gotoxy(zacatek->x, zacatek->y);
    putchar(ZN_HLAVA); //posuneme hlavu na nové místo
    gotoxy(zacatek->predchozi->x, zacatek->predchozi->y);
    putchar(ZN_TELO); //místo, kde byla hlava, přepíšeme znakem odpovídajícím tělu
    return 0;
}

/*vykreslí celého hada od počátečního znaku až do konce*/
int vykreslitCelehoHada(HADCL *zacatek){
    while(zacatek!=NULL){
        gotoxy(zacatek->x, zacatek->y);
        putchar((zacatek->dalsi==NULL)?ZN_HLAVA:ZN_TELO);
        zn[zacatek->x][zacatek->y] = C_HAD;
        zacatek = (zacatek->predchozi);
    }
    return 0;
}

/*obtiskne na plátno celý výjev při skončení Hada*/
int zaznamenatKoncovyStavHada(HADCL *zacatek, int xp, int yp, int jidloS, int xps, int yps){
    while(zacatek!=NULL){
        zn[zacatek->x][zacatek->y] = (zacatek->dalsi==NULL)?ZN_HLAVA:ZN_TELO;
        zacatek = (zacatek->predchozi);
    }
    if(xp<1||yp<1) return 1;
    zn[xp][yp] = ZN_JIDLA;
    if(jidloS<0) return 2;
    zn[xps][yps] = ZN_JIDLA_S[jidloS];
    return 0;
} /*uklidí na plátně po skončení Hada*/
int vymazatKoncovyStavHada(HADCL *zacatek, int xp, int yp, int xps, int yps){
    while(zacatek!=NULL){
        zn[zacatek->x][zacatek->y] = ' ';
        gotoxy(zacatek->x, zacatek->y);
        putchar(' ');
        zacatek = (zacatek->predchozi);
    }
    if(xp<1||yp<1) return 1;
    zn[xp][yp] = ' ';
    gotoxy(xp, yp);
    putchar(' ');
    if(xps>0&&yps>0){
        zn[xps][yps] = ' ';
        gotoxy(xps,yps);
        putchar(' ');
    }
    return 0;
}


vypsatSkore(int sk){
    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
    printf("Skore: %d", sk);
}

/*posune hada o 1 bod dopředu*/
int posunoutHada(HADCL **zacatek, HADCL **konec){
    (*zacatek)->dalsi = *konec;
    zn[(*konec)->x][(*konec)->y] = ' ';
    gotoxy((*konec)->x,(*konec)->y); putchar(' ');
    posunoutBodDleNatoceni(*zacatek, *konec);
    *konec = (*konec)->dalsi;
    (*konec)->predchozi = NULL;
    (*zacatek)->dalsi->predchozi = *zacatek;
    (*zacatek) = (*zacatek)->dalsi;
    (*zacatek)->dalsi = NULL;

    return 0;

}

/*přidá nový článek jakoby na konec hada a při tom hada posune dopředu*/
int pridatKHadovi(HADCL **zacatek){
    (*zacatek)->dalsi = (HADCL *) malloc(sizeof(HADCL));
    posunoutBodDleNatoceni(*zacatek, (*zacatek)->dalsi);
    (*zacatek)->dalsi->predchozi = *zacatek;
    *zacatek = (*zacatek)->dalsi;
    (*zacatek)->dalsi = NULL;

    return 0;
}

/*najde článek, kterým had prochází skrz okraj plátna*/
/*používá se, v situaci, kdy had prochází skrz okraj a hráč zrovna vypne Režim cyklického pohybu, aby se přečuhující část mohla elegantně odříznout*/
HADCL * najitMistoSeriznuti(HADCL *zacatek){
    int tx, ty;

    while(zacatek->predchozi!=NULL){
        if((tx= (zacatek->x - zacatek->predchozi->x))>1||tx<-1||(ty = (zacatek->y-zacatek->predchozi->y))>1||ty<-1) return zacatek;
        zacatek = zacatek->predchozi;
    }

    return zacatek;
}

/*seřízne hada, když zrovna prohází skrz okraj plátna a vypne se Režim cykl. pohybu*/
int seriznoutHada(HADCL *novyKonec){
    int t = 0;
    if(novyKonec->predchozi==NULL) return 0;
    novyKonec = novyKonec->predchozi;
    novyKonec->dalsi->predchozi = NULL;

    while(novyKonec->predchozi!=NULL){
        novyKonec = novyKonec->predchozi;
        free(novyKonec->dalsi);
        t++;
    }
    free(novyKonec);
    t++;
    return t;
}

/*zjistí, zda je současné skóre novým rekordem a případně ho zaznamená*/
int overitNovyRekord(int kandidat){
    if(kandidat>hadiRekord){
        hadiRekord = kandidat;
        ulozitConfig();
        return 1;
    }
    return 0;
}



#define MISTO_KDE_JE_HLAVA (zn[zacatek->x][zacatek->y])
#define MISTO_KDE_BUDE_HLAVA (zn[dalsiXdleNatoceni(zacatek->x)][dalsiYdleNatoceni(zacatek->y)])
int had(){
    int score = 2, xp, yp/*, xpom, ypom*/,xps=0,yps=0,  maxB = 0, jidloS = -1, zbyvajiciCasNaS = 0;
    HADCL *zacatek, *konec;
    char c, d;

    if(zn[xn][yn] != ' '){ /*uživatel se snaží začít hrát hada, když kurzor není na volné pozici - vypíše chybovou hlášku a skončí*/
        vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
        puts("Hada neni mozne zacit ve zdi!");
        gotoxy(xn, yn);
        return -1;
    }

    zn[0][0] = C_NA_NULA_NULA;
    maxB = zjistitMaxPBoduVHadovi(); //zjistí, jaké je maximální teoreticky dosažitelné skóre

    gotoxy(xn, yn);
    putchar(ZN_HLAVA); //vykreslí hlavu hada - čeká na stisknuí kurzorové klávesy, aby hra mohla začít


    for(natoceni = -1;natoceni==-1;){
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){ //výběr směru, ve kterém se had začne pohybovat - nesmí vést bezprostředně do zdi
                case('w'):
                    natoceni = 6;
                    if((yp=dalsiYdleNatoceni(yn))<1||zn[xn][yp]!=' '||HAD_CHODI_SIKMO<0) natoceni = -1;
                    break;
                case('d'):		// do prava
                    natoceni = 0;
                    if((xp=dalsiXdleNatoceni(xn))<1||zn[xp][yn]!=' '||HAD_CHODI_SIKMO<0) natoceni = -1;
					break;
				case('a'):		// do leva
				    natoceni = 4;
				    if((xp=dalsiXdleNatoceni(xn))<1||zn[xp][yn]!=' '||HAD_CHODI_SIKMO<0) natoceni = -1;
					break;
                case('s'):		// dolu
                    natoceni = 2;
                    if((yp=dalsiYdleNatoceni(yn))<1||zn[xn][yp]!=' '||HAD_CHODI_SIKMO<0) natoceni = -1;
					break;
                case('q'):      //SZ
                    natoceni = 5;
                    #define vyraz1   if((yp=dalsiYdleNatoceni(yn))<1||(xp=dalsiXdleNatoceni(xn))<1||zn[xp][yp]!=' '||!HAD_CHODI_SIKMO) natoceni = -1
                    vyraz1;
                    break;
                case('e'):      //SV
                    natoceni = 7;
                    vyraz1;
                    break;
                case('y'):      //JZ
                    natoceni = 3;
                    vyraz1;
                    break;
                case('c'):      //JV
                    natoceni = 1;
                    vyraz1;
                    break;
            }
        }
    }
    /*vytvoří se had sestávající z hlavy a 1 tělového článku*/
    konec = (HADCL *) malloc(sizeof(HADCL));
    zacatek = (HADCL *) malloc(sizeof(HADCL));
    konec->x = xn;      konec->y = yn;
    posunoutBodDleNatoceni(konec, zacatek);
    konec->predchozi = zacatek->dalsi = NULL;
    konec->dalsi = zacatek;     zacatek->predchozi = konec;

    vykreslitHada(zacatek);
    genPoziciJidla(&xp, &yp, -1);
    vykreslitJidlo(xp, yp, -1);
    vypsatSkore(score);
    definovatZdrzeniHada();


    do{ /*nekonečný cyklus, ve kterém hra probíhá*/
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){
                case('w'):      //pohyb nahoru
                    if(HAD_CHODI_SIKMO>=0&&natoceni!=2)natoceni = 6; //nemůže se obrátit nahoru, pokud zrovna jede dolů - to by okamžitě narazil sám do sebe
                    break;
                case('d'):		// do prava
                    if(HAD_CHODI_SIKMO>=0&&natoceni!=4)natoceni = 0;
					break;
				case('a'):		// do leva
					if(HAD_CHODI_SIKMO>=0&&natoceni!=0)natoceni = 4;
					break;
                case('s'):		// dolu
					if(HAD_CHODI_SIKMO>=0&&natoceni!=6)natoceni = 2;
					break;
                case('q'):      //SZ
                    if(HAD_CHODI_SIKMO&&natoceni!=1) natoceni = 5;
                    break;
                case('e'):      //SV
                    if(HAD_CHODI_SIKMO&&natoceni!=3)(natoceni = 7);
                    break;
                case('y'):      //JZ
                    if(HAD_CHODI_SIKMO&&natoceni!=7)(natoceni = 3);
                    break;
                case('c'):      //JV
                    if(HAD_CHODI_SIKMO&&natoceni!=5)(natoceni = 1);
                    break;
                case('='):  //inkrementace rychlosti pohybu
                    rychlostPohybu++;
                    definovatZdrzeniHada(); //přizpůsobit změně rychlosti i rychlost hada
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(')'):  //dekrementace rychlosti pohybu
                    rychlostPohybu--;
                    definovatZdrzeniHada(); //přizpůsobit změně rychlosti i rychlost hada
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case('p'): //přepnutí Režimu cyklického pohybu
                    cyklPohyb = !cyklPohyb;
                    if(!cyklPohyb){
                        konec = najitMistoSeriznuti(zacatek); //najít místo, kde by se had měl o okraj plátna oříznout
                        if(konec->predchozi!=NULL){ //nový konec není totožný s tím starým - není skutečně na konci hada
                            /*když je zrovna Režim editace, odříznutá část hada se obtiskne do plátna; v opačném případě ona část těla zmizí*/
                            (editRezim==1)?zaznamenatKoncovyStavHada(konec->predchozi, 0, 1, jidloS, xps, yps):vymazatKoncovyStavHada( konec->predchozi, 0, 1, ((jidloS>=0)?-1:xps), yps);
                            xn = seriznoutHada(konec);
                            score -= xn; //seříznout se o příslušnou hodnotu musí i skóre
                            if(editRezim==1) maxB -= xn;
                            if(overitNovyRekord(xn)){
                                vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
                                printf("Novy rekord: %d", hadiRekord);
                            }
                        }
                    }
                    vypsatNapovedu(xn,yn,1,0);
                    vypsatSkore(score);
                    break;
                case('1'): //režim kreslení...guma
                    editRezim=-1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('3'): //režim kreslení...editace
                    editRezim=1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('2'): //režim kreslení...pohyb
                    editRezim=0;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
            }
        }

        /*souřadnice, kam se had dostane v příštím kroku*/
        xn = dalsiXdleNatoceni(zacatek->x);
        yn = dalsiYdleNatoceni(zacatek->y);
        if(xn<0||yn<0) xn = yn = 0; //Režim cykl. pohybu není aktivní a narazili jsme do okraje
        d = zn[xn][yn];

        if(zbyvajiciCasNaS>0){vypsatCasovacProSjidlo(jidloS, zbyvajiciCasNaS);} //vypsat zbývající čas pro Bonusové jídlo
        else if(jidloS>=0){zbyvajiciCasNaS = 0, ukliditProsleSjidlo(xps, yps), jidloS = -1;} //čas vypršel


        if(d!=' '&&d!=C_JIDLA&&d!=C_JIDLA_S&&(!(d==C_HAD&&dalsiXdleNatoceni(zacatek->x)==(konec->x)&&dalsiYdleNatoceni(zacatek->y)==(konec->y)))){
            /*narazili jsme do zdi*/
            break;
        }
        else{
            if(d==C_JIDLA||d==C_JIDLA_S){
                /*něco jsme snědli...*/
                if(d==C_JIDLA_S){ /*...a je to Bonusové jídlo*/
                    score+=hodnotyJidlaS[jidloS];
                    jidloS = -hodnotyJidlaS[jidloS]; //převrátíme na zápornou hodnotu, aby program věděl, že žádné Bonusové jídlo není aktivní
                    if(jidloS!=0)pridatKHadovi(&zacatek); //přidáme 1 článek k hadovi; ty zbývající budeme přidávat postupně v průběhu následujících kroků, aby to bylo hezky rozložené a konzistentní a nevypadalo to úplně šíleně
                    else posunoutHada(&zacatek, &konec); //pokud byla hodnota jídla nulová, je třeba hada o políčko posunout
                    vybilit(0, Y_OTAZEK+1, POCET_K_VYBILENI); //uklidíme po časovači
                    zbyvajiciCasNaS = 0; //vynulujeme časovač
                }
                else{ /*! d==C_JIDLA - obyčejné jídlo*/
                    pridatKHadovi(&zacatek); /*přidáme k hadově tělu článek*/
                    score++;

                    genPoziciJidla(&xp, &yp,-1); /*vygenerujeme nové jídlo*/
                    vykreslitJidlo(xp, yp,-1);
                    if(jidloS==-1)zbyvajiciCasNaS = specielniJidlo(&xps, &yps, &jidloS); /*pokud není ve hře žádné bonusové jídlo, můžeme ho zkusit vygenerovat*/
                }
                vypsatSkore(score);
                if(maxB==score){ //zaplnili jsme celé hrací pole - výhra
                    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
                    printf("Vyhral jste!  (Score: %d)", score); vrn("*", "\t\tPro vystup ze hry stisknete _");
                    continue;
                }
            }
            else{ //nic jsme nesnědli
                if(jidloS<-1){ //ještě se nepřidaly všechny články za bonusové jídlo, které by měly
                   pridatKHadovi(&zacatek);
                   jidloS++;
                }
                else posunoutHada(&zacatek, &konec);
            }
            vykreslitHada(zacatek); //vykreslíme nově posunutého hada
            Sleep(ZDRZENI_HADA); //počkáme na další krok
            zbyvajiciCasNaS -= ZDRZENI_HADA;
        }


    } while(c!='*');

    /*konec hry*/

    zacatekKresleniCervene(); //vykreslíme všechny herní prvky v červené barvě, na znamení, že uživatel prohrál
    vykreslitCelehoHada(zacatek);
    vykreslitJidlo(xp, yp,-1);
    if(jidloS>=0)vykreslitJidlo(xps, yps,jidloS);
    konecKresleniCervene();

    /*dle volby uživatele buď celý výjev obtiskneme do plátna, nebo po sobě uklidíme*/
    anoNe(0, Y_OTAZEK, "Prejete si tento vyjev zakomponovat do obrazu?", 0)?vykreslitCelehoHada(zacatek), vykreslitJidlo(xp, yp,-1), ((jidloS>=0)?vykreslitJidlo(xps, yps, jidloS):0),zaznamenatKoncovyStavHada(zacatek, xp, yp, jidloS, xps, yps), kresliciZnak = ZN_HLAVA:vymazatKoncovyStavHada(zacatek, xp, yp, (jidloS>=0?xps:-1), yps);


    vybilit(0, Y_OTAZEK, POCET_K_VYBILENI);
    printf("Skore: %d  %s%d)  ", score, (score>hadiRekord)?"  NOVY REKORD!   (Predchozi...": "  (Rekord: ", hadiRekord);
    overitNovyRekord(score);

    xn = zacatek->x; yn = zacatek->y;
    gotoxy(xn, yn);
    putchar(kresliciZnak);
    gotoxy(xn, yn);
    editRezim = 0;

    /*návrat do základního editačního režimu*/
    return score;
}

/*!HAD - konec*/


/*!Nastavení*/

#define SETTINGS_OBECNE_INKR_1 'd' //znaky obecně sloužící v Nastavení k inkrementaci něčeho
#define SETTINGS_OBECNE_INKR_2 '+'
#define SETTINGS_OBECNE_DEKR_1 'a' //znaky obecně sloužící v Nastavení k dekrementaci něčeho
#define SETTINGS_OBECNE_DEKR_2 '-'

#define DOROVNAT_DELKU_RADKU for(radLen-=strlen(str);radLen>=0;radLen--) putchar(' ') //vypíše na zbývající políčka na řádce mezery
/*všechny funkce, které toto makro používají, musí délku řádky, kam mají být fukresleny, uchovávat v proměnné radLen, což zde zrovna platí*/


typedef int (*funkceProNastaveni)(char, int, int, int, int);  //takovouto strukturu musí dodržovat každá funkce, která má být použita jako položka v Nastavení
typedef int (*funkceProResetaciNastaveni)(void);  //takovouto strukturu musí dodržovat každá funkce, která má být použita jako položka v Nastavení


/*funkce stavěná na to, aby ji mohly využívat funkce, které v Nastavení slouží ke konfiguraci nějakého znaku*/
int nastaveniPolozkaObecneZmenitZnak(char c, char *keZmeneni, char *str, int radLen, int x, int y){
    switch(c){
        case SETTINGS_OBECNE_INKR_1:
        case SETTINGS_OBECNE_INKR_2: (*keZmeneni)++;
                   if(((unsigned char)(*keZmeneni))>254) *keZmeneni = (cyklPohyb)?33:254;
                   break;
        case SETTINGS_OBECNE_DEKR_1:
        case SETTINGS_OBECNE_DEKR_2: (*keZmeneni)--;
                   if(((unsigned char)(*keZmeneni))<=33) *keZmeneni = (cyklPohyb)?254:33;
                   break;
        case '/':
            *keZmeneni = zadatNovyKresliciZnak(x, y, radLen); //stejná funkce, jaká se používá pro výběr znaku kurzoru v základním editačním režimu - přečte buď znak, nebo jeho decimální pořadí v ASCII
            break;
    }
    gotoxy(x, y);
    printf(str, (((unsigned char)*keZmeneni)>' ')?(*keZmeneni):' ', (unsigned char)(*keZmeneni)); //vypíše řetězec, který mu předala volající funkce. Počítá s tím, že onen řetězec obsahuje něco na způsob tohoto: "%c %d" - na místo %c vypíše zvolený znak a na %d jeho ASCII hodnotu
    DOROVNAT_DELKU_RADKU; //Aby byla celá řádka hezky vybarvena bílou barvou
    return 0;
}

/*funkce stavěná na to, aby ji mohly využívat funkce, které v Nastavení slouží ke konfiguraci nějakého čísla*/
int nastaveniPolozkaObecneZmenitHodnotu(char c, int *keZmeneni, char *strng, int radLen, int x, int y, int velikostSkoku/*o kolik se číslo zvyšuje/snižuje*/, int max/*nejvyšší možná hodnota*/, int min/*nejnižší možná hodnota*/){
    char str[50];
    switch(c){
        case SETTINGS_OBECNE_INKR_1:
        case SETTINGS_OBECNE_INKR_2:
                   (*keZmeneni)+=velikostSkoku;
                   if((*keZmeneni)>max) *keZmeneni = (cyklPohyb)?min:max;
                   break;
        case SETTINGS_OBECNE_DEKR_1:
        case SETTINGS_OBECNE_DEKR_2:
                   (*keZmeneni)-=velikostSkoku;
                   if((*keZmeneni)<min) *keZmeneni = (cyklPohyb)?max:min;
                   break;
        case '/':
                   *keZmeneni = zadatCislo(x,y,radLen);
                   if((*keZmeneni)>max) *keZmeneni = max;
                   else if((*keZmeneni)<min) *keZmeneni = min;
                   break;
    }
    gotoxy(x, y);
    sprintf(str, strng, *keZmeneni);
    printf(str);
    radLen--; //aby se mezerou nepřemazala i obruba Nastavení
    DOROVNAT_DELKU_RADKU;
    return 0;
}

#define RET_VALUE_PRAZDNEHO_RADKU 5
int nastaveniPolozkaPrazdnyRadek(char c, int radLen, int x, int y, int pom){
    return RET_VALUE_PRAZDNEHO_RADKU; //podle tohoto hlavní funkce pro Nastavení pozná, že má tento řádek přeskočit
}

int nastaveniPolozkaTeloHada(char c, int radLen, int x, int y, int pom){ //upravuje znak, který se zobrazuje na místech článků hadova těla
    return nastaveniPolozkaObecneZmenitZnak(c, &ZN_TELO,       "Telo hada...          '%c' (ASCII_%3d)", radLen, x, y);
}
int nastaveniPolozkaHlavaHada(char c, int radLen, int x, int y, int pom){ //upravuje znak, který se zobrazuje na místě hadovy hlavy
    return nastaveniPolozkaObecneZmenitZnak(c, &ZN_HLAVA,      "Hlava hada...         '%c' (ASCII_%3d)", radLen, x, y);
}
int nastaveniPolozkaZnakJidla(char c, int radLen, int x, int y, int pom){ //upravuje znak, který se zobrazuje na místě obyčejného jídla
    return nastaveniPolozkaObecneZmenitZnak(c, &ZN_JIDLA,      "Hadova potrava...     '%c' (ASCII_%3d)", radLen, x, y);
}

#define DOUPRAVIT_POM_PRO_JIDLO_S pom = ((pom-1)/3) //Funkce pro specielní jídlo jsou jediné, které argument pom využívají
/*pom značí číslo řádku, na kterém se daná položka v Nastavení nalézá - dle toho se dá jednoduše dopočítat, kterému z Bonusových jídel zrovna volaná položka náleží*/
int nastaveniPolozkaZnakJidlaS(char c, int radLen, int x, int y, int pom){ //upravuje znak, který se zobrazuje na místě některého ze specielních jídel
    char str[50];
    DOUPRAVIT_POM_PRO_JIDLO_S;
    sprintf(str, "Specielni jidlo %2d...     '%%c' (ASCII_%%3d)", pom+1/*abychom jídla pro uživatele nečíslovali od nuly*/);
    return nastaveniPolozkaObecneZmenitZnak(c, &ZN_JIDLA_S[pom], str, radLen, x, y); //upraví znak daného Bonusového jídla
}
int nastaveniPolozkaTrvaniJidlaS(char c, int radLen, int x, int y, int pom){ //upravuje čas, jak dlouho dané Bonusové jídlo zůstává ve hře
    //char str[50];
    pom-=1;
    DOUPRAVIT_POM_PRO_JIDLO_S;
    //sprintf(str, "                          %%d ms");
    return nastaveniPolozkaObecneZmenitHodnotu(c, &trvaniJidlaS[pom], "                          %d ms", radLen, x, y, 100, 20000, 100);
}
int nastaveniPolozkaHodnotaJidlaS(char c, int radLen, int x, int y, int pom){ //upravuje počet bodů, který za dané bonusové jídlo dostanete
    int t;
    char str[50];
    pom-=2;
    DOUPRAVIT_POM_PRO_JIDLO_S;
    t = hodnotyJidlaS[pom];
    t += (c==SETTINGS_OBECNE_INKR_1||c==SETTINGS_OBECNE_INKR_2)?1:((c==SETTINGS_OBECNE_DEKR_1||c==SETTINGS_OBECNE_DEKR_2)?-1:0);
    sprintf(str, "                          %%d %s", ((t==1)?"bod":((t<5&&t>1)?"body":"bodu")));
    return nastaveniPolozkaObecneZmenitHodnotu(c, &hodnotyJidlaS[pom], str, radLen, x, y, 1, 50,0);
}
int nastaveniPolozkaPravdepodobnostVyskytuJidlaS(char c, int radLen, int x, int y, int pom){ //upravuje pravděpodobnost, zda se Bonusové jídlo objeví
    return nastaveniPolozkaObecneZmenitHodnotu(c, &pkostJidlaS, "Pravdepodobnost vyskytu... %d/32767", radLen, x, y, 1, RAND_MAX,1);
}

int nastaveniPolozkaSmeryPohybu(char c, int radLen, int x, int y, int pom){ //výběr, jakými směry se had může pohybovat
    char str[40];

    switch(c){
        case SETTINGS_OBECNE_INKR_1:
        case SETTINGS_OBECNE_INKR_2: HAD_CHODI_SIKMO++;
                   if(HAD_CHODI_SIKMO>1) HAD_CHODI_SIKMO = (cyklPohyb)?-1:1;
                   break;
        case SETTINGS_OBECNE_DEKR_1:
        case SETTINGS_OBECNE_DEKR_2: HAD_CHODI_SIKMO--;
                   if(HAD_CHODI_SIKMO<-1) HAD_CHODI_SIKMO = (cyklPohyb)?1:-1;
                   break;
    }

    sprintf(str, "Druh pohybu hada...   %s", HAD_CHODI_SIKMO?((HAD_CHODI_SIKMO>0)?"Kombinovany":"Sikmo"):"Rovne");
    printf(str);
    radLen--;
    DOROVNAT_DELKU_RADKU;

    return 0;
}

int NastaveniPolozkaChangelog(char c, int radLen, int x, int y, int pom){ //vstup do Changelogu
    if(c!=' ') { //pokud nebyl rozkliknut, pouze na svou řádku vypíše pozvánku
        radLen-= vrn(" ", "Changelog (pro vstup stisknete _)");
        for(;radLen>0;radLen--) putchar(' '); //!osetrit kdyz uzivatel zmeni Mezernik na jinou klavesu
        return 0;
    }
    //mezerníkem byl rozkliknut
    konecKresleniCervene(); //přepne na výpis bíle na černém
    system("cls"); //smaže obrazovku
    CHANGELOG_PART_1; //vypíše text changelogu
    CHANGELOG_PART_2;
    vrn("* ","\n\n\n\n\n(pro navrat stisknete '_' nebo _)"); //instrukce pro návrat do Nastavení
    c=0;
    while(c!='*'&&c!=' '){
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            //Sleep(100); //nepomaha ke snizeni naroku na procesor
        }
    }
    system("cls");
    return -1;
}

int nastaveniResetZaklHada(){
    ZN_JIDLA = DEFAULT_ZN_JIDLA;       ZN_HLAVA = DEFAULT_ZN__HLAVA;      ZN_TELO = DEFAULT_ZN__TELO; HAD_CHODI_SIKMO = DEFAULT_HAD_CHODI_SIKMO;
}

int nastaveniResetJidlaS(){
    pkostJidlaS = DEFAULT_PKOST_JIDLA_S;

    puvodniPovolenoSpecialniJidlo = 0;
    inicializovatDefaultSpecielniJidlo(0);
}

/*obecná funkce, které předhodíme pole pointerů na jednotlivé položky a několik dalších parametrů a vypíše plně customizovatelné nastavení*/
int jednaInstanceNastaveniRamcove(funkceProNastaveni *set, funkceProResetaciNastaveni resetFce, int pocet, char *nadpis, int x, int y, int delkaRadku, int *vychPozice, char *specZnKeZpraveNakonec,char *zpravaNakonec){
    int t;
    char c;
    if((t=strlen(nadpis))>delkaRadku) delkaRadku = t;

    //!vykresleni zakl. grafiky - zacatek
    gotoxy(x, y++);
    putchar(201);  //znaky pro rámeček
    for(t=0;t<delkaRadku;t++) putchar(205);
    putchar(187);

    gotoxy(x, y);
    putchar(186);
    puts(nadpis);
    gotoxy(x+delkaRadku+1, y++);
    putchar(186);

    gotoxy(x, y++);
    putchar(204);
    for(t=0;t<delkaRadku;t++) putchar(205);
    putchar(185);
    gotoxy(x, y);
    putchar(186);
    gotoxy(x+delkaRadku+1, y);
    putchar(186);

    for(t=0;t<pocet;t++){
        gotoxy(x, y+t);
        putchar(186);
        set[t](0, delkaRadku, x+1, y+t, t); //vypíše text jednotlivých položek
        gotoxy(x+delkaRadku+1, y+t);
        putchar(186);
    }
    gotoxy(x, y+pocet);
    putchar(200);
    for(t=0;t<delkaRadku;t++) putchar(205);
    putchar(188);
    gotoxy(x, y+pocet+1);
    if(*specZnKeZpraveNakonec!='\0')vrn(specZnKeZpraveNakonec,zpravaNakonec);
    else puts(zpravaNakonec);

    //!vykresleni zakl. grafiky - hotovo



    zacatekKresleniCerneNaBilem();
    gotoxy(x+1,y+*vychPozice);
    set[*vychPozice](0, delkaRadku, x+1, y+(*vychPozice), *vychPozice); //vypíše výchozí vybranou položku a umístí na ní kurzor

    //gotoxy(x+1,delkaRadku);

    /*nekonečný cyklus*/
    for(t=*vychPozice, c=0, x++;c!='*';){
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            kkk: //uz mi skutecne absolutne dosla fantasie na jmena
            switch(c) {
                case 'w': //posun o položku výše
                    if(t>0){
                        konecKresleniCervene();
                        gotoxy(x, y+t);
                        set[t](0, delkaRadku, x, y+t, t); //vykreslíme položku, kterou právě opouštíme, klasickou bílou barvou na černém pozadí
                        t--;
                        zacatekKresleniCerneNaBilem();
                    }
                    break;
                case 's': //posun o položku níže
                    if(t<pocet-1) {
                        konecKresleniCervene();
                        gotoxy(x, y+t);
                        set[t](0, delkaRadku, x, y+t, t); //vykreslíme položku, kterou právě opouštíme, klasickou bílou barvou na černém pozadí
                        t++;
                        zacatekKresleniCerneNaBilem();
                    }
                    break;
                case 'p': //přepne Režim cyklického pohybu
                    cyklPohyb = !cyklPohyb;
                    break;
                case('h'):  //navrátí Hada na výchozí nastavení
                    konecKresleniCervene();
                    if(anoNe(0,y+pocet+6,"Jste si jisti, ze chcete nastavit ovladani zpet na vychozi hodnoty?",1)){ //opravdu to chceme udělat
                        //inicializovatPoleNovychZnaku();
                        resetFce();
                        *vychPozice = t;
                        for(t=0;t<pocet;t++){
                            gotoxy(x, y+t);
                            set[t](0, delkaRadku, x, y+t, t); //musí vypsat odznova všechny položky, aby bylo vidět, že se projevily změny
                            gotoxy(x+delkaRadku, y+t);
                        }
                        t = *vychPozice;
                    }
                    else{
                        vybilit(0, y+pocet+5, POCET_K_VYBILENI); //uklidíme po záporně vyřízené otázce
                    }
                    zacatekKresleniCerneNaBilem();
                    break;
            }
            gotoxy(x, y+t);
            switch(set[t](c, delkaRadku, x, y+t, t)) { //návratová hodnota volané funkce
                case -1: //-1 značí, že je potřeba celé nastavení restartovat
                    *vychPozice = t;
                    return -1;
                case RET_VALUE_PRAZDNEHO_RADKU: //vstoupili jsme na prázdný řádek - tímto ho přeskočíme
                    if(c=='w'&&t==0) c='s'; //aby se program nezasekl, když je prázdný řádek úplně nahoře či úplně dole
                    if(c=='s'&&t==pocet) c='w';
                    goto kkk;
            }
        }

    }
    konecKresleniCervene(); //přepneme zase na obvyklé barvy kreslení
    gotoxy(xn, yn);
    ulozitConfig(); //uložit změny do konfiguračního souboru
    return 0; //návrat do základního editačního režimu

}

/*volá nastavení pořád se stejnými argumenty, dokud ono nenavrátí nulu - takto se dá jednoduše vyřešit to, když je potřeba celé nastavení restartovat a vykreslit znovu*/
int nastaveniRamcove(funkceProNastaveni *set, funkceProResetaciNastaveni resetFce, int pocet, char *nadpis, int x, int y, int delkaRadku, int *vychPozice, char *specZnKeZpraveNakonec,char *zpravaNakonec){
    while(jednaInstanceNastaveniRamcove(set, resetFce, pocet, nadpis, x,y,delkaRadku, vychPozice, specZnKeZpraveNakonec,zpravaNakonec)) system("cls");
}



/*podnastavení pro konfiguraci jednotlivých Bonusových jídel*/
int nastaveniProSJidloVHadovi(){
    #define OFFSET_PRVNIHO_S_JIDLA_V_NASTAVENI 3
    int pocetPolozek = abs(povolenoSpecialniJidlo)*3 +OFFSET_PRVNIHO_S_JIDLA_V_NASTAVENI;
    int vychoziPozice = 0, t;
    funkceProNastaveni *set; //vytváří se dynamicky dle toho, kolik je celkem bonusových jídel
    set = (funkceProNastaveni *) malloc(pocetPolozek*sizeof(funkceProNastaveni));

    set[0] = nastaveniPolozkaPravdepodobnostVyskytuJidlaS;  //toto je tu vždy stejně
    set[1] = nastaveniPolozkaPrazdnyRadek;
    set[2] = nastaveniPolozkaPrazdnyRadek;
    for(t=OFFSET_PRVNIHO_S_JIDLA_V_NASTAVENI;t<pocetPolozek;t+=3){  //řádky specifické pro každé Bonusové jídlo
        set[t] = nastaveniPolozkaZnakJidlaS;
        set[t+1] = nastaveniPolozkaTrvaniJidlaS;
        set[t+2] = nastaveniPolozkaHodnotaJidlaS;
    }
    //printf("%d !%d!", t, pocetPolozek); getchar();

    konecKresleniCervene();
    system("cls");

    //zavoláme funkci, která s našimi parametry vykreslí nastavení
    nastaveniRamcove(set, nastaveniResetJidlaS, pocetPolozek, "Nastaveni Specielniho Jidla", 0,0, 90, &vychoziPozice, "*wsad/","Pro navrat do centralniho Nastaveni stisknete '_',\npro pohyb mezi polozkami pouzijte '_' a '_', pro vyber moznosti pouzijte '_' a '_',\nchcete-li zadat pozadovany znak ci cislo rucne, stisknete '_'");

    return -1;
}

/*položka v Nastavení - vybírá se zde počet a aktivnost Bonusových jídel, v případě rozkliknutí otevře podnastavení pro Bonusová jídla*/
int hadiNastaveniSJidlo(char c, int radLen, int x, int y, int pom){
    char str[50];
    int t;

    switch(c){
        case '/': //přepneme aktivnost Bonusových jídel
                povolenoSpecialniJidlo=-povolenoSpecialniJidlo;
                break;
        case ' ': //rozklikneme do podnastavení pro Bonusová jídla
                if(povolenoSpecialniJidlo>0) {
                        inicializovatDefaultSpecielniJidlo(1);
                        return nastaveniProSJidloVHadovi();
                }
                break;
    }

    /*změna počtu bodusových jídel*/
    if((c==SETTINGS_OBECNE_INKR_1||c==SETTINGS_OBECNE_INKR_2||c==SETTINGS_OBECNE_DEKR_1||c==SETTINGS_OBECNE_DEKR_2)&&(povolenoSpecialniJidlo>=0)){
        //předpočítáme si, co funkce pro změnu znaku s daným číslem provede, abychom jí podle toho mohli předhodit správný řetězec k zobrazení - uznávám, že to zdaleka není elegantní, ale nezdá se, že by to mělo nějaký vliv na výkon
        t=povolenoSpecialniJidlo;
        t+=(c==SETTINGS_OBECNE_DEKR_1||c==SETTINGS_OBECNE_DEKR_2)?-1:1;
        if(t<0)t+=(MAX_DRUHU_SPEC_JIDEL+1); if(t>MAX_DRUHU_SPEC_JIDEL) t-=(MAX_DRUHU_SPEC_JIDEL+1);
        sprintf(str, "Bonusova jidla...     %%d  (%s)",(t>0)?"Povolena":"Zakazana");
        t=povolenoSpecialniJidlo;
        nastaveniPolozkaObecneZmenitHodnotu(c, &povolenoSpecialniJidlo, str, radLen, x, y, 1, MAX_DRUHU_SPEC_JIDEL, MIN_DRUHU_SPEC_JIDEL);

    }
    else{ //vypíše počet existujících druhů Bonusových jídel
        vybilit(x,y, radLen);
        sprintf(str, "Bonusova jidla...     %d  (%s)", abs(povolenoSpecialniJidlo),(povolenoSpecialniJidlo>0)?"Povolena":"Zakazana");
        printf(str);
        radLen--;
        DOROVNAT_DELKU_RADKU;
    }

    return 0;
}

/*nastavení, do kterého vstoupíte z hl. editačního režimu stisknutím '|'*/
int hadiNastaveni(){
    #define POCET_POLOZEK_V_NASTAVENI 8
    funkceProNastaveni set[POCET_POLOZEK_V_NASTAVENI]; //pole položek pro funkci Nastavení
    int vychoziPozice = 1/*nemůžeme začít na 0, jelikož tam je prázdný řádek*/, t=0;

    /*nastavíme hodnoty jednotlivých položek*/
    set[t] = nastaveniPolozkaPrazdnyRadek; t++;
    set[t] = nastaveniPolozkaHlavaHada; t++;
    set[t] = nastaveniPolozkaTeloHada; t++;
    set[t] = nastaveniPolozkaZnakJidla; t++;
    set[t] = nastaveniPolozkaSmeryPohybu; t++;
    set[t] = hadiNastaveniSJidlo; t++;
    set[t] = nastaveniPolozkaPrazdnyRadek; t++;
    set[t] = NastaveniPolozkaChangelog; t++;


    system("cls");
    /*otevřeme nastavení*/
    nastaveniRamcove(set, nastaveniResetZaklHada, POCET_POLOZEK_V_NASTAVENI, "Nastaveni"/*nadpis*/, 0,0, 90, &vychoziPozice, /*1. argument pro funkci vrn(...)*/"*wsad//",/*Připomínka, která se vypíše pod tělem nastavení*/"Pro navrat do programu stisknete '_',\npro pohyb mezi polozkami pouzijte '_' a '_', pro vyber moznosti pouzijte '_' a '_',\nchcete-li zadat pozadovany znak ci jeho poradi v ASCII rucne, stisknete '_'\n'_' stisknete i pro zakazani/povoleni specielnich jidel v Hadovi.");

    inicializovatDefaultSpecielniJidlo(1);
    prekreslit(1); //po skončení práce s nastavením vykreslíme zase hlavní obrazovku
    return 0;
}


/*!Nastavení - konec*/


/*běh v základním editačním režimu*/
behProgramu(int zNacteneho){
    char c;
	int t;
    char text[MAX_STRING];

    prizpusobitVelikostOknaRozliseni(X_MAX+90,Y_MAX+30);
    if(!zNacteneho)smazat(1); //pokud byl vytvořen nový soubor, raději ještě všechny znaky smažeme na mezery
    else{ //pokud byl načten soubor, nyní je čas ho vykreslit
                prekreslit(1);xn=1;yn=1;
                gotoxy(xn,yn);  putchar(kresliciZnak);  gotoxy(xn,yn);
                if(naZacatkuNacteniSOrezem){ //chybová hláška
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    printf("Soubor byl nacten ale doslo k orezu!");
                    gotoxy(xn,yn);
                }
    }


	do{/*hlavní nekonečný cyklus celého programu*/
		if(kbhit()/*byla stisknuta klávesa*/){
            if(priNejblizsiPrilezitostiVybilitSpodniRadku) vybilit(0, Y_OTAZEK, POCET_K_VYBILENI), gotoxy(xn, yn);
			c=prelozitZadanyZnak(malePismeno(getch())); /*překlad zadaného znaku na znak z výchozího rozložení kláves, který je pro program srozumitelný*/
			switch(c){		// prepinac
				case('d'):		// posun do prava
                    smazatKurzor(xn,yn);
					posunKurzoru(1,0);

					break;
				case('a'):		// do leva
					smazatKurzor(xn,yn);
					posunKurzoru(-1,0);
					break;
                case('s'):		// dolu
					smazatKurzor(xn,yn);
					posunKurzoru(0,1);
					break;
                case('w'):		// nahoru
					smazatKurzor(xn,yn);
					posunKurzoru(0,-1);
					//proPresahHranice();
					break;putchar('f');
                case('q'):      //na SeveroZápad
                    smazatKurzor(xn,yn);
					posunKurzoru(-1,-1);
                    break;
                case('e'):      //SV
                    smazatKurzor(xn,yn);
                    posunKurzoru(1,-1);
                    break;
                case('y'):      //JZ
                    smazatKurzor(xn,yn);
                    posunKurzoru(-1,1);
                    break;
                case('c'):      //JV
                    smazatKurzor(xn,yn);
                    posunKurzoru(1,1);
                    break;

				case('+'): //inkrementace Kurzorového znaku
                    if((unsigned char) kresliciZnak>=254){
                        if(cyklPohyb)kresliciZnak=33;}
                    else
                        kresliciZnak++;
                    vypsatNapovedu(xn,yn,1,0); //znovu se vypíše Nápověda, aby bylo vidět, že se znak změnil
                    break;
                case('-'): //dekrementace Kurzorového znaku
                    if((unsigned char)kresliciZnak<=33){
                        if(cyklPohyb)kresliciZnak=254;}
                    else
                        kresliciZnak--;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('/'):  //zadat Kurzorový znak manuálně
                    kresliciZnak = zadatNovyKresliciZnak(0, Y_OTAZEK, POCET_K_VYBILENI);
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    vypsatNapovedu(xn,yn,1,0);
                    break;

                case('1'): //Kreslící režim...guma
                    editRezim=-1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('3'): //Kreslící režim...editace
                    editRezim=1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('2'): //Kreslící režim...pohyb
                    editRezim=0;
                    vypsatNapovedu(xn,yn,1,0);
                    break;

                case('p'): //Přepnutí Reřimu cyklického pohybu
                    cyklPohyb = !cyklPohyb;
                    vypsatNapovedu(xn,yn,1,0);
                    break;

                case('7'):  //smazat plátno
                    smazat(0);
                    break;

                case('u'):  //uložit obrázek do souboru
                    ulozitDoSouboru(0, Y_OTAZEK);
                    break;
                case('@'):  //Rychle uložit do souboru
                    //printf("dsd"); getchar();
                    celaProceduraRychlehoUlozeni(0, Y_OTAZEK);
                    break;

                case('l'):  //načíst soubor
                    if((t=celaProceduraProNacteniSouboru(0,Y_OTAZEK,0,0))==0){
                        vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                        printf("Nacteni souboru neporobehlo uspesne!");
                    }
                    else{
                        prekreslit(1);
                        vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                        if(t>0) printf("Soubor byl uspesne nacten!");
                        else    printf("Soubor byl nacten ale doslo k orezu!");
                    }
                    break;

                case('4'):  //nakreslit rychlý čtverec o předem daném poloměru, dutosti atd.
                    fillSquare(xn,yn,polomerRychlehoCtverce,(editRezim<0)?' ':kresliciZnak/*pokud je Kreslící režim gumy, vykreslí to samé mezery*/,duteCtverce);
                    break;
                case('8'):  //nakreslit čtverec s poloměrem dle výběru
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    gotoxy(0,Y_OTAZEK);
                    printf("Zadejte polomer ctverce: ");
                    scanf("%d", &t);
                    while(getchar()!='\n');
                    if(t<0)t=-t;
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    fillSquare(xn,yn,t,(editRezim<0)?' ':kresliciZnak/*pokud je Kreslící režim gumy, vykreslí to samé mezery*/,duteCtverce);
                    break;

                case ('6'): //nakreslit rychlé rovnoběžné čáry
                    nakreslitCary((editRezim<0)?' ':kresliciZnak, delkaRychleCary);
                    break;
                case('9'):  //nakreslit rovnoběžné čáry s délkou dle výběru
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    gotoxy(0,Y_OTAZEK);
                    printf("Zadejte delku cary: ");
                    scanf("%d", &t);
                    while(getchar()!='\n');
                    if(t<0)t=-t;
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    nakreslitCary((editRezim<0)?' ':kresliciZnak/*pokud je Kreslící režim gumy, vykreslí to samé mezery*/, t);
                    break;

                case(','):  //přepnout Dutost čtverců
                    duteCtverce = !duteCtverce;
                    vypsatNapovedu(xn,yn,1,0);
                    break;

                case('j'): //dekrementuje polomer rychleho ctverce
                    if(polomerRychlehoCtverce>0){polomerRychlehoCtverce--;}
                    else if(polomerRychlehoCtverce<=0&&cyklPohyb) polomerRychlehoCtverce=MAX_RYCHLA_DELKA;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('k'): //inkrementuje polomer rychleho ctverce
                    if(polomerRychlehoCtverce<MAX_RYCHLA_DELKA){polomerRychlehoCtverce++;}
                    else if(polomerRychlehoCtverce>=MAX_RYCHLA_DELKA&&cyklPohyb) polomerRychlehoCtverce=0;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('n'): //dekrementuje delku rychle cary
                    if(delkaRychleCary>1) {delkaRychleCary--;}
                    else if(delkaRychleCary<=1&&cyklPohyb) delkaRychleCary=MAX_RYCHLA_DELKA;
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('m'): //inkrementuje delku rychle cary
                    if(delkaRychleCary<MAX_RYCHLA_DELKA) {delkaRychleCary++;}
                    else if(delkaRychleCary>=MAX_RYCHLA_DELKA&&cyklPohyb) delkaRychleCary=1;
                    vypsatNapovedu(xn,yn,1,0);
                    break;

                case('z'): //zobrazí/zahladí takové ty zářezy na polovině obruby rámčku kolem plátna
                    if(vyznacenyVyznBody)vykreslitOhraniceni(1,1,X_MAX, Y_MAX);
                    else vyznacitVyznMiry();
                    break;

                case(' '): //důkladně promaže Dialogovou řádku pod Dolní nápovědou
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI*4);
                    break;
                case(';'): //smaže celou obrazovku a vykreslí ji odznova
                    prekreslit(1);
                    break;

                case('t'):  //přečte textový řetězec a umístí ho na obrazovku
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    printf("Zadejte retezec (max. %d znaku), ktery chcete umistit do obrazce: ", MAX_STRING);
                    for(t=0;t>=0;){ //čte řetězec manuálně, znak po znaku
                            for(t=0;t<MAX_STRING;t++){
                                text[t] = getchar();
                                if(text[t]=='\n'){
                                    text[t]='\0';
                                    t=-3;
                                    break;
                                }
                            }
                            if(t>=0){
                                while(text[t]!='\n')text[t]=getchar();
                                vybilit(0,Y_OTAZEK,POCET_K_VYBILENI*3);
                                printf("Chyba! Zadejte platny retezec: ");
                            }
                    }
                    vybilit(0,Y_OTAZEK,POCET_K_VYBILENI);
                    umistitText(xn,yn,text);
                    break;

                case('0'):  //přepne Natočení (pro textové řetězce, čáry, apod.)
                        if(natoceni>=7)natoceni = 0;
                        else natoceni++;
                        vypsatNapovedu(xn,yn,1,0);
                        break;

                case('5'): //prohození funkcí mezi klávesami
                    prohoditKlavesy(0,Y_OTAZEK);
                    break;
                case('h'):  //vrátit výchozí rozložení kláves
                    if(anoNe(0,Y_OTAZEK,"Jste si jisti, ze chcete nastavit ovladani zpet na vychozi hodnoty?",1)){
                        inicializovatPoleNovychZnaku();
                        ZN_JIDLA = DEFAULT_ZN_JIDLA;       ZN_HLAVA = DEFAULT_ZN__HLAVA;      ZN_TELO = DEFAULT_ZN__TELO; //nastaví i výchozí znaky pro tělo hada
                        ulozitConfig(); //uloží konfigurační soubor
                        vypsatNapovedu(xn,yn,0,1);
                    }
                    break;

				case('*'): //ukončit program
				    if(!anoNe(0,Y_OTAZEK,"Opravdu si prejete ukoncit program?",0)) c = ' '; //pokud se uživatel rozhodl, že nakonec program přeci jen ještě nechce vypnout, nahradí to přečtený znak něčím neškodným
				    gotoxy(0,Y_OTAZEK);
					break;
                case('.'): //kreslící znak se změní na to, na čem se právě nalézá kurzor
                    (zn[xn][yn] != ' ') && (kresliciZnak = zn[xn][yn]);
                    vypsatNapovedu(xn,yn,1,0);
                    break;
                case('v'): //Výběr a následné kopírování oblasti v plátnu
                    kopirovat();
                    break;
                case('='):  //inkrementace rychlosti pohybu
                    rychlostPohybu++;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case(')'):  //dekrementace rychlosti pohybu
                    rychlostPohybu--;
                    vypsatNapovedu(xn, yn, 1, 0);
                    break;
                case('o'):  //Ořez plátna
                    orez();
                    break;
                case('\\'): //spustí Hada
                    had();
                    break;
                case('|'):  //vstup do Nastavení
                    hadiNastaveni();
                    break;
				default:
                    continue;
			}

			/*vykreslí kurzor s příslušným znakem na pozici, kde má být*/
			gotoxy(xn,yn);
			putchar(kresliciZnak);
			gotoxy(xn,yn);
		}
	} while(c!='*'); //byla-li přečtena hvězdička, program se ukončí
}

/*vykreslí úvodní nabídku tak, aby odpovídala aktuálně vybrané položce*/
uvodZmenaVyberu( int x, int y, int vyber){
    gotoxy(x,y);
    if(vyber==0){ //vybrána 1. položka
        printf("%cVYTVORIT NOVY SOUBOR               %c",186,186);
        gotoxy(x,++y);
        printf("%cNacist existujici soubor           %c",186,186);
        gotoxy(x,++y);
        printf("%cKonec                              %c",186,186);
        xn = x+1;yn=y-2;
    }
    if(vyber==1){ //vybrána 2. položka
        printf("%cVytvorit novy soubor               %c",186,186);
        gotoxy(x,++y);
        printf("%cNACIST EXISTUJICI SOUBOR           %c",186,186);
        gotoxy(x,++y);
        printf("%cKonec                              %c",186,186);
        xn = x+1;yn=y-1;
    }
    if(vyber==2){ //vybrána 3. položka
        printf("%cVytvorit novy soubor               %c",186,186);
        gotoxy(x,++y);
        printf("%cNacist existujici soubor           %c",186,186);
        gotoxy(x,++y);
        printf("%cKONEC                              %c",186,186);
        xn = x+1;yn=y;
    }
}

/*úvodní nabídka*/
uvod(){
    char c;
    int t,x=0,y=0, novyS = 0, yv;

    system("cls"); //vyčistí obrazovku
    gotoxy(x,y);
    putchar(201);//znak ohraničení
    for(t=0;t<35;t++) putchar(205);
    putchar(187);
    gotoxy(x,++y);
    printf("%cVitejte v Edytoru Textove Graf%cky  %c",186,251,186);
    gotoxy(x,++y);
    putchar(204);
    for(t=0;t<35;t++) putchar(205);
    putchar(185);
    gotoxy(x,++y);
    putchar(186); for(t=0;t<35;t++) putchar(' '); putchar(186);
    gotoxy(x,++y);yv = y;
    uvodZmenaVyberu(x,yv,novyS);
    y+=2;
    gotoxy(x,++y);
    putchar(200);
    for(t=0;t<35;t++) putchar(205);
    putchar(188);
    gotoxy(x,++y);
    vrn("ws ","(Pro pohyb pouzijte klavesy '_' a '_'. _ potvrdi vyber.)\n\nVytvoril Jakub Hronik");

    do{
        if(kbhit()){
            c=prelozitZadanyZnak(malePismeno(getch()));
            switch(c){
                case('w'): //pohyb nahoru
                    if(novyS>0) novyS--;
                    uvodZmenaVyberu(x,yv,novyS);
                    break;
                case('s'): //pohyb dolů
                    if(novyS<2) novyS++;
                    uvodZmenaVyberu(x,yv,novyS);
                    break;
                case(' '): //potvrzení vůběru
                    system("cls");
                    //bylo zvoleno vytvoření nového souboru
                    if(novyS==0) zmenitRozliseni(0,0,1), behProgramu(0);
                    //bylo zvoleno načtení souboru
                    if(novyS==1){system("cls"); gotoxy(0,0); if((t=celaProceduraProNacteniSouboru(0,0,4,1))==0){gotoxy(0,0);printf("Nacitani souboru probehlo neuspesne!                              \n*PRESS ENTER*");while(getchar()!='\n');getchar();uvod();break;} if(t<0)naZacatkuNacteniSOrezem=1; behProgramu(1);}
                    //až toto doběhne, program se ukončí, jelikož byl přečten mezerník
                    //pokud není, co by doběhlo (vybrána 3. položka - "Ukončit program), program se ukončí rovnou
                    break;
                default:
                    continue;
            }
        }
        gotoxy(xn,yn);
    }while(c!=' ');

}


/*pouze načte soubor, který převzala jako argument a otevře ho, či přejde do úvodní nabídky*/
/*velmi ošklivě zneužívá operátorů čárky a zkráceného vyhodnocování logických výrazů, aby mohla celá sestávat z jediného příkazu*/
main(int argc, char *argv[])
{
    srand(time(NULL)), //inicializace generátoru náhodných čísel
    sprintf(nazevProgramu, "EyTG%c - Edytor Textov%c Graf%cky 1.7.2    ", 251,130,251),  //připraví řetězec s názvem programu
    strcpy(programTitle, nazevProgramu),
    SetConsoleTitle(programTitle),  //nastaví titulek okna s programem
    prizpusobitVelikostOknaRozliseni(80,15), //nastaví okno tak, aby se v něm hezky výjímala úvodní nabídka

    inicializovatPoleNovychZnaku(),//inicializace systémových proměnných
    inicializovatHada(),
    nacistConfig(),

    (((argc > 1) && (((nastavitJmenoAktualnihoSouboru(argv[1]),(nacistSoubor(fopen(argv[1], "r"), 1, 0, 0)<0))&&(naZacatkuNacteniSOrezem=1)), behProgramu(1)))
	|| uvod()); //zjistí, zda byl programu při spuštění předán druhý argument a případně načte soubor, který mu odpovídá; jinak přejde do úvodní nabídky
}
