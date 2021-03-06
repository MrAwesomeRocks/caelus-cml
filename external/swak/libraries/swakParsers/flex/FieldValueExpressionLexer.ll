/*----------------------- -*- C++ -*- ---------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
-------------------------------------------------------------------------------
License
    This file is part of swak.

    swak is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    swak is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with swak.  If not, see <http://www.gnu.org/licenses/>.

Description


Contributors/Copyright:
    2006-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/
%{                                          /* -*- C++ -*- */
#include "FieldValueExpressionDriverYY.hpp"
#include <errno.h>
#include "FieldValuePluginFunction.hpp"
#include "FieldValueExpressionParser.tab.hh"
typedef parserField::FieldValueExpressionParser::semantic_type YYSTYPE;
%}

%s setname
%s zonename
%s fsetname
%s fzonename
%s psetname
%s pzonename
%s patchname
%s vectorcomponent
%s tensorcomponent
%x parsedByOtherParser
%x needsIntegerParameter
%x otherMeshField

%option noyywrap nounput batch debug
%option stack
%option prefix="fvexpr"
%option reentrant
%option bison-bridge

id      [[:alpha:]_][[:alnum:]_]*
setid   [[:alpha:]_][[:alnum:]_-]*
patchid [[:alpha:]_][[:alnum:]_-]*
int     [[:digit:]]+

exponent_part              [eE][-+]?[[:digit:]]+
fractional_constant        (([[:digit:]]*"."[[:digit:]]+)|([[:digit:]]+".")|([[:digit:]]+))
float                      ((({fractional_constant}{exponent_part}?)|([[:digit:]]+{exponent_part}))|0)

%{
# define YY_USER_ACTION yylloc->columns (yyleng);
%}
%%

%{
    typedef parserField::FieldValueExpressionParser::token token;

    yylloc->step ();

    // recipie from http://www.gnu.org/software/bison/manual/html_node/Multiple-start_002dsymbols.html#Multiple-start_002dsymbols
    // allows multiple start symbols
    if (start_token)
    {
        if(driver.traceScanning()) {
            CML::Info << "Start token: " << start_token << CML::endl;
        }

        int t = start_token;
        start_token = 0;
        return t;
    }
%}

<INITIAL,setname,zonename,fsetname,fzonename,psetname,pzonename,patchname,needsIntegerParameter,otherMeshField>[ \t]+             yylloc->step ();
[\n]+                yylloc->lines (yyleng); yylloc->step ();

<INITIAL,setname,zonename,fsetname,fzonename,psetname,pzonename,patchname>[-+*/%(),&^<>!?:.]               return yytext[0];

<needsIntegerParameter,otherMeshField>[(] return yytext[0];
<needsIntegerParameter,otherMeshField>[)] { BEGIN(INITIAL); return yytext[0]; }

&&                   return token::TOKEN_AND;
\|\|                 return token::TOKEN_OR;
==                   return token::TOKEN_EQ;
!=                   return token::TOKEN_NEQ;
\<=                   return token::TOKEN_LEQ;
\>=                   return token::TOKEN_GEQ;

<vectorcomponent,tensorcomponent>x    { BEGIN(INITIAL); return token::TOKEN_x; }
<vectorcomponent,tensorcomponent>y    { BEGIN(INITIAL); return token::TOKEN_y; }
<vectorcomponent,tensorcomponent>z    { BEGIN(INITIAL); return token::TOKEN_z; }

<tensorcomponent>xx    { BEGIN(INITIAL); return token::TOKEN_xx; }
<tensorcomponent>xy    { BEGIN(INITIAL); return token::TOKEN_xy; }
<tensorcomponent>xz    { BEGIN(INITIAL); return token::TOKEN_xz; }
<tensorcomponent>yx    { BEGIN(INITIAL); return token::TOKEN_yx; }
<tensorcomponent>yy    { BEGIN(INITIAL); return token::TOKEN_yy; }
<tensorcomponent>yz    { BEGIN(INITIAL); return token::TOKEN_yz; }
<tensorcomponent>zx    { BEGIN(INITIAL); return token::TOKEN_zx; }
<tensorcomponent>zy    { BEGIN(INITIAL); return token::TOKEN_zy; }
<tensorcomponent>zz    { BEGIN(INITIAL); return token::TOKEN_zz; }
<tensorcomponent>ii    { BEGIN(INITIAL); return token::TOKEN_ii; }
<tensorcomponent>T     { BEGIN(INITIAL); return token::TOKEN_transpose; }

