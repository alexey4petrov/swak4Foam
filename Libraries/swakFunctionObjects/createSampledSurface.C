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

#include "createSampledSurface.H"

#include "SurfacesRepository.H"

namespace Foam {
    defineTypeNameAndDebug(createSampledSurface,0);
}

Foam::createSampledSurface::createSampledSurface
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
):
    active_(true),
    obr_(obr)
{
    if (!isA<fvMesh>(obr))
    {
        active_=false;
        WarningIn("createSampledSurface::createSampledSurface")
            << "Not a fvMesh. Nothing I can do"
                << endl;
    }
    read(dict);
    execute();
}

Foam::createSampledSurface::~createSampledSurface()
{}

void Foam::createSampledSurface::read(const dictionary& dict)
{
    if(active_) {
        SurfacesRepository::getRepository().getSurface(
            dict,
            dynamic_cast<const fvMesh &>(obr_)
        );
    }
}

void Foam::createSampledSurface::execute()
{
}


void Foam::createSampledSurface::end()
{
}

void Foam::createSampledSurface::write()
{
}

void Foam::createSampledSurface::clearData()
{
}

// ************************************************************************* //
