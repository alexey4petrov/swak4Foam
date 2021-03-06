/*---------------------------------------------------------------------------*\
 ##   ####  ######     | 
 ##  ##     ##         | Copyright: ICE Stroemungsfoschungs GmbH
 ##  ##     ####       |
 ##  ##     ##         | http://www.ice-sf.at
 ##   ####  ######     |
-------------------------------------------------------------------------------
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2008 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is based on OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

 ICE Revision: $Id$ 
\*---------------------------------------------------------------------------*/

#include "CommonValueExpressionDriver.H"
#include "GlobalVariablesRepository.H"

#include "Random.H"

namespace Foam {


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(CommonValueExpressionDriver,0);
defineRunTimeSelectionTable(CommonValueExpressionDriver, dictionary);
defineRunTimeSelectionTable(CommonValueExpressionDriver, idName);

    // Currently not working
bool CommonValueExpressionDriver::cacheSets_=true;

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


CommonValueExpressionDriver::CommonValueExpressionDriver(
    const CommonValueExpressionDriver& orig
)
:
    variableStrings_(orig.variableStrings_),
    result_(orig.result_),
    variables_(orig.variables_),
    storedVariables_(orig.storedVariables_),
    storedVariablesIndex_(orig.storedVariablesIndex_),
    globalVariableScopes_(orig.globalVariableScopes_),
    lines_(orig.lines_),
    lookup_(orig.lookup_),
    content_(""),
    trace_scanning_ (orig.trace_scanning_),
    trace_parsing_ (orig.trace_parsing_)
{
    setSearchBehaviour(
        orig.cacheReadFields_,
        orig.searchInMemory_,
        orig.searchOnDisc_
    );
}

CommonValueExpressionDriver::CommonValueExpressionDriver(const dictionary& dict)
:
    variableStrings_(readVariableStrings(dict)),
    storedVariablesIndex_(-1),
    globalVariableScopes_(dict.lookupOrDefault("globalScopes",wordList())),
    content_(""),
    trace_scanning_ (dict.lookupOrDefault("traceScanning",false)),
    trace_parsing_ (dict.lookupOrDefault("traceParsing",false))
{
    if(dict.found("storedVariables")) {
        storedVariables_=List<StoredExpressionResult>(dict.lookup("storedVariables"));
    }

    if(debug) {
        Info << "CommonValueExpressionDriver::CommonValueExpressionDriver(const dictionary& dict)" << endl;
    }

    setSearchBehaviour(
        dict.lookupOrDefault("cacheReadFields",false),
        dict.lookupOrDefault("searchInMemory",true),
        dict.lookupOrDefault("searchOnDisc",false)
    );

    readTables(dict);
}

CommonValueExpressionDriver::CommonValueExpressionDriver(
    bool cacheReadFields,
    bool searchInMemory,
    bool searchOnDisc
)
:
    variableStrings_(),
    storedVariablesIndex_(-1),
    globalVariableScopes_(),
    content_(""),
    trace_scanning_ (false),
    trace_parsing_ (false)
{
    setSearchBehaviour(
        cacheReadFields,
        searchInMemory,
        searchOnDisc
    );
}

void CommonValueExpressionDriver::readVariablesAndTables(const dictionary &dict)
{
    if(dict.found("globalScopes")) {
        setGlobalScopes(wordList(dict.lookup("globalScopes")));
    }

    if(dict.found("storedVariables")) {
        storedVariables_=List<StoredExpressionResult>(dict.lookup("storedVariables"));
    }

    setVariableStrings(dict);

    readTables(dict);
}

void CommonValueExpressionDriver::readTables(const dictionary &dict)
{
    if(dict.found("timelines")) {
        readTables(dict.lookup("timelines"),lines_);
    }

    if(dict.found("lookuptables")) {
        readTables(dict.lookup("lookuptables"),lookup_);
    }
}

void CommonValueExpressionDriver::setSearchBehaviour(
    bool cacheReadFields,
    bool searchInMemory,
    bool searchOnDisc
)
{
    cacheReadFields_=cacheReadFields;
    searchInMemory_=
        searchInMemory
        ||
        cacheReadFields;
    searchOnDisc_=searchOnDisc;
}

autoPtr<CommonValueExpressionDriver> CommonValueExpressionDriver::New
(
    const dictionary& dict,
    const fvMesh& mesh
)
{
    word driverType(dict.lookup("valueType"));
    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(driverType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "autoPtr<CommonValueExpressionDriver> CommonValueExpressionDriver::New"
        )   << "Unknown  CommonValueExpressionDriver type " << driverType
            << endl << endl
            << "Valid valueTypes are :" << endl
	  //            << dictionaryConstructorTablePtr_->sortedToc() // does not work in 1.6
            << dictionaryConstructorTablePtr_->toc()
            << exit(FatalError);
    }

