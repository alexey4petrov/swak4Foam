/*  -*- C++ -*- */

%skeleton "lalr1.cc"
/* %require "2.1a" */
%defines
%define "parser_class_name" "FaFieldValueExpressionParser"

%{
#include <areaFields.H>
#include <edgeFields.H>
#include <facGrad.H>
    // #include <facCurl.H>
    //#include <facMagSqrGradGrad.H>
#include <facLnGrad.H>
#include <facDiv.H>
#include <facEdgeIntegrate.H>
    //#include <facReconstruct.H>
#include <facAverage.H>
#include <edgeInterpolate.H>
#include <facLaplacian.H>

#include <faMatrix.H>

#include <functional>
#include <cmath>

    namespace Foam {
        class FaFieldValueExpressionDriver;
    }

    using Foam::FaFieldValueExpressionDriver;
%}

%name-prefix="parserFaField"

%parse-param { FaFieldValueExpressionDriver& driver }
%lex-param { FaFieldValueExpressionDriver& driver }

%locations
%initial-action
{
	     // Initialize the initial location.
	     //     @$.begin.filename = @$.end.filename = &driver.file;
};

%debug
%error-verbose

%union
{
    Foam::scalar val;
    Foam::vector *vec;
    Foam::string *name;
    Foam::string *vname;
    Foam::string *fsname;
    Foam::string *fvname;
    Foam::areaVectorField *vfield;
    Foam::areaScalarField *sfield;
    Foam::edgeScalarField *fsfield;
    Foam::edgeVectorField *fvfield;
};

%{
#include "FaFieldValueExpressionDriverYY.H"
#include "FaFieldValueExpressionDriverLogicalTemplates.H"
%}

%token <name>   TOKEN_LINE  "timeline"
%token <name>   TOKEN_SID   "scalarID"
%token <vname>  TOKEN_VID   "vectorID"
%token <fsname> TOKEN_FSID  "faceScalarID"
%token <fvname> TOKEN_FVID  "faceVectorID"
%token <val>    TOKEN_NUM   "number"
%token <vec>    TOKEN_VEC   "vector"
%type  <val>    scalar      "sexpression"  
%type  <sfield>    exp        "expression"
%type  <sfield>    lexp       "lexpression"
%type  <fsfield>   flexp       "flexpression"
%type  <vfield>    vexp       "vexpression"
%type  <fsfield>   fsexp      "fsexpression"
%type  <fvfield>   fvexp      "fvexpression"
%type  <vfield>    vector     
%type  <fvfield>    fvector     

%token TOKEN_VECTOR

%token TOKEN_TRUE
%token TOKEN_FALSE

%token TOKEN_pi
%token TOKEN_rand
%token TOKEN_id
%token TOKEN_cpu
%token TOKEN_randNormal
%token TOKEN_position
%token TOKEN_fposition
%token TOKEN_fprojection
%token TOKEN_face
%token TOKEN_length
%token TOKEN_area
%token TOKEN_rdist

%token TOKEN_div
%token TOKEN_grad
%token TOKEN_lnGrad
%token TOKEN_laplacian

%token TOKEN_integrate
%token TOKEN_surfSum
%token TOKEN_interpolate
%token TOKEN_faceAverage

%token TOKEN_surf

%token TOKEN_deltaT
%token TOKEN_time

%token TOKEN_pow
%token TOKEN_log
%token TOKEN_exp
%token TOKEN_mag
%token TOKEN_sin
%token TOKEN_cos
%token TOKEN_tan
%token TOKEN_min
%token TOKEN_max
%token TOKEN_sum
%token TOKEN_average
%token TOKEN_sqr
%token TOKEN_sqrt

%token TOKEN_log10
%token TOKEN_asin
%token TOKEN_acos
%token TOKEN_atan
%token TOKEN_sinh
%token TOKEN_cosh
%token TOKEN_tanh
%token TOKEN_asinh
%token TOKEN_acosh
%token TOKEN_atanh
%token TOKEN_erf
%token TOKEN_erfc
%token TOKEN_lgamma
%token TOKEN_besselJ0
%token TOKEN_besselJ1
%token TOKEN_besselY0
%token TOKEN_besselY1

