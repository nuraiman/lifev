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

/*!
    @file
    @brief File containing a class to  deal the time advancing scheme.
    This class consider \f$\theta\f$-method for first order problems and
    TimeAdvanceNewmark scheme for the second order problems.

    @date

    @author Matteo Pozzoli <matteo1.pozzoli@mail.polimi.it>
    @contributor Matteo Pozzoli <matteo1.pozzoli@mail.polimi.it>
    @maintainer Matteo Pozzoli <matteo1.pozzoli@mail.polimi.it>
*/

#ifndef TimeAdvanceNewmark_H
#define TimeAdvanceNewmark_H 1

// Tell the compiler to ignore specific kind of warnings:
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cmath>

// Tell the compiler to ignore specific kind of warnings:
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <lifev/core/fem/TimeAdvance.hpp>
#include <lifev/core/array/VectorEpetra.hpp>

namespace LifeV
{
//! TimeAdvanceNewmark  - Class to deal the \f$theta\f$-method and TimeAdvanceNewmark scheme
/*!
  @author Matteo Pozzoli <matteo1.pozzoli@mail.polimi.it>

  This class can be used to approximate problems of the first order and the second order in time.
  In the first case the temporal scheme is a theta-method, while in the second case is a TimeAdvanceNewmark scheme.

  This class defines the state vector \f$X^{n+1}\f$, a suitable  extrapolation of vector \f$X^*\f$ and   opportune coefficients used to determinate \f$X^{n+1}\f$ and \f$X^*\f$.
<ol>
<li> First order problem:

   \f[ M u' + A(u) = f \f]

   the state vector is \f[X^{n+1} = (U^{n+1},V^{n+1}, U^{n}, V^{n})\f].
   where \f$U\f$ is an approximation of \f$u\f$ and \f$V\f$  of \f$\dot{u}\f$.

   We consider a parameter \f$\theta\f$, and we apply the following theta method:

   \f[ U^{n+1} = U^{n} + \Delta t  \theta V^{n+1} + (1 − \theta) V^n,  \f]

    so the approximation of velocity at timestep \f$n+1\f$ is:

    \f[  V^{ n+1} = 1/ (\theta * \Delta t) (U^{n+1}-U^n)+( 1 -  1 / \theta) V^n;\f]

    We can  linearize non-linear term \f$A(u)\f$  as \f$A(U^*) U^{n+1}\f$,  where the \f$U^*\f$ becomes:

    \f[ U^∗ = U^n + \Delta t V^n, \f]

    The coefficients \f$\alpha\f$, \f$beta\f$ depend on \f$theta\f$,
     and we can use a explicit \f$theta\f$-method.
     </li>
    <li> Second order Method:

     \f[ M \ddot{u} + D(u, \dot{u})+ A(u) = f \f]

     the state vector is \f[X^{n+1} = (U^{n+1},V^{n+1}, W^{n+1}, U^{n}, V^{n}, W^{n}).\f]
     where U is an approximation of \f$u\f$ and \f$V\f$  of \f$\dot{u}\f$ and \f$W\f$ of \f$\ddot{u}\f$ .

     We introduce the parameters (\f$\theta\f$, \f$\gamma\f$)  and we apply the following TimeAdvanceNewmark method:

     \f[ U^{n+1} = U^{n} + \Delta t  \theta V^{n+1} + (1 − \theta) V^n,  \f]

     so the approximation of velocity at time step \f$n+1\f$ is

     \f[ V^{ n+1} = \beta / (\theta * \Delta t) (U^{n+1}-U^n)+( 1 - \gamma/ \theta) V^n+ (1- \gamma /(2\theta)) \Delta t W^n;\f]

     and we determine \f$W^{n+1}\f$ by

     \f[ W^{n+1} =1/(\theta \Delta t^2) (U^{n+1}-U^{n}) - 1 / \Delta t V^n + (1-1/(2\theta)) W^n \f].

     We can  linearize non-linear term \f$A(u)\f$  as \f$A(U^*) U^{n+1}\f$ and  \f$D(u, \dot{u})\f$ as \f$D(U^*, V^*) V^{n+1}\f$,
     where \f$U^*\f$ is given by

     \f[ U^∗ = U^n + \Delta t V^n \f]

      and  \f$V^*\f$ is

      \f[  V^* =V^n+ \Delta t W^n  \f]

      The coefficients \f$\xi\f$, \f$\alpha\f$, \f$\beta\f$, \f$\beta_V\f$ depend on \f$\theta\f$ and \f$\gamma\f$.
      </li>
      </ol>
*/

template<typename feVectorType = VectorEpetra >
class TimeAdvanceNewmark :
        public TimeAdvance < feVectorType >
{
public:

    //! @name Public Types
    //@{

    typedef TimeAdvance< feVectorType >                    super;
    typedef typename super::feVectorContainer_Type         feVectorContainer_Type ;
    typedef typename super::feVectorContainerPtr_Type      feVectorContainerPtr_Type;
    typedef typename feVectorContainerPtr_Type::iterator   feVectorContainerPtrIterate_Type;

    //@}

    //! @name Constructor & Destructor
    //@{

    //! Empty  Constructor
    TimeAdvanceNewmark();

     //! Destructor
    ~TimeAdvanceNewmark() {}

    //@}

    //! @name Methods
    //@{

    //!Update the state vector
    /*! Update the vectors of the previous time steps by shifting on the right  the old values.
    @param solution current (new) value of the state vector
    */
    void shiftRight(const feVectorType& solution);

    //! Update the right hand side \f$ f_V \f$ of the time derivative formula
    /*!
    Set and Return the right hand side \f$ f_V \f$ of the time derivative formula
    @param timeStep defined the  time step need to compute the
    */
    feVectorType updateRHSFirstDerivative(const Real& timeStep = 1 );

    //! Update the right hand side \f$ f_W \f$ of the time derivative formula
    /*
    Set and Return the right hand side \f$ f_W \f$ of the time derivative formula
    @param timeStep defined the  time step need to compute the \f$ f_W \f$
    */
   feVectorType updateRHSSecondDerivative(const Real& timeStep = 1 );

    //!Show the properties  of temporal scheme
    void showMe() const;

    //@}

    //!@name Set Methods
    //@{

     //! Initialize the parameters of time advance scheme
     /*
     Initialize parameters of time advance scheme used in TimeAdvanceNewmark scheme
     @param  coefficients define the TimeAdvanceNewmark's coefficients
     @param  orderDerivate  define the order of derivate;
     */
     void setup (const  std::vector<Real>&  coefficients, const  UInt& orderDerivate);

     //! initialize parameters of time advance scheme;
     /*
     Initialize parameters of time advance scheme used in BDF;
     @param  order define the order of BDF;
     @param  orderDerivate  define the order of derivate;
     */
     void setup ( const UInt& order, const  UInt& orderDerivate);

     //! Initialize the StateVector
     /*!
     Initialize all the entries of the unknown vector to be derived with the vector x0 (duplicated).
     this class is virtual because used in bdf;
     @param x0 is the initial solution;
     */
     void setInitialCondition( const feVectorType& x0);

     //! initialize the state vector
     /*!
     Initialize all the entries of the unknown vector to be derived with the vector x0, v0 (duplicated).
     this class is virtual because used in \f$\theta\f$-method scheme;
     @param x0 is the initial unk;
     @param v0 is the initial velocity
     */
     void setInitialCondition( const feVectorType& x0, const feVectorType& v0 );

     //! initialize the state vector
     /*!
     Initialize all the entries of the unknown vector to be derived with the vector x0, v0,w0 (duplicated).
     this class is virtual because used in Newamrk scheme;
     @param x0 is the initial solution;
     @param v0 is the initial velocity
     @param w0 is the initial accelerate
     */
     void setInitialCondition(const feVectorType& x0, const feVectorType& v0, const feVectorType& w0 );

    //! Initialize the state vector
    /*! Initialize all the entries of the unknown vector to be derived with a
     set of vectors x0
     note: this is taken as a copy (not a reference), since x0 is resized inside the method.
     */
    void setInitialCondition( const feVectorContainer_Type& x0);

   //@}

  //!@name Get Methods
  //@{

   //!Return the \f$i\f$-th coefficient of the unk's extrapolation
  /*!
   @param i index of  extrapolation coefficient
   @returns beta
   */
   Real coefficientExtrapolation(const  UInt& i ) const;

   //! Return the \f$i\f$-th coefficient of the velocity's extrapolation
   /*!
   @param \f$i\f$ index of velocity's extrapolation  coefficient
   @returns \f$\beta^V_i\f$
   */
   Real coefficientExtrapolationVelocity(const UInt& i ) const;

    //! Compute the polynomial extrapolation of solution
    /*!
    Compute the polynomial extrapolation approximation of order \f$n-1\f$ of
    \f$u^{n+1}\f$ defined by the n stored state vectors
    */
   feVectorType   extrapolation() const;

    //! Compute the polynomial extrapolation of velocity
    /*!
    Compute the polynomial extrapolation approximation of order \f$n-1\f$ of
    \f$u^{n+1}\f$ defined by the n stored state vectors
    */
    feVectorType  extrapolationVelocity() const;

    //! Return the current velocity
    feVectorType velocity()  const;

    //!Return the current accelerate
    feVectorType accelerate() const ;

  //@}

private:

    //! Coefficient of TimeAdvanceNewmark: \f$theta\f$
    Real M_theta;

    //! Coefficient of TimeAdvanceNewmark: \f$\gamma\f$
    Real M_gamma;

};

// ==================================================
// Constructors & Destructor
// ==================================================
template<typename feVectorType>
TimeAdvanceNewmark <feVectorType> ::TimeAdvanceNewmark():super()
  {
  }

// ===================================================
// Methods
// ===================================================
template<typename feVectorType>
void TimeAdvanceNewmark <feVectorType>::shiftRight(const feVectorType& solution)
{
    ASSERT (  this->M_timeStep != 0 ,  "M_timeStep must be different to 0");

    feVectorContainerPtrIterate_Type it   =  this->M_unknowns.end();
    feVectorContainerPtrIterate_Type itb1 =  this->M_unknowns.begin() +  this->M_size/2;
    feVectorContainerPtrIterate_Type itb  =  this->M_unknowns.begin();

    for ( ; itb1 != it; ++itb1, ++itb)
        *itb1 = *itb;

    itb  =  this->M_unknowns.begin();

    // insert unk in unknowns[0];
    *itb = new feVectorType( solution );

    itb++;

    //update velocity
    feVectorType velocityTemp(solution);
    velocityTemp *=  this->M_alpha[0] / this->M_timeStep;
    velocityTemp -= *this->M_rhsContribution[0];

    // update unknows[1] with velocityTemp is current velocity
    *itb = new feVectorType(velocityTemp);

    if ( this->M_orderDerivate == 2 )
    {
     itb++;

      //update accelerate
      feVectorType accelerateTemp( solution );
      accelerateTemp *= this->M_xi[ 0 ] / ( this->M_timeStep * this->M_timeStep);
      accelerateTemp -= * this->M_rhsContribution[ 1 ];

      *itb = new feVectorType( accelerateTemp );
    }
    return;
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::updateRHSFirstDerivative(const Real& timeStep )
{
    feVectorContainerPtrIterate_Type it  =  this->M_rhsContribution.begin();

    feVectorType fv(* this->M_unknowns[0]);

    fv *=  this->M_alpha[ 1 ] / timeStep ;

    for (UInt i= 1; i  < this->M_firstOrderDerivateSize; ++i )
        fv += ( this->M_alpha[ i+1 ] * std::pow( timeStep, static_cast<Real>(i - 1 ) ) ) *  (* this->M_unknowns[ i ]);

    *it = new feVectorType(fv);

    return fv;
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::updateRHSSecondDerivative(const Real& timeStep )
{
    feVectorContainerPtrIterate_Type it  =  this->M_rhsContribution.end()-1;

    feVectorType fw(* this->M_unknowns[0]);

    fw *=  this->M_xi[ 1 ] /(timeStep * timeStep) ;

    for ( UInt i = 1;  i < this->M_secondOrderDerivateSize; ++i )

      fw += ( this->M_xi[ i+1 ] * std::pow(timeStep, static_cast<Real>(i - 2) ) ) * ( *this->M_unknowns[ i ]);

    *it = new feVectorType(fw);

    return fw;
}

template<typename feVectorType>
void
TimeAdvanceNewmark<feVectorType>::showMe() const
{
    std::cout << "*** TimeAdvanceNewmarkTime discretization maximum order of derivate "
                    << this->M_orderDerivate<< " ***"<< std::endl;
    std::cout <<" Coefficients : "      <<  std::endl;
    std::cout <<" theta :        "      << M_theta<<"\n"
              <<" gamma :  "            <<  M_gamma<<"\n"
              <<" size unknowns :"      << this->M_size<<"\n";

    for ( UInt i = 0; i <  this->M_alpha.size(); ++i )
        std::cout << "  alpha(" << i << ") = " <<  this->M_alpha[ i ]<< std::endl;

    if (this->M_orderDerivate == 2)
    {
        for ( UInt i = 0; i <  this->M_xi.size(); ++i )
            std::cout << "       xi(" << i << ") = " <<  this->M_xi[ i ] << std::endl;
    }

    std::cout <<"Delta Time : "<< this->M_timeStep<<"\n";
    std::cout <<"*************************************\n";
}

// ===================================================
// Set Methods
// ===================================================

template<typename feVectorType>
void
TimeAdvanceNewmark<feVectorType>::setup (const UInt& /*order*/, const  UInt& /*orderDerivate*/)
{
    ERROR_MSG("use setup for BDF but the time advance scheme is TimeAdvanceNewmark or  theta-method");
}

template<typename feVectorType>
void
TimeAdvanceNewmark<feVectorType>::setup(const std::vector<Real>& coefficients, const  UInt& orderDerivate)
{
    //initialize theta
    M_theta = coefficients[0];

    //initilialize gamma
    M_gamma = coefficients[1];

    //initialize Order Derivate
    this->M_orderDerivate= orderDerivate;

    // If theta equal 0, explicit meta method
    if (M_theta == 0)
    {
        ASSERT (this->M_orderDerivate == 2,  "theta is 0 must be different from 0 in TimeAdvanceNewmark");
        this->M_size = 4;
        this->M_alpha[ 0 ] =  1;
        this->M_alpha[ 1 ] =  1;
        this->M_alpha[ 2 ] =  1;
        this->M_order  = 1;
    }
    else
    {
        if (this->M_orderDerivate == 1 )  // Theta method
        {
            this->M_gamma = 1;
            //  unknown vector's  dimension;
            this->M_size = 4;
            this->M_alpha.resize(3);
            this->M_xi.resize(3);
            this->M_beta.resize(3);
            this->M_alpha[ 0 ] =  M_gamma / M_theta;
            this->M_alpha[ 1 ] =  M_gamma / M_theta;
            this->M_alpha[ 2 ] =  M_gamma / M_theta - 1.0;
            this->M_beta[0] = 1;
            this->M_beta[1] = 1;
            this->M_beta[2] = 0.5;
            this->M_xi[0]   = 0;
            this->M_xi[1]   = 0;
            this->M_xi[2]   = 0;
            this->M_coefficientsSize = 3;
        }
        else     //TimeAdvanceNewmarkMethod
        {
            //unknown vector's dimension
            this->M_size = 6 ;
            this->M_alpha.resize(4);
            this->M_xi.resize(4);
            this->M_beta.resize(3);
            this->M_betaVelocity.resize(3);
            //initialitation alpha coefficients
            this->M_alpha[ 0 ] =  M_gamma / M_theta;
            this->M_alpha[ 1 ] =  M_gamma / M_theta;
            this->M_alpha[ 2 ] =  M_gamma / M_theta - 1.0;
            this->M_alpha[ 3 ] = M_gamma / (2.0 * M_theta) -1.0;

            //initialitation xi coefficients
            this->M_xi[ 0 ] =  1. / M_theta;
            this->M_xi[ 1 ] =  1. / M_theta;
            this->M_xi[ 2 ] =  1. / M_theta;
            this->M_xi[ 3 ] =  1. / ( 2.0 * M_theta )-1.0;


            //initialitation extrap coefficients
            this->M_beta[ 0 ] = 1;
            this->M_beta[ 1 ] = 1;
            this->M_beta[ 2 ] = 0.5;
            this->M_betaVelocity[ 0 ] = 0;
            this->M_betaVelocity[ 1 ] = 1;
            this->M_betaVelocity[ 2 ] = 1;

            this->M_coefficientsSize  = 4;
        }
        this->M_unknowns.reserve(this->M_size);
        this-> M_rhsContribution.reserve(2);
        // check order  scheme
        if (this->M_alpha[0] == 0.5)
            this->M_order = 2;
        else
            this->M_order = 1;

        this->M_firstOrderDerivateSize  =  static_cast<Real>(this->M_size) / 2.0;
        this->M_secondOrderDerivateSize = static_cast<Real>(this->M_size) / 2.0;
    }
}

template<typename feVectorType>
void TimeAdvanceNewmark<feVectorType>::setInitialCondition( const feVectorType& x0)
{
    feVectorContainerPtrIterate_Type iter     = this->M_unknowns.begin();
    feVectorContainerPtrIterate_Type iter_end = this->M_unknowns.end();

    feVectorType zero(x0);
    zero *=0;

    for ( ; iter != iter_end; ++iter )
    {
        delete *iter;
        *iter = new feVectorType(zero, Unique);
    }

    for ( UInt i(this->M_unknowns.size()) ; i < this->M_size; ++i )
        this->M_unknowns.push_back(new feVectorType(x0));

    feVectorContainerPtrIterate_Type iterRhs     = this->M_rhsContribution.begin();
    feVectorContainerPtrIterate_Type iterRhs_end = this->M_rhsContribution.end();

    for ( ; iterRhs !=iterRhs_end ; ++iterRhs)
    {
        delete *iterRhs;
        *iterRhs = new feVectorType(zero, Unique);
    }
}

template<typename feVectorType>
void TimeAdvanceNewmark<feVectorType>::setInitialCondition( const feVectorType& x0, const feVectorType& v0)
{
    feVectorContainerPtrIterate_Type iter       = this->M_unknowns.begin();
    feVectorContainerPtrIterate_Type iter_end   = this->M_unknowns.end();

   //!initialize zero
    feVectorType zero(x0);
    zero *=0;

    for ( ; iter != iter_end; ++iter )
    {
        delete *iter;
        *iter = new feVectorType(zero, Unique);
    }

    this->M_unknowns.push_back(new feVectorType(x0));
    this->M_unknowns.push_back(new feVectorType(v0));

    for (UInt i=0; i<4; ++i )
    {
        this->M_unknowns.push_back(new feVectorType(zero, Unique));
        *this->M_unknowns[i+2]  *=0;
    }
    this->setInitialRHS(zero);
}

template<typename feVectorType>
void TimeAdvanceNewmark<feVectorType>::setInitialCondition( const feVectorType& x0, const feVectorType& v0, const feVectorType& w0)
{
    feVectorContainerPtrIterate_Type iter       = this->M_unknowns.begin();
    feVectorContainerPtrIterate_Type iter_end   = this->M_unknowns.end();

  //!initialize zero
    feVectorType zero(x0);
    zero *=0;

    for ( ; iter != iter_end; ++iter )
    {
        delete *iter;
        *iter = new feVectorType(zero, Unique);
    }

    this->M_unknowns.push_back(new feVectorType(x0));
    this->M_unknowns.push_back(new feVectorType(v0));
    this->M_unknowns.push_back(new feVectorType(w0));

    for (UInt i=0; i<3; ++i )
    {
        this->M_unknowns.push_back(new feVectorType(zero, Unique));
        *this->M_unknowns[i+3] *=0;
    }
    this->setInitialRHS(zero);
}

template<typename feVectorType>
void TimeAdvanceNewmark<feVectorType>::setInitialCondition( const feVectorContainer_Type& x0)
{
    const UInt n0 = x0.size();

    ASSERT( n0 != 0, "vector null " );

    feVectorContainerPtrIterate_Type iter     = this->M_unknowns.begin();
    feVectorContainerPtrIterate_Type iter_end = this->M_unknowns.end();

    UInt i(0);

    //!initialize zero
    feVectorType zero(x0[0]);
    zero *=0;

    for ( ; iter != iter_end && i< n0 ; ++iter, ++i )
    {
        delete *iter;
        *iter = new feVectorType(x0[i]);
    }

    for ( i = this->M_unknowns.size() ; i < this->M_size && i< n0; ++i )
        this->M_unknowns.push_back(new feVectorType(x0[i]));

    for ( i = this->M_unknowns.size() ; i < this->M_size; i++ )
        this->M_unknowns.push_back(new feVectorType( x0[ n0-1 ] ) );

    this->setInitialRHS(zero);
}

// ===================================================
// Get Methods
// ===================================================

template<typename feVectorType>
Real
TimeAdvanceNewmark<feVectorType>::coefficientExtrapolation(const UInt& i) const
{
  ASSERT ( i <  3 ,  "coeff_der i must equal 0 or 1 because U^*= U^n + timeStep*V^n + timeStep^2 / 2 W^n");
  return  this->M_beta(i)*std::pow( this->M_timeStep, static_cast<Real>(i) );
}

template<typename feVectorType>
Real
TimeAdvanceNewmark<feVectorType>::coefficientExtrapolationVelocity(const UInt& i ) const
{
 return  this->M_betaVelocity(i)*std::pow( this->M_timeStep, static_cast<Real>(i));
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::extrapolation()  const
{
    feVectorType extrapolation(*this->M_unknowns[0]);
    extrapolation += this->M_timeStep * ( *this->M_unknowns[ 1 ]);
    if ( this->M_orderDerivate == 2 )
        extrapolation += ( this->M_timeStep * this->M_timeStep ) / 2.0 * ( *this->M_unknowns[2]);
    return extrapolation;
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::extrapolationVelocity() const
{
    feVectorType extrapolation( *this->M_unknowns[1]);
    extrapolation += this->M_timeStep * ( *this->M_unknowns[ 2 ]);

    return extrapolation;
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::velocity() const
{
    feVectorType velocity( *this->M_unknowns[1]);
    return velocity;
}

template<typename feVectorType>
feVectorType
TimeAdvanceNewmark<feVectorType>::accelerate() const
{
    feVectorType accelerate( *this->M_unknowns[2]);
    return accelerate;
}

// ===================================================
// Macros
// ===================================================

//! define the TimeAdvanceNewmark factory
inline
TimeAdvance< VectorEpetra >* createTimeAdvanceNewmark() { return new TimeAdvanceNewmark<VectorEpetra>(); }

namespace
{
static bool registerTimeAdvanceNewmark= TimeAdvanceFactory::instance().registerProduct( "Newmark",  &createTimeAdvanceNewmark);
}

}
#endif  /*TimeAdvanceNewmark*/