    if(debug) {
        Info << "Creating driver of type " << driverType << endl;
    }

    return autoPtr<CommonValueExpressionDriver>
    (
        cstrIter()(dict,mesh)
    );
}

autoPtr<CommonValueExpressionDriver> CommonValueExpressionDriver::New
(
    const word& driverType,
    const word& id,
    const fvMesh& mesh
)
{
    idNameConstructorTable::iterator cstrIter =
        idNameConstructorTablePtr_->find(driverType);

    if (cstrIter == idNameConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "autoPtr<CommonValueExpressionDriver> CommonValueExpressionDriver::New"
        )   << "Unknown  CommonValueExpressionDriver type " << driverType
            << endl << endl
            << "Valid valueTypes are :" << endl
	  //            << idNameConstructorTablePtr_->sortedToc() // does not work in 1.6
            << idNameConstructorTablePtr_->toc()
            << exit(FatalError);
    }

    if(debug) {
        Info << "Creating driver of type " << driverType << endl;
    }

    return autoPtr<CommonValueExpressionDriver>
    (
        cstrIter()(id,mesh)
    );
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CommonValueExpressionDriver::~CommonValueExpressionDriver()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

stringList CommonValueExpressionDriver::readVariableStrings(const dictionary &dict)
{
    if(!dict.found("variables")) {
        return stringList();
    }
    ITstream data(dict.lookup("variables"));
    token nextToken;
    data.read(nextToken);
    if(nextToken.isString()) {
        data.rewind();
        return stringList(1,string(data));
    } else if(
        nextToken.type()==token::PUNCTUATION
        &&
        nextToken.pToken()==token::BEGIN_LIST
    ) {
        data.rewind();
        return stringList(data);
    } if(nextToken.isLabel()) {
        token anotherToken;
        data.read(anotherToken);
        if(
            anotherToken.type()==token::PUNCTUATION
            &&
            anotherToken.pToken()==token::BEGIN_LIST
        ) {
            data.rewind();
            return stringList(data);
        }
    }

    FatalErrorIn("CommonValueExpressionDriver::readVariableStrings(const dictionary &dict)")
        << " Entry 'variables' must either be a string or a list of strings"
            << endl
            << abort(FatalError);
    
    return stringList();
}

void CommonValueExpressionDriver::setVariableStrings(const dictionary &dict)
{
    variableStrings_=readVariableStrings(dict);
}

Ostream &CommonValueExpressionDriver::writeVariableStrings(Ostream &out) const
{
    if(variableStrings_.size()==0) {
        out << string("");
    } else if(variableStrings_.size()==1) {
        out << variableStrings_[0];
    } else {
        out << variableStrings_;
    }

    return out;
}