%token TOKEN_sign
%token TOKEN_pos
%token TOKEN_neg

%left '?' ':'
%left TOKEN_OR
%left TOKEN_AND
%left TOKEN_EQ TOKEN_NEQ
%left TOKEN_LEQ TOKEN_GEQ '<' '>'
%left '-' '+'
%left '%' '*' '/' '&' '^'
%left TOKEN_NEG TOKEN_NOT
// %right '^'
%left '.'

%printer             { debug_stream () << *$$; } "scalarID" "vectorID" "faceScalarID" "faceVectorID" 
%printer             { Foam::OStringStream buff; buff << *$$; debug_stream () << buff.str().c_str(); } "vector"
%destructor          { delete $$; } "timeline" "scalarID" "faceScalarID" "faceVectorID" "vectorID" "vector" "expression" "vexpression" "fsexpression" "fvexpression" "lexpression" "flexpression"  
%printer             { debug_stream () << $$; } "number"  "sexpression"
%printer             { debug_stream () << $$->name().c_str(); } "expression"  "vexpression" "lexpression" "flexpression" "fsexpression" "fvexpression"


%%
%start unit;

unit:   exp                     { driver.setScalarResult($1);  }
        | vexp                  { driver.setVectorResult($1);  }
        | lexp                  { driver.setLogicalResult($1); }
;

vexp:   vector                                    { $$ = $1; }
        | vexp '+' vexp 		          { $$ = new Foam::areaVectorField(*$1 + *$3); delete $1; delete $3; }
        | exp '*' vexp 		                  { $$ = new Foam::areaVectorField(*$1 * *$3); delete $1; delete $3; }
        | vexp '*' exp 		                  { $$ = new Foam::areaVectorField(*$1 * *$3); delete $1; delete $3; }
        | vexp '/' exp 		                  { $$ = new Foam::areaVectorField(*$1 / *$3); delete $1; delete $3; }
        | vexp '^' vexp 		          { $$ = new Foam::areaVectorField(*$1 ^ *$3); delete $1; delete $3; }
        | vexp '-' vexp 		          { $$ = new Foam::areaVectorField(*$1 - *$3); delete $1; delete $3;}
        | '-' vexp %prec TOKEN_NEG                { $$ = new Foam::areaVectorField(-*$2); delete $2; }
        | '(' vexp ')'		                  { $$ = $2; }  
        | lexp '?' vexp ':' vexp                  { $$ = driver.doConditional($1,$3,$5,driver.makeVectorField(Foam::vector())); delete $1; delete $3; delete $5; }
        | TOKEN_position '(' ')'                  { $$ = driver.makePositionField(); }
        | TOKEN_laplacian '(' fsexp ',' vexp ')'  { $$ = new Foam::areaVectorField(Foam::fac::laplacian(*$3,*$5)); delete $3; delete $5; }
        | TOKEN_faceAverage '(' fvexp ')'         { $$ = new Foam::areaVectorField(Foam::fac::average(*$3)); delete $3; }
        | TOKEN_integrate '(' fvexp ')'           { $$ = new Foam::areaVectorField(Foam::fac::edgeIntegrate(*$3)); delete $3; }
        | TOKEN_surfSum '(' fvexp ')'             { $$ = new Foam::areaVectorField(Foam::fac::edgeSum(*$3)); delete $3; }
        | TOKEN_min '(' vexp ')'                 { $$ = driver.makeVectorField(Foam::min(*$3).value()); delete $3; }
        | TOKEN_max '(' vexp ')'                 { $$ = driver.makeVectorField(Foam::max(*$3).value()); delete $3; }
        | TOKEN_sum '(' vexp ')'                 { $$ = driver.makeVectorField(Foam::sum(*$3).value()); delete $3; }
        | TOKEN_average '(' vexp ')'             { $$ = driver.makeVectorField(Foam::average(*$3).value()); delete $3; }
        | TOKEN_grad '(' exp ')'                  { $$ = new Foam::areaVectorField(Foam::fac::grad(*$3)); delete $3; }
        | TOKEN_div '(' fsexp ',' vexp ')'        { $$ = new Foam::areaVectorField(Foam::fac::div(*$3,*$5)); delete $3; delete $5; }
        | TOKEN_VID                               { $$=driver.getField<Foam::areaVectorField>(*$1); }