pow                   return token::TOKEN_pow;
exp                   return token::TOKEN_exp;
log                   return token::TOKEN_log;
mag                   return token::TOKEN_mag;
magSqr                return token::TOKEN_magSqr;
sin                   return token::TOKEN_sin;
cos                   return token::TOKEN_cos;
tan                   return token::TOKEN_tan;
min                   return token::TOKEN_min;
max                   return token::TOKEN_max;
minPosition           return token::TOKEN_minPosition;
maxPosition           return token::TOKEN_maxPosition;
sum                   return token::TOKEN_sum;
average               return token::TOKEN_average;
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
pts                   return token::TOKEN_pposition;
fpos                  return token::TOKEN_fposition;
fproj                 return token::TOKEN_fprojection;
face                  return token::TOKEN_face;
area                  return token::TOKEN_area;
vol                   return token::TOKEN_volume;
dist                  return token::TOKEN_dist;
distToPatch                  {
    BEGIN(patchname);
    return token::TOKEN_distToPatch;
                      }
nearDist              return token::TOKEN_nearDist;
rdist                 return token::TOKEN_rdist;
rand                  { BEGIN(needsIntegerParameter); return token::TOKEN_rand; }
randFixed             { BEGIN(needsIntegerParameter); return token::TOKEN_randFixed; }
id                    return token::TOKEN_id;
randNormal            { BEGIN(needsIntegerParameter); return token::TOKEN_randNormal; }
randNormalFixed       { BEGIN(needsIntegerParameter); return token::TOKEN_randNormalFixed; }

cpu                   return token::TOKEN_cpu;
weight                return token::TOKEN_weight;

set                   {
    BEGIN(setname);
    return token::TOKEN_set;
                      }

zone                  {
    BEGIN(zonename);
    return token::TOKEN_zone;
                      }

fset                   {
    BEGIN(fsetname);
    return token::TOKEN_fset;
                      }

fzone                  {
    BEGIN(fzonename);
    return token::TOKEN_fzone;
                      }

pset                   {
    BEGIN(psetname);
    return token::TOKEN_pset;
                      }

pzone                  {
    BEGIN(pzonename);
    return token::TOKEN_pzone;
                      }

onPatch                  {
    BEGIN(patchname);
    return token::TOKEN_onPatch;
                      }

internalFace          return token::TOKEN_internalFace;

grad                  return token::TOKEN_grad;
curl                  return token::TOKEN_curl;
magSqrGradGrad        return token::TOKEN_magSqrGradGrad;
snGrad                return token::TOKEN_snGrad;
div                   return token::TOKEN_div;
laplacian             return token::TOKEN_laplacian;
ddt                   return token::TOKEN_ddt;
oldTime               return token::TOKEN_oldTime;
d2dt2                 return token::TOKEN_d2dt2;
meshPhi               return token::TOKEN_meshPhi;
flux                  return token::TOKEN_flux;

integrate             return token::TOKEN_integrate;
surfSum               return token::TOKEN_surfSum;
interpolate           return token::TOKEN_interpolate;
interpolateToPoint    return token::TOKEN_interpolateToPoint;
interpolateToCell     return token::TOKEN_interpolateToCell;
faceAverage           return token::TOKEN_faceAverage;
reconstruct           return token::TOKEN_reconstruct;

deltaT                return token::TOKEN_deltaT;
time                  return token::TOKEN_time;

vector                 return token::TOKEN_VECTOR;
tensor                 return token::TOKEN_TENSOR;
symmTensor             return token::TOKEN_SYMM_TENSOR;
sphericalTensor        return token::TOKEN_SPHERICAL_TENSOR;

surf                   return token::TOKEN_surf;
point                  return token::TOKEN_point;

diag                   return token::TOKEN_diag;
tr                     return token::TOKEN_tr;
dev                    return token::TOKEN_dev;
symm                   return token::TOKEN_symm;
skew                   return token::TOKEN_skew;
det                    return token::TOKEN_det;
cof                    return token::TOKEN_cof;
inv                    return token::TOKEN_inv;
sph                    return token::TOKEN_sph;
twoSymm                return token::TOKEN_twoSymm;
dev2                   return token::TOKEN_dev2;
eigenValues            return token::TOKEN_eigenValues;
eigenVectors           return token::TOKEN_eigenVectors;

