#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 14 "parse.y"
#include <stdlib.h>
#include "do_sql.h"


#define YYSTYPE int


/* While we parse the SET and WHERE clause we need someplace
 * to temporarily store the type of relation */
static int  whrrelat;

/* We don't want to pass pointers to allocated memory on the */
/* yacc stack, since the memory might not be freed when an */
/* error is detected.  Instead, we allocate the memory and */
/* put the pointer into the following table where it is easy */
/* free on error.  */
char *parsestr[MXPARSESTR];

static int   n;            /* temp/scratch integer */

extern struct Sql_Cmd cmd; /* encoded SQL command (a global) */
extern char *yytext;
extern int   yyleng;
extern void  yyerror(char *);
extern int   yylex();
#line 43 "parse.tab.c"
#define YYERRCODE 256
#define SQLBEGIN 257
#define SQLCOMMIT 258
#define SELECT 259
#define UPDATE 260
#define FROM 261
#define WHERE 262
#define NAME 263
#define STRING 264
#define INTEGER 265
#define REALNUM 266
#define LIMIT 267
#define OFFSET 268
#define SET 269
#define EQ 270
#define NE 271
#define GT 272
#define LT 273
#define GE 274
#define LE 275
#define AND 276
const short yylhs[] = {                                        -1,
    0,    0,    0,    0,    0,    0,    1,    1,    2,    2,
    3,    3,    6,    6,    7,    7,    8,    8,   10,   10,
   10,   11,   11,   11,   11,   11,   11,    9,    9,    9,
    4,    4,   13,   13,   12,   12,   12,   12,    5,    5,
};
const short yylen[] = {                                         2,
    0,    1,    1,    1,    1,    1,    2,    1,    2,    1,
    7,    6,    1,    3,    1,    3,    0,    2,    3,    3,
    3,    1,    1,    1,    1,    1,    1,    0,    2,    4,
    7,    6,    3,    3,    1,    1,    1,    1,    5,    4,
};
const short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    2,    3,    4,    5,    6,
    7,    9,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   14,    0,    0,   39,    0,    0,    0,    0,
    0,    0,   16,    0,    0,    0,    0,    0,   35,   36,
   37,   38,   34,   33,    0,   22,   23,   24,   25,   26,
   27,    0,    0,    0,    0,   11,   31,   21,   19,   20,
    0,   30,
};
const short yydgoto[] = {                                       5,
    6,    7,    8,    9,   10,   14,   22,   29,   38,   36,
   52,   43,   25,
};
const short yysindex[] = {                                   -240,
  -34,  -33, -236, -235,    0,    0,    0,    0,    0,    0,
    0,    0,  -10,  -37, -238,   -9, -230, -229, -228,  -23,
   -8, -225,    0, -231,  -39,    0, -223,  -36, -226, -242,
 -228, -226,    0, -259,  -36, -234, -222,  -15,    0,    0,
    0,    0,    0,    0,  -14,    0,    0,    0,    0,    0,
    0, -242,  -41,  -36, -221,    0,    0,    0,    0,    0,
 -219,    0,
};
const short yyrindex[] = {                                     48,
   49,   50,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -35,    0,    0,    0,    0,    0,    0,   51,
    1,    2,    0,    0,    2,    0,    0,    0,    6,    0,
    0,    6,    0,    0,    0,    3,    0,   52,    0,    0,
    0,    0,    0,    0,   53,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    8,    0,    0,    0,    0,    0,
    0,    0,
};
const short yygindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,   29,   23,  -25,
    0,    4,   26,
};
#define YYTABLESIZE 270
const short yytable[] = {                                      59,
   15,   17,   18,   35,   31,   28,   18,   29,   13,   53,
   46,   47,   48,   49,   50,   51,    1,    2,    3,    4,
   39,   40,   41,   42,   11,   12,   13,   15,   60,   16,
   19,   20,   21,   23,   24,   26,   28,   27,   30,   33,
   37,   54,   55,   56,   57,   62,   61,    1,    8,   10,
   40,   12,   32,   32,   45,   58,   44,    0,    0,   15,
   17,   18,    0,    0,   28,    0,   29,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   28,   17,    0,   13,   34,    0,    0,    0,
    0,    0,    0,    0,   54,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   15,    0,    0,    0,    0,   15,   17,   18,
};
const short yycheck[] = {                                      41,
    0,    0,    0,   40,   44,    0,   44,    0,   44,   35,
  270,  271,  272,  273,  274,  275,  257,  258,  259,  260,
  263,  264,  265,  266,   59,   59,  263,  263,   54,   40,
  269,   41,  263,  263,  263,   59,  262,   46,  270,  263,
  267,  276,  265,   59,   59,  265,  268,    0,    0,    0,
    0,    0,    0,   25,   32,   52,   31,   -1,   -1,   59,
   59,   59,   -1,   -1,   59,   -1,   59,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  262,  261,   -1,  261,  263,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  276,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  262,   -1,   -1,   -1,   -1,  267,  267,  267,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 276
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,"'.'",0,0,0,0,0,0,0,0,0,0,0,0,"';'",0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"SQLBEGIN",
"SQLCOMMIT","SELECT","UPDATE","FROM","WHERE","NAME","STRING","INTEGER",
"REALNUM","LIMIT","OFFSET","SET","EQ","NE","GT","LT","GE","LE","AND",
};
const char * const yyrule[] = {
"$accept : command",
"command :",
"command : begin_statement",
"command : commit_statement",
"command : select_statement",
"command : update_statement",
"command : function_call",
"begin_statement : SQLBEGIN ';'",
"begin_statement : SQLBEGIN",
"commit_statement : SQLCOMMIT ';'",
"commit_statement : SQLCOMMIT",
"select_statement : SELECT column_list FROM table_name where_clause limit_clause ';'",
"select_statement : SELECT column_list FROM table_name where_clause limit_clause",
"column_list : NAME",
"column_list : column_list ',' NAME",
"table_name : NAME",
"table_name : NAME '.' NAME",
"where_clause :",
"where_clause : WHERE test_condition",
"test_condition : '(' test_condition ')'",
"test_condition : test_condition AND test_condition",
"test_condition : NAME relation literal",
"relation : EQ",
"relation : NE",
"relation : GT",
"relation : LT",
"relation : GE",
"relation : LE",
"limit_clause :",
"limit_clause : LIMIT INTEGER",
"limit_clause : LIMIT INTEGER OFFSET INTEGER",
"update_statement : UPDATE NAME SET set_list where_clause limit_clause ';'",
"update_statement : UPDATE NAME SET set_list where_clause limit_clause",
"set_list : set_list ',' set_list",
"set_list : NAME EQ literal",
"literal : NAME",
"literal : STRING",
"literal : INTEGER",
"literal : REALNUM",
"function_call : SELECT NAME '(' ')' ';'",
"function_call : SELECT NAME '(' ')'",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 250 "parse.y"


/***************************************************************
 * freesql(): - Free allocated memory from previous command.
 *
 * Input:        None.
 * Output:       None.
 * Affects:      Frees memory from last command
 ***************************************************************/


/***************************************************************
 * dosql_init(): - Set up data structures prior to parse of
 * an SQL command.
 *
 * Input:        None.
 * Output:       None.
 * Affects:      structure cmd is initialized
 ***************************************************************/
void dosql_init() {
    int   i;

    for (i=0; i<NCMDCOLS; i++) {
        if (cmd.cols[i])
            free(cmd.cols[i]);
        if (cmd.updvals[i])
            free(cmd.updvals[i]); /* values for column updates */
        if (cmd.whrcols[i])
            free(cmd.whrcols[i]); /* cols in where */
        if (cmd.whrvals[i])
            free(cmd.whrvals[i]); /* values in where clause */
        cmd.cols[i]    = (char *) 0;
        cmd.updvals[i] = (char *) 0;
        cmd.whrcols[i] = (char *) 0;
        cmd.whrvals[i] = (char *) 0;
    }
    if (cmd.tbl);
        free(cmd.tbl);
    for (i=0; i<MXPARSESTR; i++) {
        if (parsestr[i]) {
            free(parsestr[i]);
            parsestr[i] = (char *) NULL;
        }
    }

    cmd.tbl = (char *) 0;
    cmd.ptbl = (TBLDEF *) 0;
    cmd.ncols    = 0;
    cmd.nwhrcols = 0;
    cmd.limit  = 1<<30;  /* no real limit */
    cmd.offset = 0;
    cmd.err    = 0;
}

void yyerror(char *s)
{
    send_error(LOC, E_BADPARSE);
    return;
}


#line 324 "parse.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 70 "parse.y"
{ YYABORT; }
break;
case 7:
#line 81 "parse.y"
{	cmd.command = RTA_BEGIN;
			YYACCEPT;
		}
break;
case 8:
#line 85 "parse.y"
{	cmd.command = RTA_BEGIN;
			YYACCEPT;
		}
break;
case 9:
#line 92 "parse.y"
{	cmd.command = RTA_COMMIT;
			YYACCEPT;
		}
break;
case 10:
#line 96 "parse.y"
{	cmd.command = RTA_COMMIT;
			YYACCEPT;
		}
break;
case 11:
#line 103 "parse.y"
{	cmd.command = RTA_SELECT;
			YYACCEPT;
		}
break;
case 12:
#line 107 "parse.y"
{	cmd.command = RTA_SELECT;
			YYACCEPT;
		}
break;
case 13:
#line 114 "parse.y"
{	cmd.cols[cmd.ncols] = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
			cmd.ncols++;
		}
break;
case 14:
#line 119 "parse.y"
{	cmd.cols[cmd.ncols] = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
			cmd.ncols++;
			if (cmd.ncols > NCMDCOLS) {
				/* too many columns in list */
				send_error(LOC, E_BADPARSE);
			}
		}
break;
case 15:
#line 131 "parse.y"
{
			cmd.tbl = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
		}
break;
case 16:
#line 136 "parse.y"
{
			cmd.tbl = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
		}
break;
case 21:
#line 153 "parse.y"
{	n = cmd.nwhrcols;
			cmd.whrcols[n] = parsestr[(int) yyvsp[-2]];
			parsestr[(int) yyvsp[-2]] = (char *) NULL;
			cmd.whrrel[n] = whrrelat;
			cmd.whrvals[n] = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
			cmd.nwhrcols++;
			if (cmd.nwhrcols > NCMDCOLS) {
				/* too many columns in list */
				send_error(LOC, E_BADPARSE);
			}
		}
break;
case 22:
#line 169 "parse.y"
{	whrrelat = RTA_EQ; }
break;
case 23:
#line 170 "parse.y"
{	whrrelat = RTA_NE; }
break;
case 24:
#line 171 "parse.y"
{	whrrelat = RTA_GT; }
break;
case 25:
#line 172 "parse.y"
{	whrrelat = RTA_LT; }
break;
case 26:
#line 173 "parse.y"
{	whrrelat = RTA_GE; }
break;
case 27:
#line 174 "parse.y"
{	whrrelat = RTA_LE; }
break;
case 29:
#line 181 "parse.y"
{	cmd.limit  = atoi(parsestr[(int) yyvsp[0]]);
            free(parsestr[(int) yyvsp[0]]);
			parsestr[(int) yyvsp[0]] = (char *) NULL;
		}
break;
case 30:
#line 186 "parse.y"
{	cmd.limit  = atoi(parsestr[(int) yyvsp[-2]]);
            free(parsestr[(int) yyvsp[-2]]);
			parsestr[(int) yyvsp[-2]] = (char *) NULL;
			cmd.offset = atoi(parsestr[(int) yyvsp[0]]);
            free(parsestr[(int) yyvsp[0]]);
			parsestr[(int) yyvsp[0]] = (char *) NULL;
		}
break;
case 31:
#line 198 "parse.y"
{	cmd.command = RTA_UPDATE;
			cmd.tbl     = parsestr[(int) yyvsp[-5]];
			parsestr[(int) yyvsp[-5]] = (char *) NULL;
		}
break;
case 32:
#line 203 "parse.y"
{	cmd.command = RTA_UPDATE;
			cmd.tbl     = parsestr[(int) yyvsp[-4]];
			parsestr[(int) yyvsp[-4]] = (char *) NULL;
		}
break;
case 34:
#line 212 "parse.y"
{	n = cmd.ncols;
			cmd.cols[n] = parsestr[(int) yyvsp[-2]];
			parsestr[(int) yyvsp[-2]] = (char *) NULL;
			cmd.updvals[n] = parsestr[(int) yyvsp[0]];
			parsestr[(int) yyvsp[0]] = (char *) NULL;
			cmd.ncols++;
			if (cmd.ncols > NCMDCOLS) {
				/* too many columns in list */
				send_error(LOC, E_BADPARSE);
			}
		}
break;
case 39:
#line 235 "parse.y"
{	cmd.command = RTA_CALL;
			cmd.tbl = parsestr[(int) yyvsp[-3]];
			parsestr[(int) yyvsp[-3]] = (char *) NULL;
			YYACCEPT;
		}
break;
case 40:
#line 241 "parse.y"
{	cmd.command = RTA_CALL;
			cmd.tbl = parsestr[(int) yyvsp[-2]];
			parsestr[(int) yyvsp[-2]] = (char *) NULL;
			YYACCEPT;
		}
break;
#line 691 "parse.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