;

fsexp:  TOKEN_surf '(' scalar ')'           { $$ = driver.makeEdgeScalarField($3); }
        | fsexp '+' fsexp 		    { $$ = new Foam::edgeScalarField(*$1 + *$3); delete $1; delete $3; }
        | fsexp '*' fsexp 		    { $$ = new Foam::edgeScalarField(*$1 * *$3); delete $1; delete $3; }
        | fvexp '&' fvexp 		    { $$ = new Foam::edgeScalarField(*$1 & *$3); delete $1; delete $3; }
        | fsexp '/' fsexp 		    { $$ = new Foam::edgeScalarField(*$1 / *$3); delete $1; delete $3; }
        | fsexp '-' fsexp 		    { $$ = new Foam::edgeScalarField(*$1 - *$3); delete $1; delete $3;}
        | TOKEN_pow '(' fsexp ',' scalar ')'{ $$ = new Foam::edgeScalarField(Foam::pow(*$3, $5)); delete $3; }
        | TOKEN_log '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::log(*$3)); delete $3; }
        | TOKEN_exp '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::exp(*$3)); delete $3; }
        | TOKEN_sqr '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::sqr(*$3)); delete $3; }
        | TOKEN_sqrt '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::sqrt(*$3)); delete $3; }
        | TOKEN_sin '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::sin(*$3)); delete $3; }
        | TOKEN_cos '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::cos(*$3)); delete $3; }
        | TOKEN_tan '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::tan(*$3)); delete $3; }
        | TOKEN_log10 '(' fsexp ')'         { $$ = new Foam::edgeScalarField(Foam::log10(*$3)); delete $3; }
        | TOKEN_asin '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::asin(*$3)); delete $3; }
        | TOKEN_acos '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::acos(*$3)); delete $3; }
        | TOKEN_atan '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::atan(*$3)); delete $3; }
        | TOKEN_sinh '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::sinh(*$3)); delete $3; }
        | TOKEN_cosh '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::cosh(*$3)); delete $3; }
        | TOKEN_tanh '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::tanh(*$3)); delete $3; }
        | TOKEN_asinh '(' fsexp ')'         { $$ = new Foam::edgeScalarField(Foam::asinh(*$3)); delete $3; }
        | TOKEN_acosh '(' fsexp ')'         { $$ = new Foam::edgeScalarField(Foam::acosh(*$3)); delete $3; }
        | TOKEN_atanh '(' fsexp ')'         { $$ = new Foam::edgeScalarField(Foam::atanh(*$3)); delete $3; }
        | TOKEN_erf '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::erf(*$3)); delete $3; }
        | TOKEN_erfc '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::erfc(*$3)); delete $3; }
        | TOKEN_lgamma '(' fsexp ')'        { $$ = new Foam::edgeScalarField(Foam::lgamma(*$3)); delete $3; }
        | TOKEN_besselJ0 '(' fsexp ')'      { $$ = new Foam::edgeScalarField(Foam::j0(*$3)); delete $3; }
        | TOKEN_besselJ1 '(' fsexp ')'      { $$ = new Foam::edgeScalarField(Foam::j1(*$3)); delete $3; }
        | TOKEN_besselY0 '(' fsexp ')'      { $$ = new Foam::edgeScalarField(Foam::y0(*$3)); delete $3; }
        | TOKEN_besselY1 '(' fsexp ')'      { $$ = new Foam::edgeScalarField(Foam::y1(*$3)); delete $3; }
        | TOKEN_sign '(' fsexp ')'          { $$ = new Foam::edgeScalarField(Foam::sign(*$3)); delete $3; }
        | TOKEN_pos '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::pos(*$3)); delete $3; }
        | TOKEN_neg '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::neg(*$3)); delete $3; }
        | TOKEN_min '(' fsexp ')'           { $$ = driver.makeEdgeScalarField(Foam::min(*$3).value()); delete $3; }
        | TOKEN_max '(' fsexp ')'           { $$ = driver.makeEdgeScalarField(Foam::max(*$3).value()); delete $3; }
        | TOKEN_sum '(' fsexp ')'           { $$ = driver.makeEdgeScalarField(Foam::sum(*$3).value()); delete $3; }
        | TOKEN_average '(' fsexp ')'       { $$ = driver.makeEdgeScalarField(Foam::average(*$3).value()); delete $3; }
        | '-' fsexp %prec TOKEN_NEG         { $$ = new Foam::edgeScalarField(-*$2); delete $2; }
        | '(' fsexp ')'		            { $$ = $2; }  
        | fvexp '.' 'x'                     { $$ = new Foam::edgeScalarField($1->component(0)); delete $1; }
        | fvexp '.' 'y'                     { $$ = new Foam::edgeScalarField($1->component(1)); delete $1; }
        | fvexp '.' 'z'                     { $$ = new Foam::edgeScalarField($1->component(2)); delete $1; }
        | flexp '?' fsexp ':' fsexp         { $$ = driver.doConditional($1,$3,$5,driver.makeEdgeScalarField(0.)); delete $1; delete $3; delete $5; }
        | TOKEN_mag '(' fsexp ')'           { $$ = new Foam::edgeScalarField(Foam::mag(*$3)); delete $3; }
        | TOKEN_mag '(' fvexp ')'           { $$ = new Foam::edgeScalarField(Foam::mag(*$3)); delete $3; }
        | TOKEN_length '(' ')'                { $$ = driver.makeLengthField(); }
        | TOKEN_lnGrad '(' exp ')'          { $$ = new Foam::edgeScalarField(Foam::fac::lnGrad(*$3)); delete $3; }
        | TOKEN_interpolate '(' exp ')'     { $$ = new Foam::edgeScalarField(Foam::fac::interpolate(*$3)); delete $3; }
        | TOKEN_FSID                        { $$ = driver.getField<Foam::edgeScalarField>(*$1); }