true                   return token::TOKEN_TRUE;
false                  return token::TOKEN_FALSE;

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

<otherMeshField>{id} {
    CML::word *ptr=new CML::word (yytext);
    if(
        driver.isForeignField<CML::volScalarField>(
            driver.otherMeshName(),
            *ptr
        )
    ) {
        yylval->name = ptr; return token::TOKEN_OTHER_MESH_SID;
    } else if(
        driver.isForeignField<CML::volVectorField>(
            driver.otherMeshName(),
            *ptr
        )
    ) {
        yylval->name = ptr; return token::TOKEN_OTHER_MESH_VID;
    } else if(
        driver.isForeignField<CML::volTensorField>(
            driver.otherMeshName(),
            *ptr
        )
    ) {
        yylval->name = ptr; return token::TOKEN_OTHER_MESH_TID;
    } else if(
        driver.isForeignField<CML::volSymmTensorField>(
            driver.otherMeshName(),
            *ptr
        )
    ) {
        yylval->name = ptr; return token::TOKEN_OTHER_MESH_YID;
    } else if(
        driver.isForeignField<CML::volSphericalTensorField>(
            driver.otherMeshName(),
            *ptr
        )
    ) {
        yylval->name = ptr; return token::TOKEN_OTHER_MESH_HID;
    } else {
        driver.error(
            *yylloc,
            "Foreign mesh "+driver.otherMeshName()
            +" does not have a field named "+(*ptr)
        );
    }
                     }