Ostream &CommonValueExpressionDriver::writeCommon(Ostream &os,bool debug) const
{
    os.writeKeyword("variables");
    writeVariableStrings(os) << token::END_STATEMENT << nl;

    os.writeKeyword("timelines");
    writeTables(os,lines_);
    os << token::END_STATEMENT << nl;

    os.writeKeyword("lookuptables");
    writeTables(os,lookup_);
    os << token::END_STATEMENT << nl;

    if(debug) {
        os.writeKeyword("variableValues");
        os << variables() << endl;
        os << token::END_STATEMENT << nl;
    }
    
    if(storedVariables_.size()>0) {
        const_cast<CommonValueExpressionDriver&>(*this).updateStoredVariables(true);
        
        os.writeKeyword("storedVariables");
        os << storedVariables_ << endl;
        os << token::END_STATEMENT << nl;
    }

    return os;
}

word CommonValueExpressionDriver::getResultType()
{
    return result_.type();
}

void CommonValueExpressionDriver::clearResult()
{
    result_.clearResult();
}

vectorField *CommonValueExpressionDriver::composeVectorField(scalarField *x,scalarField *y,scalarField *z)
{
    vectorField *result=new vectorField(this->size());

    forAll(*result,faceI) {
        (*result)[faceI]=Foam::vector((*x)[faceI],(*y)[faceI],(*z)[faceI]);
    }

    return result;
}

tensorField *CommonValueExpressionDriver::composeTensorField(
    scalarField *xx,scalarField *xy,scalarField *xz,
    scalarField *yx,scalarField *yy,scalarField *yz,
    scalarField *zx,scalarField *zy,scalarField *zz
)
{
    tensorField *result=new tensorField(this->size());

    forAll(*result,faceI) {
        (*result)[faceI]=Foam::tensor(
            (*xx)[faceI],(*xy)[faceI],(*xz)[faceI],
            (*yx)[faceI],(*yy)[faceI],(*yz)[faceI],
            (*zx)[faceI],(*zy)[faceI],(*zz)[faceI]
        );
        
    }

    return result;
}

symmTensorField *CommonValueExpressionDriver::composeSymmTensorField(
    scalarField *xx,scalarField *xy,scalarField *xz,
    scalarField *yy,scalarField *yz,
    scalarField *zz
)
{
    symmTensorField *result=new symmTensorField(this->size());

    forAll(*result,faceI) {
        (*result)[faceI]=Foam::symmTensor(
            (*xx)[faceI],(*xy)[faceI],(*xz)[faceI],
            (*yy)[faceI],(*yz)[faceI],
            (*zz)[faceI]
        );
        
    }

    return result;
}

sphericalTensorField *CommonValueExpressionDriver::composeSphericalTensorField(
    scalarField *ii
)
{
    sphericalTensorField *result=new sphericalTensorField(this->size());

    forAll(*result,faceI) {
        (*result)[faceI]=Foam::sphericalTensor(
            (*ii)[faceI]
        );
        
    }

    return result;
}

    /* void CommonValueExpressionDriver::parse (const std::string& f)
{
    content_ = f;
    scan_begin ();
    parserCommon::CommonValueExpressionParser parser (*this);
    parser.set_debug_level (trace_parsing);
    parser.parse ();
    scan_end ();
}
        */

ExpressionResult CommonValueExpressionDriver::getUniform(label size,bool noWarning)
{
    return result_.getUniform(size,noWarning);
}

void CommonValueExpressionDriver::error (const std::string& m)
{
    Foam::FatalErrorIn("parsingValue")
        //        << Foam::args.executable()
            << " Parser Error: " << m
            << Foam::exit(Foam::FatalError);
}

const Time &CommonValueExpressionDriver::runTime() const
{
    return this->mesh().time();
}

scalarField *CommonValueExpressionDriver::makeModuloField(const scalarField &a,const scalarField &b)
{
    assert(a.size()==b.size());

    scalarField *result=new scalarField(this->size());
    forAll(*result,i) {
        scalar val=fmod(a[i],b[i]);
        if(fabs(val)>(b[i]/2)) {
            if(val>0) {
                val -= b[i];
            } else {
                val += b[i];
            }
        }
        (*result)[i]=val;
    }

    return result;
}