;
 
fvexp:  fvector                            { $$ = $1; }
        | fvexp '+' fvexp 		   { $$ = new Foam::edgeVectorField(*$1 + *$3); delete $1; delete $3; }
        | fsexp '*' fvexp 		   { $$ = new Foam::edgeVectorField(*$1 * *$3); delete $1; delete $3; }
        | fvexp '*' fsexp 		   { $$ = new Foam::edgeVectorField(*$1 * *$3); delete $1; delete $3; }
        | fvexp '^' fvexp 		   { $$ = new Foam::edgeVectorField(*$1 ^ *$3); delete $1; delete $3; }
        | fvexp '/' fsexp 		   { $$ = new Foam::edgeVectorField(*$1 / *$3); delete $1; delete $3; }
        | fvexp '-' fvexp 		   { $$ = new Foam::edgeVectorField(*$1 - *$3); delete $1; delete $3;}
        | '-' fvexp %prec TOKEN_NEG 	   { $$ = new Foam::edgeVectorField(-*$2); delete $2; }
        | '(' fvexp ')'		           { $$ = $2; }  
        | flexp '?' fvexp ':' fvexp        { $$ = driver.doConditional($1,$3,$5,driver.makeEdgeVectorField(Foam::vector::zero)); delete $1; delete $3; delete $5; }
        | TOKEN_fposition '(' ')'          { $$ = driver.makeEdgePositionField(); }
        | TOKEN_fprojection '(' ')'        { $$ = driver.makeEdgeProjectionField(); }
        | TOKEN_face '(' ')'               { $$ = driver.makeEdgeField(); }
        | TOKEN_lnGrad '(' vexp ')'        { $$ = new Foam::edgeVectorField(Foam::fac::lnGrad(*$3)); delete $3; }
        | TOKEN_interpolate '(' vexp ')'   { $$ = new Foam::edgeVectorField(Foam::fac::interpolate(*$3)); delete $3; }
        | TOKEN_min '(' fvexp ')'          { $$ = driver.makeEdgeVectorField(Foam::min(*$3).value()); delete $3; }
        | TOKEN_max '(' fvexp ')'          { $$ = driver.makeEdgeVectorField(Foam::max(*$3).value()); delete $3; }
        | TOKEN_sum '(' fvexp ')'          { $$ = driver.makeEdgeVectorField(Foam::sum(*$3).value()); delete $3; }
        | TOKEN_average '(' fvexp ')'      { $$ = driver.makeEdgeVectorField(Foam::average(*$3).value()); delete $3; }
        | TOKEN_FVID                       { $$ = driver.getField<Foam::edgeVectorField>(*$1); }
