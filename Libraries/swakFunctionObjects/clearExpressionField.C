//  OF-extend Revision: $Id$ 
/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright  held by original author
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

\*---------------------------------------------------------------------------*/

#include "clearExpressionField.H"

#include "FieldValueExpressionDriver.H"

#include "expressionFieldFunctionObject.H"

namespace Foam {
    defineTypeNameAndDebug(clearExpressionField,0);
}

Foam::clearExpressionField::clearExpressionField
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    obr_(obr)
{
    read(dict);
}

Foam::clearExpressionField::~clearExpressionField()
{}

void Foam::clearExpressionField::read(const dictionary& dict)
{
    name_=word(dict.lookup("fieldName"));
}

void Foam::clearExpressionField::execute()
{
    const functionObjectList &fol=obr_.time().functionObjects();
    bool found=false;

    forAll(fol,i) {
        if(isA<expressionFieldFunctionObject>(fol[i])) {
            expressionField &ef=const_cast<expressionField &>(
                //                dynamicCast<const expressionFieldFunctionObject&>(fol[i]).outputFilter() // doesn't work with gcc 4.2
                dynamic_cast<const expressionFieldFunctionObject&>(fol[i]).outputFilter()
            );

            if(ef.name()==name_) {
                found=true;                
                ef.clearData();
            }
        }
    }

    if(!found) {
        WarningIn("clearExpressionField::execute()")
            << "No function object named " << name_ << " found" 
                << endl;
    }
}


void Foam::clearExpressionField::end()
{
}

void Foam::clearExpressionField::write()
{
}

// ************************************************************************* //