scalarField *CommonValueExpressionDriver::makeRandomField(label seed)
{
    scalarField *result=new scalarField(this->size());

    if(seed<=0) {
        seed=runTime().timeIndex()-seed;
    }

    Foam::Random rand(seed);
    forAll(*result,i) {
        (*result)[i]=rand.scalar01();
    }

    return result;
}

scalarField *CommonValueExpressionDriver::getLine(const string &name,scalar t)
{
    return new scalarField(this->size(),lines_[name](t));
}

tmp<scalarField> CommonValueExpressionDriver::getLookup(const string &name,const scalarField &val)
{
    scalarField *result=new scalarField(val.size());
    const interpolationTable<scalar> &table=lookup_[name];

    forAll(val,i) {
        (*result)[i]=table(val[i]);
    }

    return tmp<scalarField>(result);
}

scalar CommonValueExpressionDriver::getLineValue(const string &name,scalar t)
{
    return lines_[name](t);
}

scalarField *CommonValueExpressionDriver::makeGaussRandomField(label seed)
{
    scalarField *result=new scalarField(this->size());

    if(seed<=0) {
        seed=runTime().timeIndex()-seed;
    }

    Foam::Random rand(seed);
    forAll(*result,i) {
        (*result)[i]=rand.GaussNormal();
    }

    return result;
}

bool CommonValueExpressionDriver::update()
{
    return true;
}

void CommonValueExpressionDriver::updateStoredVariables(bool force)
{
    if(storedVariablesIndex_<0) {
        if(debug) {
            Info << "First update: " << mesh().time().timeIndex() << endl;
        }
        storedVariablesIndex_=mesh().time().timeIndex();
        forAll(storedVariables_,i) {
            StoredExpressionResult &v=storedVariables_[i];
            if(!v.hasValue()) {
                if(debug) {
                    Info << "First valuate: " << v.initialValueExpression() 
                        << " -> " << v.name() << endl;
                }
                parse(v.initialValueExpression());
                v=result_;
            }
        }        
    }

    if(
        force
        ||
        storedVariablesIndex_!=mesh().time().timeIndex()
    ) {
        if(debug) {
            Info << "Store variables: " << force << " " 
                << storedVariablesIndex_ << " " << mesh().time().timeIndex() << endl;
        }
        forAll(storedVariables_,i) {
            StoredExpressionResult &v=storedVariables_[i];
            if(variables_.found(v.name())) {
                if(debug) {
                    Info << "Storing variable: " << v.name() << " " 
                        << variables_[v.name()] << endl;
                }
                v=variables_[v.name()];
            }
        }
        storedVariablesIndex_=mesh().time().timeIndex();
    }
}

void CommonValueExpressionDriver::clearVariables()
{
    if(debug) {
        Info << "Clearing variables" << endl;
    }

    this->update();

    updateStoredVariables();
    variables_.clear();
    forAll(storedVariables_,i) {
        StoredExpressionResult &v=storedVariables_[i];
        variables_.insert(v.name(),v);
    }
    
    addVariables(variableStrings_,false);
}

void CommonValueExpressionDriver::evaluateVariable(const word &name,const string &expr)
{
    parse(expr);

    if(debug) {
        Info << "Evaluating: " << expr << " -> " << name << endl;
        Info << result_;
    }

    variables_.set(name,ExpressionResult(result_));
}