;
 
scalar:	TOKEN_NUM		        { $$ = $1; }
        | '-' TOKEN_NUM         	{ $$ = -$2; } 
;

exp:    TOKEN_NUM                                  { $$ = driver.makeScalarField($1); }
        | exp '+' exp 		                   { $$ = new Foam::areaScalarField(*$1 + *$3); delete $1; delete $3; }
        | exp '-' exp 		                   { $$ = new Foam::areaScalarField(*$1 - *$3); delete $1; delete $3; }
        | exp '*' exp 		                   { $$ = new Foam::areaScalarField(*$1 * *$3); delete $1; delete $3; }
        | exp '%' exp 		                   { $$ = driver.makeModuloField(*$1,*$3); delete $1; delete $3; }
        | exp '/' exp 		                   { $$ = new Foam::areaScalarField(*$1 / *$3); delete $1; delete $3; }
        | TOKEN_pow '(' exp ',' scalar ')'	   { $$ = new Foam::areaScalarField(Foam::pow(*$3, $5)); delete $3; }
        | TOKEN_log '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::log(*$3)); delete $3; }
        | TOKEN_exp '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::exp(*$3)); delete $3; }
        | vexp '&' vexp 	                   { $$ = new Foam::areaScalarField(*$1 & *$3); delete $1; delete $3;}
        | '-' exp %prec TOKEN_NEG 	           { $$ = new Foam::areaScalarField(-*$2); delete $2; }
	| '(' exp ')'		                   { $$ = $2; }
        | TOKEN_sqr '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::sqr(*$3)); delete $3; }
        | TOKEN_sqrt '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::sqrt(*$3)); delete $3; }
        | TOKEN_sin '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::sin(*$3)); delete $3; }
        | TOKEN_cos '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::cos(*$3)); delete $3; }
        | TOKEN_tan '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::tan(*$3)); delete $3; }
        | TOKEN_log10 '(' exp ')'                  { $$ = new Foam::areaScalarField(Foam::log10(*$3)); delete $3; }
        | TOKEN_asin '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::asin(*$3)); delete $3; }
        | TOKEN_acos '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::acos(*$3)); delete $3; }
        | TOKEN_atan '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::atan(*$3)); delete $3; }
        | TOKEN_sinh '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::sinh(*$3)); delete $3; }
        | TOKEN_cosh '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::cosh(*$3)); delete $3; }
        | TOKEN_tanh '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::tanh(*$3)); delete $3; }
        | TOKEN_asinh '(' exp ')'                  { $$ = new Foam::areaScalarField(Foam::asinh(*$3)); delete $3; }
        | TOKEN_acosh '(' exp ')'                  { $$ = new Foam::areaScalarField(Foam::acosh(*$3)); delete $3; }
        | TOKEN_atanh '(' exp ')'                  { $$ = new Foam::areaScalarField(Foam::atanh(*$3)); delete $3; }
        | TOKEN_erf '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::erf(*$3)); delete $3; }
        | TOKEN_erfc '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::erfc(*$3)); delete $3; }
        | TOKEN_lgamma '(' exp ')'                 { $$ = new Foam::areaScalarField(Foam::lgamma(*$3)); delete $3; }
        | TOKEN_besselJ0 '(' exp ')'               { $$ = new Foam::areaScalarField(Foam::j0(*$3)); delete $3; }
        | TOKEN_besselJ1 '(' exp ')'               { $$ = new Foam::areaScalarField(Foam::j1(*$3)); delete $3; }
        | TOKEN_besselY0 '(' exp ')'               { $$ = new Foam::areaScalarField(Foam::y0(*$3)); delete $3; }
        | TOKEN_besselY1 '(' exp ')'               { $$ = new Foam::areaScalarField(Foam::y1(*$3)); delete $3; }
        | TOKEN_sign '(' exp ')'                   { $$ = new Foam::areaScalarField(Foam::sign(*$3)); delete $3; }
        | TOKEN_pos '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::pos(*$3)); delete $3; }
        | TOKEN_neg '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::neg(*$3)); delete $3; }
        | TOKEN_min '(' exp ')'                    { $$ = driver.makeScalarField(Foam::min(*$3).value()); delete $3; }
        | TOKEN_max '(' exp ')'                    { $$ = driver.makeScalarField(Foam::max(*$3).value()); delete $3; }
        | TOKEN_sum '(' exp ')'                    { $$ = driver.makeScalarField(Foam::sum(*$3).value()); delete $3; }
        | TOKEN_average '(' exp ')'                { $$ = driver.makeScalarField(Foam::average(*$3).value()); delete $3; }
        | TOKEN_mag '(' exp ')'                    { $$ = new Foam::areaScalarField(Foam::mag(*$3)); delete $3; }
        | TOKEN_mag '(' vexp ')'                   { $$ = new Foam::areaScalarField(Foam::mag(*$3)); delete $3; }
        | TOKEN_div '(' vexp ')'                   { $$ = new Foam::areaScalarField(Foam::fac::div(*$3)); delete $3; }
        | TOKEN_div '(' fsexp ')'                  { $$ = new Foam::areaScalarField(Foam::fac::div(*$3)); delete $3; }
        | TOKEN_div '(' fsexp ',' exp ')'          { $$ = new Foam::areaScalarField(Foam::fac::div(*$3,*$5)); delete $3; delete $5; }
        | TOKEN_laplacian '(' exp ')'              { $$ = new Foam::areaScalarField(Foam::fac::laplacian(*$3)); delete $3; }
        | TOKEN_laplacian '(' exp ',' exp ')'      { $$ = new Foam::areaScalarField(Foam::fac::laplacian(*$3,*$5)); delete $3; delete $5; }
        | TOKEN_laplacian '(' fsexp ',' exp ')'    { $$ = new Foam::areaScalarField(Foam::fac::laplacian(*$3,*$5)); delete $3; delete $5; }
        | TOKEN_faceAverage '(' fsexp ')'          { $$ = new Foam::areaScalarField(Foam::fac::average(*$3)); delete $3; }
        | TOKEN_integrate '(' fsexp ')'            { $$ = new Foam::areaScalarField(Foam::fac::edgeIntegrate(*$3)); delete $3; }
        | TOKEN_surfSum '(' fsexp ')'              { $$ = new Foam::areaScalarField(Foam::fac::edgeSum(*$3)); delete $3; }
        | vexp '.' 'x'                             { $$ = new Foam::areaScalarField($1->component(0)); delete $1; }
        | vexp '.' 'y'                             { $$ = new Foam::areaScalarField($1->component(1)); delete $1; }
        | vexp '.' 'z'                             { $$ = new Foam::areaScalarField($1->component(2)); delete $1; }
        | lexp '?' exp ':' exp                     { $$ = driver.doConditional($1,$3,$5,driver.makeScalarField(0.)); delete $1; delete $3; delete $5; }
        | TOKEN_pi                                 { $$ = driver.makeScalarField(M_PI); }
        | TOKEN_rdist '(' vexp ')'                 { $$ = driver.makeRDistanceField(*$3); delete $3; }
        | TOKEN_area '(' ')'                       { $$ = driver.makeAreaField(); }
        | TOKEN_rand '(' ')'                       { $$ = driver.makeRandomField(); }
        | TOKEN_randNormal '(' ')'                 { $$ = driver.makeGaussRandomField(); }
        | TOKEN_id '(' ')'                         { $$ = driver.makeCellIdField(); }
        | TOKEN_cpu'(' ')'                         { $$ = driver.makeScalarField(Foam::Pstream::myProcNo()); }
        | TOKEN_deltaT '(' ')'                     { $$ = driver.makeScalarField(driver.runTime().deltaT().value()); }
        | TOKEN_time '(' ')'                       { $$ = driver.makeScalarField(driver.runTime().time().value()); }
        | TOKEN_SID		                   { $$ = driver.getField<Foam::areaScalarField>(*$1); }
        | TOKEN_LINE		                   { $$ = driver.makeScalarField(driver.getLineValue(*$1,driver.runTime().time().value())); delete $1; }
