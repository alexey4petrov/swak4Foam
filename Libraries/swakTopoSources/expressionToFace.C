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

#include "expressionToFace.H"
#include "polyMesh.H"
#include "cellSet.H"
#include "Time.H"
#include "syncTools.H"
#include "addToRunTimeSelectionTable.H"

#include "FieldValueExpressionDriver.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

defineTypeNameAndDebug(expressionToFace, 0);

addToRunTimeSelectionTable(topoSetSource, expressionToFace, word);

addToRunTimeSelectionTable(topoSetSource, expressionToFace, istream);

}


Foam::topoSetSource::addToUsageTable Foam::expressionToFace::usage_
(
    expressionToFace::typeName,
    "\n    Usage: expressionToFace <expression>\n\n"
    "    Select all faces for which expression evaluates to true on one and false on the other side\n\n"
);

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::expressionToFace::combine(topoSet& set, const bool add) const
{
    if(Pstream::parRun()) {
        WarningIn("Foam::expressionToFace::combine(topoSet& set, const bool add) const")
            << " Does not give correct results if faces are on the processor boundary"
                << endl;
    }

    fvMesh mesh(set.db());

    FieldValueExpressionDriver driver
        (
            mesh.time().timeName(),
            mesh.time(),
            mesh,
            true, // cache stuff
            true, // search in memory
            true  // search on disc
        );
    driver.parse(expression_);
    if(!driver.resultIsLogical()) {
        FatalErrorIn("Foam::expressionToFace::combine(topoSet& set, const bool add) const")
            << "Expression " << expression_ << " does not evaluate to a logical expression"
                << endl
                << abort(FatalError);
    }
    const volScalarField &condition=driver.getScalar();

    const labelList &own=condition.mesh().faceOwner();
    const labelList &nei=condition.mesh().faceNeighbour();

    for(label faceI=0;faceI<condition.mesh().nInternalFaces();faceI++)
    {
        if (condition[own[faceI]] != condition[nei[faceI]])
        {
            addOrDelete(set, faceI, add);
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from componenta
Foam::expressionToFace::expressionToFace
(
    const polyMesh& mesh,
    const string& expression
)
:
    topoSetSource(mesh),
    expression_(expression)
{}


// Construct from dictionary
Foam::expressionToFace::expressionToFace
(
    const polyMesh& mesh,
    const dictionary& dict
)
:
    topoSetSource(mesh),
    expression_(dict.lookup("expression"))
{}


// Construct from Istream
Foam::expressionToFace::expressionToFace
(
    const polyMesh& mesh,
    Istream& is
)
:
    topoSetSource(mesh),
    expression_(checkIs(is))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::expressionToFace::~expressionToFace()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::expressionToFace::applyToSet
(
    const topoSetSource::setAction action,
    topoSet& set
) const
{
    if ((action == topoSetSource::ADD) || (action == topoSetSource::NEW))
    {
        Info<< "    Adding all elements of for which " << expression_ << " evaluates to true ..."
            << endl;
        
        combine(set,true);
    }
    else if (action == topoSetSource::DELETE)
    {
        Info<< "    Removing all elements of for which " << expression_ << " evaluates to true ..."
            << endl;

        combine(set,false);
    }
}


// ************************************************************************* //