void CommonValueExpressionDriver::evaluateVariableRemote(const string &remoteExpr,const word &name,const string &expr)
{
    string remote=remoteExpr;
    word regionName="";
    word id="";
    word type="patch";

    std::string::size_type slashPos=remote.find('/');

    if(slashPos!=std::string::npos) {
        regionName=remote.substr(slashPos+1);
        remote=remote.substr(0,slashPos);
    }
    
    std::string::size_type quotePos=remote.find('\'');
    if(quotePos!=std::string::npos) {
        id=remote.substr(quotePos+1);
        type=remote.substr(0,quotePos);        
    } else {
        id=remote;
    }

    if(
        type=="patch" 
        && 
        (id=="internalField" || id=="")
    ) {
        type="internalField";
    }

    const fvMesh *pRegion=&(this->mesh());

    if(regionName!="") {
        //        pRegion=&(dynamicCast<const fvMesh&>( // doesn't work with gcc 4.2
        pRegion=&(dynamic_cast<const fvMesh&>(
                      pRegion->time().lookupObject<objectRegistry>(regionName))
        );
    }

    const fvMesh &region=*pRegion;

    autoPtr<CommonValueExpressionDriver> otherDriver=CommonValueExpressionDriver::New(
        type,
        id,
        region
    );

    otherDriver->setSearchBehaviour(
        this->cacheReadFields(),
        this->searchInMemory(),
        this->searchOnDisc()
    );
    otherDriver->setGlobalScopes(
        this->globalVariableScopes_
    );

    otherDriver->parse(expr);
    variables_.insert(name,otherDriver->getUniform(this->size(),false));
}

void CommonValueExpressionDriver::addVariables(const stringList &exprList,bool clear)
{
    if(clear) {
        clearVariables();
    }
    forAll(exprList,i) {
        addVariables(exprList[i],false);
    }
}

void CommonValueExpressionDriver::addVariables(const string &exprList,bool clear)
{
    if(clear) {
        clearVariables();
    }

    std::string::size_type start=0;
    std::string::size_type end=0;

    while(start<exprList.length()) {
        end=exprList.find(';',start);
        if(end==std::string::npos) {
            FatalErrorIn("CommonValueExpressionDriver::addVariables(const string &exprList,bool clear)")
                << "No terminating ';' found in expression '"
                    << exprList.substr(start) << "'\n"
                    << endl
                    << abort(FatalError);            
        }
        std::string::size_type  eqPos=exprList.find('=',start);
        if(eqPos==std::string::npos || eqPos > end) {
            FatalErrorIn("CommonValueExpressionDriver::addVariables(const string &exprList,bool clear)")
                << "No '=' found in expression '"
                    << exprList.substr(start,end-start) << "'\n"
                    << endl
                    << abort(FatalError);            
        }
        string expr(exprList.substr(eqPos+1,end-eqPos-1));

        std::string::size_type  startPos=exprList.find('{',start);
        if(startPos!=std::string::npos && startPos<eqPos) {
            std::string::size_type  endPos=exprList.find('}',start);
            if(endPos!=(eqPos-1)) {
                FatalErrorIn("CommonValueExpressionDriver::addVariables")
                    << "No closing '}' found in " 
                        << exprList.substr(start,eqPos-start)
                        << endl
                        << abort(FatalError);
            }
            word name(exprList.substr(start,startPos-start));
            string remoteExpr(exprList.substr(startPos+1,endPos-startPos-1));
            evaluateVariableRemote(remoteExpr,name,expr);
        } else {
            word name(exprList.substr(start,eqPos-start));
            evaluateVariable(name,expr);
        }
        start=end+1;
    }
}

void CommonValueExpressionDriver::readTables(Istream &is,HashTable<interpolationTable<scalar> > &tables,bool clear)
{
    if(clear) {
        tables.clear();
    }
    List<dictionary> lines(is);

    forAll(lines,i) {
        const dictionary &dict=lines[i];
        tables.insert(dict.lookup("name"),interpolationTable<scalar>(dict));
    }
}

void CommonValueExpressionDriver::writeTables(Ostream &os,const HashTable<interpolationTable<scalar> > &tables) const
{
    os << token::BEGIN_LIST << nl;
    forAllConstIter(HashTable<interpolationTable<scalar> >,tables,it) {
        os << token::BEGIN_BLOCK << nl;
        os.writeKeyword("name") << it.key() << token::END_STATEMENT << nl;
        (*it).write(os);
        os << token::END_BLOCK << nl;
    }
    os << token::END_LIST;
}