<INITIAL>{id}                 {
    CML::word *ptr=new CML::word (yytext);

    if(CML::MeshesRepository::getRepository().hasMesh(*ptr)) {
        BEGIN(otherMeshField);
        driver.otherMeshName()=(*ptr);
        yylval->name=ptr;
        return token::TOKEN_OTHER_MESH_ID;
    } else if(driver.isLine(*ptr)) {
        yylval->name = ptr; return token::TOKEN_LINE;
    } else if(driver.isLookup(*ptr)) {
        yylval->name = ptr; return token::TOKEN_LOOKUP;
    } else if(
        driver.isVariable<CML::surfaceScalarField::value_type>(*ptr,false,driver.mesh().nInternalFaces())
    ) {
        yylval->name = ptr; return token::TOKEN_FSID;
    } else if(
        driver.isVariable<CML::pointScalarField::value_type>(*ptr,true)
    ) {
        yylval->name = ptr; return token::TOKEN_PSID;
    } else if(
        driver.isVariable<CML::surfaceVectorField::value_type>(*ptr,false,driver.mesh().nInternalFaces())
    ) {
        yylval->name = ptr; return token::TOKEN_FVID;
    } else if(
        driver.isVariable<CML::pointVectorField::value_type>(*ptr,true)
    ) {
        yylval->name = ptr; return token::TOKEN_PVID;
    } else if(
        driver.isVariable<CML::surfaceTensorField::value_type>(*ptr,false,driver.mesh().nInternalFaces())
    ) {
        yylval->name = ptr; return token::TOKEN_FTID;
    } else if(
        driver.isVariable<CML::pointTensorField::value_type>(*ptr,true)
    ) {
        yylval->name = ptr; return token::TOKEN_PTID;
    } else if(
        driver.isVariable<CML::surfaceSymmTensorField::value_type>(*ptr,false,driver.mesh().nInternalFaces())
    ) {
        yylval->name = ptr; return token::TOKEN_FYID;
    } else if(
        driver.isVariable<CML::pointSymmTensorField::value_type>(*ptr,true)
    ) {
        yylval->name = ptr; return token::TOKEN_PYID;
    } else if(
        driver.isVariable<CML::surfaceSphericalTensorField::value_type>(*ptr,false,driver.mesh().nInternalFaces())
    ) {
        yylval->name = ptr; return token::TOKEN_FHID;
    } else if(
        driver.isVariable<CML::pointSphericalTensorField::value_type>(*ptr,true)
    ) {
        yylval->name = ptr; return token::TOKEN_PHID;
    } else if(
        driver.isVariable<CML::volScalarField::value_type>(*ptr)
        ||
        driver.isThere<CML::volScalarField>(*ptr)
    ) {
        yylval->name = ptr; return token::TOKEN_SID;
    } else if(
        driver.isVariable<CML::volVectorField::value_type>(*ptr)
        ||
        driver.isThere<CML::volVectorField>(*ptr)
    ) {
        yylval->name = ptr; return token::TOKEN_VID;
    } else if(
        driver.isVariable<CML::volTensorField::value_type>(*ptr)
        ||
        driver.isThere<CML::volTensorField>(*ptr)
    ) {
        yylval->name = ptr; return token::TOKEN_TID;
    } else if(
        driver.isVariable<CML::volSymmTensorField::value_type>(*ptr)
        ||
        driver.isThere<CML::volSymmTensorField>(*ptr)
    ) {
        yylval->name = ptr; return token::TOKEN_YID;
    } else if(
        driver.isVariable<CML::volSphericalTensorField::value_type>(*ptr)
        ||
        driver.isThere<CML::volSphericalTensorField>(*ptr)
    ) {
        yylval->name = ptr; return token::TOKEN_HID;
    } else if(driver.isThere<CML::surfaceVectorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FVID;
    } else if(driver.isThere<CML::surfaceScalarField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FSID;
    } else if(driver.isThere<CML::surfaceTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FTID;
    } else if(driver.isThere<CML::surfaceSymmTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FYID;
    } else if(driver.isThere<CML::surfaceSphericalTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FHID;
    } else if(driver.isThere<CML::pointVectorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PVID;
    } else if(driver.isThere<CML::pointScalarField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PSID;
    } else if(driver.isThere<CML::pointTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PTID;
    } else if(driver.isThere<CML::pointSymmTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PYID;
    } else if(driver.isThere<CML::pointSphericalTensorField>(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PHID;
    } else if(CML::FieldValuePluginFunction::exists(driver,*ptr)) {
        // OK. We'll create the function two times. But this is less messy
        // than passing it two times
        CML::autoPtr<CML::FieldValuePluginFunction> fInfo(
            CML::FieldValuePluginFunction::New(
                driver,
                *ptr
            )
        );

        int tokenTyp=-1;
        if(fInfo->returnType()=="volScalarField") {
             tokenTyp=token::TOKEN_FUNCTION_SID;
        } else if(fInfo->returnType()=="surfaceScalarField") {
             tokenTyp=token::TOKEN_FUNCTION_FSID;
        } else if(fInfo->returnType()=="pointScalarField") {
             tokenTyp=token::TOKEN_FUNCTION_PSID;
        } else if(fInfo->returnType()=="volVectorField") {
             tokenTyp=token::TOKEN_FUNCTION_VID;
        } else if(fInfo->returnType()=="surfaceVectorField") {
             tokenTyp=token::TOKEN_FUNCTION_FVID;
        } else if(fInfo->returnType()=="pointVectorField") {
             tokenTyp=token::TOKEN_FUNCTION_PVID;
        } else if(fInfo->returnType()=="volTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_TID;
        } else if(fInfo->returnType()=="surfaceTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_FTID;
        } else if(fInfo->returnType()=="pointTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_PTID;
        } else if(fInfo->returnType()=="volSymmTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_YID;
        } else if(fInfo->returnType()=="surfaceSymmTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_FYID;
        } else if(fInfo->returnType()=="pointSymmTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_PYID;
        } else if(fInfo->returnType()=="volSphericalTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_HID;
        } else if(fInfo->returnType()=="surfaceSphericalTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_FHID;
        } else if(fInfo->returnType()=="pointSphericalTensorField") {
             tokenTyp=token::TOKEN_FUNCTION_PHID;
        } else if(fInfo->returnType()=="volLogicalField") {
             tokenTyp=token::TOKEN_FUNCTION_LID;
        } else if(fInfo->returnType()=="surfaceLogicalField") {
             tokenTyp=token::TOKEN_FUNCTION_FLID;
        } else if(fInfo->returnType()=="pointLogicalField") {
             tokenTyp=token::TOKEN_FUNCTION_PLID;
        } else {
            driver.error (
                *yylloc,
                "Function "+*ptr+" returns unsupported type "
                + fInfo->returnType()
            );
        }

        //        BEGIN(parsedByOtherParser);

        yylval->name = ptr;
        return tokenTyp;
    } else {
        if((*ptr)=="I") {
            delete ptr;
            return token::TOKEN_unitTensor;
        }
        driver.error (*yylloc, "field "+*ptr+" not existing or of wrong type");
    }
                     }

<setname>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isCellSet(*ptr)) {
        yylval->name = ptr; return token::TOKEN_SETID;
    } else {
        driver.error (*yylloc, "cellSet id "+*ptr+" not existing or of wrong type");
    }
                     }