;

lexp: TOKEN_TRUE                       { $$ = driver.makeScalarField(1); }
    | TOKEN_FALSE                      { $$ = driver.makeScalarField(0); }
    | exp '<' exp                      { $$ = driver.doCompare($1,std::less<Foam::scalar>(),$3);  delete $1; delete $3; }
    | exp '>' exp                      { $$ = driver.doCompare($1,std::greater<Foam::scalar>(),$3);  delete $1; delete $3; }
    | exp TOKEN_LEQ exp                { $$ = driver.doCompare($1,std::less_equal<Foam::scalar>(),$3);  delete $1; delete $3; }
    | exp TOKEN_GEQ exp                { $$ = driver.doCompare($1,std::greater_equal<Foam::scalar>(),$3);  delete $1; delete $3; }
    | exp TOKEN_EQ exp                 { $$ = driver.doCompare($1,std::equal_to<Foam::scalar>(),$3);  delete $1; delete $3; }
    | exp TOKEN_NEQ exp                { $$ = driver.doCompare($1,std::not_equal_to<Foam::scalar>(),$3);  delete $1; delete $3; }
    | '(' lexp ')'		       { $$ = $2; }
    | lexp TOKEN_AND lexp              { $$ = driver.doLogicalOp($1,std::logical_and<Foam::scalar>(),$3);  delete $1; delete $3; }
    | lexp TOKEN_OR lexp               { $$ = driver.doLogicalOp($1,std::logical_or<Foam::scalar>(),$3);  delete $1; delete $3; }
    | '!' lexp %prec TOKEN_NOT         { $$ = driver.doLogicalNot($2); delete $2; }
