//@HEADER
/*
*******************************************************************************

    Copyright (C) 2004, 2005, 2007 EPFL, Politecnico di Milano, INRIA
    Copyright (C) 2010 EPFL, Politecnico di Milano, Emory University

    This file is part of LifeV.

    LifeV is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LifeV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LifeV.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************
*/
//@HEADER

#include <lifev/fsi/solver/FSIFixedPoint.hpp>

namespace LifeV
{

// ===================================================
// Constructors & Destructor
// ===================================================

FSIFixedPoint::FSIFixedPoint():
        super(),
        M_nonLinearAitken(),
        M_rhsNew(),
        M_beta()
{
}


FSIFixedPoint::~FSIFixedPoint()
{}


// ===================================================
// Methods
// ===================================================

void  FSIFixedPoint::solveJac(vector_Type        &muk,
                           const vector_Type  &res,
                           const Real   /*_linearRelTol*/)
{
    M_nonLinearAitken.restart();

    if (M_data->algorithm()=="RobinNeumann")
    {
        muk = M_nonLinearAitken.computeDeltaLambdaScalar(this->lambdaSolidOld(), res);
    }
    else
    {
        muk = M_nonLinearAitken.computeDeltaLambdaScalar(this->lambdaSolidOld(), -1.*res);
    }
}

void FSIFixedPoint::evalResidual(vector_Type &res, const vector_Type& disp, UInt iter)
{

    if (this->isSolid())
    {
        std::cout << "*** Residual computation g(x_" << iter <<" )";
        if (iter == 0) std::cout << " [NEW TIME STEP] ";
        std::cout << std::endl;
    }

    this->setLambdaSolidOld(disp);

    eval(disp, iter);

    res  = disp;
    res -= this->lambdaSolid();
}


void
FSIFixedPoint::setupFEspace()
{
    super::setLinearFluid(false);
    super::setLinearSolid(false);

    super::setupFEspace();
}



void
FSIFixedPoint::setupFluidSolid()
{
    // call FSI setup()

    debugStream( 6205 ) << "Setting up the FSI problem \n";

    super::setLinearFluid(false);
    super::setLinearSolid(false);

    super::setupFluidSolid();

    if ( this->isFluid() )
    {
        M_rhsNew.reset(new vector_Type(this->M_fluid->getMap()));
        M_beta.reset(new vector_Type(this->M_fluid->getMap()));
    }

}

void
FSIFixedPoint::setDataFile( GetPot const& dataFile )
{
    super::setDataFile( dataFile );

    M_nonLinearAitken.setDefaultOmega(M_data->defaultOmega(), 0.001);
    M_nonLinearAitken.setOmegaRange( M_data->OmegaRange() );

    if ( M_data->algorithm() == "RobinNeumann" )
        M_nonLinearAitken.setDefaultOmega(-1, 1);

}

// ===================================================
// Private Methods
// ===================================================

void FSIFixedPoint::eval( const vector_Type& _disp,
                       UInt                iter)
{
    // If M_data->updateEvery() == 1, normal fixedPoint algorithm
    // If M_data->updateEvery()  > 1, recompute computational domain every updateEvery iterations (transpiration)
    // If M_data->updateEvery() <= 0, recompute computational domain and matrices only at first subiteration (semi-implicit)
    bool recomputeMatrices ( iter == 0 || ( M_data->updateEvery() > 0 && iter % M_data->updateEvery() == 0 ) );

    std::cout << "recomputeMatrices = " << recomputeMatrices << " == " << true
              << "; iter = " << iter
              << "; updateEvery = " << M_data->updateEvery() << std::endl;

    if (iter == 0 && this->isFluid())
    {
        this->M_fluid->resetPreconditioner();
        //this->M_solid->resetPrec();
    }



    M_epetraWorldComm->Barrier();

    // possibly unsafe when using more cpus,
    this->setLambdaFluid(_disp);

    //Change in sign in the residual for RN:
    // if(this->algorithm()=="RobinNeumann")   this->setMinusSigmaFluid( sigmaSolidUnique );
    if (M_data->algorithm()=="RobinNeumann")   this->setMinusSigmaFluid( this->sigmaSolid() );


    vector_Type sigmaFluidUnique (this->sigmaFluid().map(), Unique);

    if (this->isFluid())
    {
        this->M_meshMotion->iterate(*M_BCh_mesh);

        this->transferMeshMotionOnFluid(M_meshMotion->disp(),
                                        this->veloFluidMesh());

        this->veloFluidMesh()    -= this->dispFluidMeshOld();
        this->veloFluidMesh()    *= 1./(M_data->dataFluid()->dataTime()->timeStep());

        // copying displacement to a repeated indeces displacement, otherwise the mesh wont know
        // the value of the displacement for some points

        vector_Type const meshDisplacement( M_meshMotion->disp(), Repeated );
        this->moveMesh(meshDisplacement);
        /*
        vector_Type const meshDisplacement( M_meshMotion->dispDiff(), Repeated );
        this->moveMesh(meshDispDiff);
        */

        *M_beta *= 0.;

        this->transferMeshMotionOnFluid(M_meshMotion->dispDiff(),  *M_beta);

        *M_beta *= -1./M_data->dataFluid()->dataTime()->timeStep();

        *M_beta += this->M_bdf->extrapolation();

        double alpha = this->M_bdf->coefficientFirstDerivative( 0 ) / M_data->dataFluid()->dataTime()->timeStep();

        //*M_rhsNew   = *this->M_rhs;
        //*M_rhsNew  *= alpha;


        if (recomputeMatrices)
        {

            this->M_fluid->updateSystem( alpha, *M_beta, *M_rhs );
        }
        else
        {
            this->M_fluid->updateRightHandSide( *M_rhs );
        }

        //	if(this->algorithm()=="RobinNeumann") this->updatealphaf(this->veloFluidMesh());// this->setAlphaf();

        this->M_fluid->iterate( *M_BCh_u );

        std::cout << "Finished iterate, transfering to interface \n" << std::flush;

        this->transferFluidOnInterface(this->M_fluid->residual(), sigmaFluidUnique);

    }

    M_epetraWorldComm->Barrier();

    if ( true && this->isFluid() )
    {
        vector_Type vel  (this->fluid().velocityFESpace().map());
        vector_Type press(this->fluid().pressureFESpace().map());

        vel.subset(*this->M_fluid->solution());
        press.subset(*this->M_fluid->solution(), this->fluid().velocityFESpace().dim()*this->fluid().pressureFESpace().fieldDim());

        std::cout << "norm_inf( vel ) " << vel.normInf() << std::endl;
        std::cout << "norm_inf( press ) " << press.normInf() << std::endl;

    }



    this->setSigmaFluid( sigmaFluidUnique );
    this->setSigmaSolid( sigmaFluidUnique );

    M_epetraWorldComm->Barrier();


    vector_Type lambdaSolidUnique   (this->lambdaSolid().map(),    Unique);
    vector_Type lambdaDotSolidUnique(this->lambdaDotSolid().map(), Unique);
    vector_Type sigmaSolidUnique    (this->sigmaSolid().map(),     Unique);

    if (this->isSolid())
    {
        this->M_solid->iterate( M_BCh_d );
        this->transferSolidOnInterface(this->M_solid->getDisplacement(),     lambdaSolidUnique);
        this->transferSolidOnInterface(this->M_solid->getVelocity(),      lambdaDotSolidUnique);
        this->transferSolidOnInterface(this->M_solid->getResidual(), sigmaSolidUnique);
    }

    this->setLambdaSolid( lambdaSolidUnique );
    this->setLambdaDotSolid( lambdaDotSolidUnique );
    this->setSigmaSolid( sigmaSolidUnique );

    M_epetraWorldComm->Barrier();


    // Some displays:
    Real norm;

    norm = _disp.normInf();
    if (this->isSolid() && this->isLeader())
        std::cout << " ::: norm(disp     )    = " << norm << std::endl;

    norm = this->lambdaSolid().normInf();
    if (this->isSolid() && this->isLeader())
        std::cout << " ::: norm(dispNew  )    = " << norm << std::endl;

    norm = this->lambdaDotSolid().normInf();
    if (this->isSolid() && this->isLeader())
        std::cout << " ::: norm(velo     )    = " << norm << std::endl;

    norm = this->sigmaFluid().normInf();
    if (this->isSolid() && this->isLeader())
        std::cout << " ::: max Residual Fluid = " << norm << std::endl;

    norm = this->sigmaSolid().normInf();
    if (this->isSolid() && this->isLeader())
        std::cout << " ::: max Residual Solid = " << norm  << std::endl;

    if (this->isFluid())
    {
        norm = M_fluid->residual().normInf();
        if (this->isLeader())
            std::cout << "Max ResidualF        = " << norm << std::endl;
    }
    if (this->isSolid())
    {
        norm = M_solid->getDisplacement().norm2();
        if (this->isLeader())
            std::cout << "NL2 DiplacementS     = " << norm << std::endl;

        norm = M_solid->getResidual().normInf();

        if (this->isLeader())
            std::cout << "Max ResidualS        = " << norm << std::endl;
    }

}




void FSIFixedPoint::registerMyProducts( )
{
    FSIFactory_Type::instance().registerProduct( "fixedPoint", &createFP );
    solid_Type::StructureSolverFactory::instance().registerProduct( "LinearVenantKirchhof", &FSIOperator::createLinearStructure );
    //solid_Type::StructureSolverFactory::instance().registerProduct( "NonLinearVenantKirchhof", &FSI::createNonLinearStructure );
}

}   // Namespace LifeV