<zonename>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isCellZone(*ptr)) {
        yylval->name = ptr; return token::TOKEN_ZONEID;
    } else {
        driver.error (*yylloc, "cellZone id "+*ptr+" not existing or of wrong type");
    }
                     }
<fsetname>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isFaceSet(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FSETID;
    } else {
        driver.error (*yylloc, "faceSet id "+*ptr+" not existing or of wrong type");
    }
                     }
<fzonename>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isFaceZone(*ptr)) {
        yylval->name = ptr; return token::TOKEN_FZONEID;
    } else {
        driver.error (*yylloc, "faceZone id "+*ptr+" not existing or of wrong type");
    }
                     }

<psetname>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isPointSet(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PSETID;
    } else {
        driver.error (*yylloc, "pointSet id "+*ptr+" not existing or of wrong type");
    }
                     }
<pzonename>{setid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    if(driver.isPointZone(*ptr)) {
        yylval->name = ptr; return token::TOKEN_PZONEID;
    } else {
        driver.error (*yylloc, "pointZone id "+*ptr+" not existing or of wrong type");
    }
                     }

<patchname>{patchid}              {
    CML::word *ptr=new CML::word (yytext);
    BEGIN(INITIAL);
    CML::label patchI=driver.mesh().boundaryMesh().findPatchID(*ptr);
    if(patchI>=0) {
        yylval->name = ptr; return token::TOKEN_PATCHID;
    } else {
        driver.error (*yylloc, "patch id "+*ptr+" does not exist in the mesh");
    }
                     }

<parsedByOtherParser>. {
    numberOfFunctionChars--;
    if(driver.traceScanning()) {
        CML::Info << " Remaining characters to be eaten: " << numberOfFunctionChars
            << CML::endl;
    }
    if(numberOfFunctionChars>0) {
        return token::TOKEN_IN_FUNCTION_CHAR;
    } else {
        BEGIN(INITIAL);
        return token::TOKEN_LAST_FUNCTION_CHAR;
    }
                       }

.                    driver.error (*yylloc, "invalid character");
<needsIntegerParameter>.                    driver.error (*yylloc, "invalid character when only an integer parameter is expected");

%%

void FieldValueExpressionDriver::scan_begin ()
{
    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::scan_begin "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }

    if(scanner_!=nullptr) {
        FatalErrorInFunction
            << "Already existing scanner " << getHex(scanner_)
            << endl
            << exit(FatalError);

    }

    yylex_init(&scanner_);
    struct yyguts_t * yyg = (struct yyguts_t*)scanner_;
    yy_flex_debug = traceScanning();
    yy_scan_string(content().c_str(),scanner_);
//    if (!(yyin = fopen (file.c_str (), "r")))
//        error (std::string ("cannot open ") + file);

    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::scan_begin - finished "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }
}

void FieldValueExpressionDriver::scan_end ()
{
    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::scan_end "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }

    if(scanner_==nullptr) {
        FatalErrorInFunction
            << "Uninitialized Scanner. Can't delete it"
            << endl
            << exit(FatalError);

    }

    yylex_destroy(scanner_);
    // WarningInFunction
    //     << "Scanner " <<  getHex(scanner_) << " is not deleted"
    //         << endl;

    scanner_=nullptr;
//	    fclose (yyin);
}

void FieldValueExpressionDriver::startEatCharacters()
{
    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::startEatCharacters() "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }

    struct yyguts_t * yyg = (struct yyguts_t*)scanner_;
    BEGIN(parsedByOtherParser);
}

void FieldValueExpressionDriver::startVectorComponent()
{
    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::startVectorComponent() "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }

    struct yyguts_t * yyg = (struct yyguts_t*)scanner_;
    BEGIN(vectorcomponent);
}

void FieldValueExpressionDriver::startTensorComponent()
{
    if(traceScanning()) {
        Info << "FieldValueExpressionDriver::startTensorComponent() "
            << getHex(this) << endl;
        Info << "Scanner: " << getHex(scanner_) << endl;
    }

    struct yyguts_t * yyg = (struct yyguts_t*)scanner_;
    BEGIN(tensorcomponent);
}