;

flexp: TOKEN_surf '(' TOKEN_TRUE ')'  { $$ = driver.makeEdgeScalarField(1); }
    | TOKEN_surf '(' TOKEN_FALSE ')'  { $$ = driver.makeEdgeScalarField(0); }
    | fsexp '<' fsexp                 { $$ = driver.doCompare($1,std::less<Foam::scalar>(),$3);  delete $1; delete $3; }
    | fsexp '>' fsexp                 { $$ = driver.doCompare($1,std::greater<Foam::scalar>(),$3);  delete $1; delete $3; }
    | fsexp TOKEN_LEQ fsexp           { $$ = driver.doCompare($1,std::less_equal<Foam::scalar>(),$3);  delete $1; delete $3; }
    | fsexp TOKEN_GEQ fsexp           { $$ = driver.doCompare($1,std::greater_equal<Foam::scalar>(),$3);  delete $1; delete $3; }
    | fsexp TOKEN_EQ fsexp            { $$ = driver.doCompare($1,std::equal_to<Foam::scalar>(),$3);  delete $1; delete $3; }
    | fsexp TOKEN_NEQ fsexp           { $$ = driver.doCompare($1,std::not_equal_to<Foam::scalar>(),$3);  delete $1; delete $3; }
    | '(' flexp ')'		      { $$ = $2; }
    | flexp TOKEN_AND flexp           { $$ = driver.doLogicalOp($1,std::logical_and<Foam::scalar>(),$3);  delete $1; delete $3; }
    | flexp TOKEN_OR flexp            { $$ = driver.doLogicalOp($1,std::logical_or<Foam::scalar>(),$3);  delete $1; delete $3; }
    | '!' flexp %prec TOKEN_NOT       { $$ = driver.doLogicalNot($2); delete $2; }
;

vector: TOKEN_VECTOR '(' exp ',' exp ',' exp ')' {     $$ = driver.makeVectorField($3,$5,$7);  delete $3; delete $5; delete $7;}
;

fvector: TOKEN_VECTOR '(' fsexp ',' fsexp ',' fsexp ')' {     $$ = driver.makeEdgeVectorField($3,$5,$7);  delete $3; delete $5; delete $7;}
;

%%

void parserFaField::FaFieldValueExpressionParser::error (const parserFaField::FaFieldValueExpressionParser::location_type& l,const std::string& m)
{
     driver.error (l, m);
}
