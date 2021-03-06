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
    \\  /    A nd           | Copyright  held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is based on OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

 ICE Revision: $Id$ 
\*---------------------------------------------------------------------------*/

#include "forceEquation.H"
#include "polyMesh.H"
#include "cellSet.H"

#include "FieldValueExpressionDriver.H"

namespace Foam {

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from dictionary
template<class T>
forceEquation<T>::forceEquation
(
    const dictionary& dict,
    const fvMesh& mesh
)
:
    FieldValueExpressionDriver(dict,mesh),
    valueExpression_(dict.lookup("valueExpression")),
    maskExpression_(dict.lookup("maskExpression")),
    verbose_(dict.lookupOrDefault<bool>("verbose",true))
{
    if(verbose_) {
        WarningIn(string("forceEquation<") + pTraits<T>::typeName + ">::forceEquation") 
            << "Fixing to the values " << valueExpression_ 
                << " with the mask " << maskExpression_ 
                << " will be verbose. To switch this off set the parameter 'verbose'"
                << "to false" << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class T>
forceEquation<T>::~forceEquation()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
bool forceEquation<T>::getMask(DynamicList<label> &cellIDs,const word &psi)
{
    parse(maskExpression_);
    if(!resultIsLogical()) {
        FatalErrorIn("forceEquation<scalar>::operator()(fvMatrix<T> &)")
            << "Result of " << maskExpression_ << " is not a logical expression"
                << endl
                << abort(FatalError);
    }

    const volScalarField &cond=getScalar();

    forAll(cond,cellI) {
        if(cond[cellI]!=0) {
            cellIDs.append(cellI);
        }
    }

    cellIDs.shrink();
    label size=cellIDs.size();
    reduce(size,plusOp<label>());

    if(size==0) {
        if(verbose_) {
            Info << "No cells fixed for field " << psi << endl;
        }
        return false;
    }
    if(verbose_) {
        Info << size << " cells fixed for field " << psi << endl;
    }

    return true;
}

template<class T>
tmp<volScalarField> forceEquation<T>::getMask()
{
    clearVariables();
    parse(maskExpression_);

    return tmp<volScalarField>
        (
            new volScalarField
            (
                getScalar()
            )
        );
}

template<>
void forceEquation<scalar>::operator()(fvMatrix<scalar> &eq)
{
    clearVariables();

    DynamicList<label> cellIDs;

    if(!getMask(cellIDs,eq.psi().name())) {
        return;
    }

    Field<scalar> values(cellIDs.size());

    parse(valueExpression_);
    const volScalarField &calculated=getScalar();

    forAll(cellIDs,i) {
        values[i]=calculated[cellIDs[i]];
    }

    eq.setValues(cellIDs,values);
}

template<>
void forceEquation<vector>::operator()(fvMatrix<vector> &eq)
{
    clearVariables();

    DynamicList<label> cellIDs;

    if(!getMask(cellIDs,eq.psi().name())) {
        return;
    }

    Field<vector> values(cellIDs.size());

    parse(valueExpression_);
    const volVectorField &calculated=getVector();

    forAll(cellIDs,i) {
        values[i]=calculated[cellIDs[i]];
    }

    eq.setValues(cellIDs,values);
}

// Catch all for those not implemented
template<class T>
void forceEquation<T>::operator()(fvMatrix<T> &)
{
    FatalErrorIn(
        "forceEquation<T>operator()(fvMatrix<T> &)"
    )
        <<  "not implemented for for T="
            << pTraits<T>::typeName
            << endl
            << abort(FatalError);
}

template
class forceEquation<scalar>;

template
class forceEquation<vector>;

template
class forceEquation<tensor>;

template
class forceEquation<symmTensor>;

template
class forceEquation<sphericalTensor>;

} // end namespace

// ************************************************************************* //