const fvMesh &CommonValueExpressionDriver::regionMesh
(
    const dictionary &dict,
    const fvMesh &mesh
)
{
    if(!dict.found("region")) {
        if(debug) {
            Info << "Using original mesh " << endl;
        }

        return mesh;
    }

    if(debug) {
        Info << "Using mesh " << dict.lookup("region")  << endl;
    }
    
    //     return dynamicCast<const fvMesh&>( // soesn't work with gcc 3.2
    return dynamic_cast<const fvMesh&>(
        mesh.time().lookupObject<objectRegistry>(
            dict.lookup("region")
        )
    );    
}

string CommonValueExpressionDriver::getTypeOfField(const string &name) const
{
    IOobject f 
        (
            name,
            mesh().time().timeName(),
            mesh(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );
    f.headerOk();

    if(debug) {
        Info << "Name: " << name << " Time: " << mesh().time().timeName() 
            << " Path: " << f.filePath() << " Class: " << f.headerClassName() << endl;
    }

    return f.headerClassName();
}

string CommonValueExpressionDriver::getTypeOfSet(const string &name) const
{
    if(debug) {
        Info << "Lookinh for set named " << name << endl;
    }
    IOobject f 
        (
            name,
            mesh().time().timeName(),
            polyMesh::meshSubDir/"sets",
	    mesh(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );
    
    if(f.headerOk()) {;
        return f.headerClassName();
    } else {
        Info << "No set " << name << " at t=" << mesh().time().timeName() 
            << " falling back to 'constant'" << endl;
        f=IOobject 
        (
            name,
            "constant",
            polyMesh::meshSubDir/"sets",
	    mesh(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );
        f.headerOk();
        return f.headerClassName();
    }
}

void CommonValueExpressionDriver::setTrace(
    bool scanning,
    bool parsing
)
{
    trace_parsing_=parsing;
    trace_scanning_=scanning;
}

void CommonValueExpressionDriver::outputResult(Ostream &o)
{
    word rType=getResultType();
    
    if(rType==pTraits<scalar>::typeName) {
        o << getResult<scalar>();
    } else if(rType==pTraits<vector>::typeName) {
        o << getResult<vector>();
    } else if(rType==pTraits<tensor>::typeName) {
        o << getResult<tensor>();
    } else if(rType==pTraits<symmTensor>::typeName) {
        o << getResult<symmTensor>();
    } else if(rType==pTraits<sphericalTensor>::typeName) {
        o << getResult<sphericalTensor>();
    } else { 
        o << "No implementation for " << rType;
    }
}

string CommonValueExpressionDriver::outputEntry()
{
    OStringStream o;
    
    word rType=getResultType();
    if(rType==pTraits<scalar>::typeName) {
        result_.getResult<scalar>(true)().writeEntry("",o);
    } else if(rType==pTraits<vector>::typeName) {
        result_.getResult<vector>(true)().writeEntry("",o);
    } else if(rType==pTraits<tensor>::typeName) {
        result_.getResult<tensor>(true)().writeEntry("",o);
    } else if(rType==pTraits<symmTensor>::typeName) {
        result_.getResult<symmTensor>(true)().writeEntry("",o);
    } else if(rType==pTraits<sphericalTensor>::typeName) {
        result_.getResult<sphericalTensor>(true)().writeEntry("",o);
    } else { 
        o << "No implementation for " << rType << ";";
    }

    return o.str();
}

const word CommonValueExpressionDriver::time() const
{
    return this->mesh().time().timeName();
}

const ExpressionResult &CommonValueExpressionDriver::lookupGlobal(
    const word &name
) const
{
    return GlobalVariablesRepository::getGlobalVariables().get(
        name,
        globalVariableScopes_
    );
}

void CommonValueExpressionDriver::setGlobalScopes(const wordList &other)
{
    globalVariableScopes_=other;
}

// ************************************************************************* //

} // namespace
