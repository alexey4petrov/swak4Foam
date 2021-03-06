 
%{                                          /* -*- C++ -*- */
#include "SubsetValueExpressionDriverYY.H"
#include <errno.h>
%}

%s setname
%x needsIntegerParameter

%option noyywrap nounput batch debug 
%option stack
%option prefix="parserSubset"

id      [[:alpha:]_][[:alnum:]_]*
setid   [[:alpha:]_][[:alnum:]_-]*
int     [[:digit:]]+

exponent_part              [eE][-+]?[[:digit:]]+
fractional_constant        (([[:digit:]]*"."[[:digit:]]+)|([[:digit:]]+".")|([[:digit:]]+))
float                      ((({fractional_constant}{exponent_part}?)|([[:digit:]]+{exponent_part}))|0)

%{
# define YY_USER_ACTION yylloc->columns (yyleng);
%}
%%

%{
    yylloc->step ();
%}

<INITIAL,setname,needsIntegerParameter>[ \t]+             yylloc->step ();
[\n]+                yylloc->lines (yyleng); yylloc->step ();

<INITIAL,setname>[-+*/%(),&^<>!?:.]               return yytext[0];

<needsIntegerParameter>[(] return yytext[0];
<needsIntegerParameter>[)] { BEGIN(INITIAL); return yytext[0]; }

%{
    typedef parserSubset::SubsetValueExpressionParser::token token;
%}

&&                   return token::TOKEN_AND;
\|\|                 return token::TOKEN_OR;
==                   return token::TOKEN_EQ;
!=                   return token::TOKEN_NEQ;
\<=                   return token::TOKEN_LEQ;
\>=                   return token::TOKEN_GEQ;

pow                   return token::TOKEN_pow;
exp                   return token::TOKEN_exp;
log                   return token::TOKEN_log;
mag                   return token::TOKEN_mag;
sin                   return token::TOKEN_sin;
cos                   return token::TOKEN_cos;
tan                   return token::TOKEN_tan;
min                   return token::TOKEN_min;
max                   return token::TOKEN_max;
average               return token::TOKEN_average;
sum                   return token::TOKEN_sum;
sqr                   return token::TOKEN_sqr;
sqrt                  return token::TOKEN_sqrt;

log10                 return token::TOKEN_log10;
asin                  return token::TOKEN_asin;
acos                  return token::TOKEN_acos;
atan                  return token::TOKEN_atan;
sinh                  return token::TOKEN_sinh;
cosh                  return token::TOKEN_cosh;
tanh                  return token::TOKEN_tanh;
asinh                 return token::TOKEN_asinh;
acosh                 return token::TOKEN_acosh;
atanh                 return token::TOKEN_atanh;
erf                   return token::TOKEN_erf;
erfc                  return token::TOKEN_erfc;
lgamma                return token::TOKEN_lgamma;
besselJ0              return token::TOKEN_besselJ0;
besselJ1              return token::TOKEN_besselJ1;
besselY0              return token::TOKEN_besselY0;
besselY1              return token::TOKEN_besselY1;

sign                  return token::TOKEN_sign;
positive              return token::TOKEN_pos;
negative              return token::TOKEN_neg;

pi                    return token::TOKEN_pi;
pos                   return token::TOKEN_position;
area                  return token::TOKEN_area;
vol                   return token::TOKEN_volume;
pts                   return token::TOKEN_points;
Sf                    return token::TOKEN_Sf;
normal                return token::TOKEN_normal;
rand                  { BEGIN(needsIntegerParameter); return token::TOKEN_rand; }
randFixed             { BEGIN(needsIntegerParameter); return token::TOKEN_randFixed; }
id                    return token::TOKEN_id;
cpu                   return token::TOKEN_cpu;
flip                  return token::TOKEN_flip;
randNormal            { BEGIN(needsIntegerParameter); return token::TOKEN_randNormal; }
randNormalFixed       { BEGIN(needsIntegerParameter); return token::TOKEN_randNormalFixed; }

deltaT                return token::TOKEN_deltaT;
time                  return token::TOKEN_time;

vector                 return token::TOKEN_VECTOR;
tensor                 return token::TOKEN_TENSOR;
symmTensor             return token::TOKEN_SYMM_TENSOR;
sphericalTensor        return token::TOKEN_SPHERICAL_TENSOR;

true                   return token::TOKEN_TRUE;
false                  return token::TOKEN_FALSE;

toPoint                 return token::TOKEN_toPoint;
toFace                  return token::TOKEN_toFace;

transpose              return token::TOKEN_transpose;
diag                   return token::TOKEN_diag;
tr                     return token::TOKEN_tr;
dev                    return token::TOKEN_dev;
symm                   return token::TOKEN_symm;
skew                   return token::TOKEN_skew;
det                    return token::TOKEN_det;
cof                    return token::TOKEN_cof;
inv                    return token::TOKEN_inv;


{float}                {
                       errno = 0;
                       yylval->val = atof(yytext);
                       return token::TOKEN_NUM;
                     }

<needsIntegerParameter>{int}                {
                       errno = 0;
                       yylval->integer = atoi(yytext);
                       return token::TOKEN_INT;
                     }

[xyz]                return yytext[0];

<INITIAL>{id}                 {
    Foam::string *ptr=new Foam::string (yytext);
    if(driver.isLine(*ptr)) {
        yylval->name = ptr; return token::TOKEN_LINE;
    } else if(driver.isLookup(*ptr)) {
        yylval->name = ptr; return token::TOKEN_LOOKUP;
    } else if(driver.is<Foam::vector>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_VID;
    } else if(driver.is<Foam::tensor>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_TID;
    } else if(driver.is<Foam::symmTensor>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_YID;
    } else if(driver.is<Foam::sphericalTensor>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_HID;
    } else if(driver.is<Foam::scalar>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_SID;
        //    } else if(driver.is<Foam::bool>(*ptr)) {
        //        yylval->name = ptr; return token::TOKEN_LID;
    } else if(driver.is<Foam::vector>(*ptr,true)) {
        yylval->name = ptr; return token::TOKEN_PVID;
    } else if(driver.is<Foam::tensor>(*ptr,true)) {
        yylval->name = ptr; return token::TOKEN_PTID;
    } else if(driver.is<Foam::symmTensor>(*ptr,true)) {
        yylval->name = ptr; return token::TOKEN_PYID;
    } else if(driver.is<Foam::sphericalTensor>(*ptr,true)) {
        yylval->name = ptr; return token::TOKEN_PHID;
    } else if(driver.is<Foam::scalar>(*ptr,true)) {
        yylval->name = ptr; return token::TOKEN_PSID;
        //    } else if(driver.is<Foam::bool>(*ptr,true)) {
        //        yylval->name = ptr; return token::TOKEN_PLID;
    } else {
        driver.error (*yylloc, "field "+*ptr+" not existing or of wrong type");
    }
                     }

.                    driver.error (*yylloc, "invalid character");
<needsIntegerParameter>.                    driver.error (*yylloc, "invalid character when only an integer parameter is expected");


%%

YY_BUFFER_STATE bufferSubset;

void SubsetValueExpressionDriver::scan_begin ()
{
    yy_flex_debug = trace_scanning_;
    bufferSubset=yy_scan_string(content_.c_str());
    

//    if (!(yyin = fopen (file.c_str (), "r")))
//        error (std::string ("cannot open ") + file);
}

void SubsetValueExpressionDriver::scan_end ()
{
//	    fclose (yyin);
    yy_delete_buffer(bufferSubset);
}
