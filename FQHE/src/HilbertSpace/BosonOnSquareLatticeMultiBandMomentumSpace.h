////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                                                            //
//                            DiagHam  version 0.01                           //
//                                                                            //
//                    Copyright (C) 2001-2011 Nicolas Regnault                //
//                                                                            //
//                                                                            //
//                          class of bosons on square lattice                 //
//                                  in momentum space                         //
//                                                                            //
//                        last modification : 16/09/2011                      //
//                                                                            //
//                                                                            //
//    This program is free software; you can redistribute it and/or modify    //
//    it under the terms of the GNU General Public License as published by    //
//    the Free Software Foundation; either version 2 of the License, or       //
//    (at your option) any later version.                                     //
//                                                                            //
//    This program is distributed in the hope that it will be useful,         //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           //
//    GNU General Public License for more details.                            //
//                                                                            //
//    You should have received a copy of the GNU General Public License       //
//    along with this program; if not, write to the Free Software             //
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#ifndef BOSONONSQUARELATTICEMULTIBANDMOMENTUMSPACE_H
#define BOSONONSQUARELATTICEMULTIBANDMOMENTUMSPACE_H

#include "config.h"
#include "HilbertSpace/BosonOnSphereShort.h"

#include <iostream>



class BosonOnSquareLatticeMultiBandMomentumSpace : public BosonOnSphereShort
{

  friend class FQHESquareLatticeMultiBandSymmetrizeU1U1StateOperation;
  friend class FQHESquareLatticeMultiBandSymmetrizeU1U1StateOperation;
  
 protected:

  // number of sites in the x direction
  int NbrSiteX;
  // number of sites in the y direction
  int NbrSiteY;
  // momentum along the x direction
  int KxMomentum;
  // momentum along the y direction
  int KyMomentum;

  int NbrBand;

 public:

  // default constructor
  // 
  BosonOnSquareLatticeMultiBandMomentumSpace ();

  // basic constructor
  // 
  // nbrBosons = number of bosons
  // nbrSiteX = number of sites in the x direction
  // nbrSiteY = number of sites in the y direction
  // kxMomentum = momentum along the x direction
  // kyMomentum = momentum along the y direction
  // memory = amount of memory granted for precalculations
  BosonOnSquareLatticeMultiBandMomentumSpace (int nbrBosons, int nbrSiteX, int nbrSiteY, int kxMomentum, int kyMomentum, int nbrBand, unsigned long memory = 10000000);

  // copy constructor (without duplicating datas)
  //
  // bosons = reference on the hilbert space to copy to copy
  BosonOnSquareLatticeMultiBandMomentumSpace(const BosonOnSquareLatticeMultiBandMomentumSpace& bosons);

  // destructor
  //
  ~BosonOnSquareLatticeMultiBandMomentumSpace ();

  // assignement (without duplicating datas)
  //
  // bosons = reference on the hilbert space to copy to copy
  // return value = reference on current hilbert space
  BosonOnSquareLatticeMultiBandMomentumSpace& operator = (const BosonOnSquareLatticeMultiBandMomentumSpace& bosons);

  // clone Hilbert space (without duplicating datas)
  //
  // return value = pointer to cloned Hilbert space
  AbstractHilbertSpace* Clone();

  // save Hilbert space description to disk
  //
  // fileName = name of the file where the Hilbert space description has to be saved
  // return value = true if no error occured
  virtual bool WriteHilbertSpace (char* fileName);

  // get the list of occupied orbitals in a given state
  //
  // state = ID of the state
  // orbitals = list of orbitals to be filled
  virtual void GetOccupied(int state, int* orbitals);

  // print a given State
  //
  // Str = reference on current output stream 
  // state = ID of the state to print
  // return value = reference on current output stream 
  virtual ostream& PrintState (ostream& Str, int state);
/*
  // evaluate a density matrix of a subsystem of the whole system described by a given ground state. The density matrix is only evaluated in a given momentum sector and fixed number of particles
  // 
  // subsytemSizeX = number of sites along the x direction that belong to the subsytem
  // subsytemSizeY = number of sites along the y direction that belong to the subsytem
  // subsytemStartX = x momentum marking the beginning of the rectangluar subsystem
  // subsytemStartY = y momentum marking the beginning of the rectangluar subsystem
  // nbrParticleSector = number of particles that belong to the subsytem 
  // kxSector = Kx momentum sector in which the entanglement matrix has to be evaluated 
  // kySector = Ky momentum sector in which the entanglement matrix has to be evaluated 
  // groundState = reference on the total system ground state
  // return value = density matrix of the subsytem
  virtual HermitianMatrix EvaluatePartialDensityMatrixMomentumSpace (int subsytemSizeX, int subsytemSizeY, int subsytemStartX, int subsytemStartY, int nbrParticleSector, int kxSector, int kySector, ComplexVector& groundState);

  // core part of the evaluation density matrix momentum space partition calculation
  // 
  // minIndex = first index to consider in complementary Hilbert space
  // nbrIndex = number of indices to consider in complementary Hilbert space
  // complementaryHilbertSpace = pointer to the complementary Hilbert space (i.e part B)
  // destinationHilbertSpace = pointer to the destination Hilbert space (i.e. part A)
  // groundState = reference on the total system ground state
  // densityMatrix = reference on the density matrix where result has to stored
  // return value = number of components that have been added to the density matrix
  virtual long EvaluatePartialDensityMatrixMomentumSpaceCore (int minIndex, int nbrIndex, ParticleOnSphere* complementaryHilbertSpace,  ParticleOnSphere* destinationHilbertSpace, ComplexVector& groundState,  HermitianMatrix* densityMatrix);
  
  // evaluate an entanglement matrix of a subsystem of the whole system described by a given ground state. The entanglement matrix is only evaluated in a given momentum sector and fixed number of particles
  // 
  // subsytemSizeX = number of sites along the x direction that belong to the subsytem
  // subsytemSizeY = number of sites along the y direction that belong to the subsytem
  // nbrParticleSector = number of particles that belong to the subsytem 
  // kxSector = Kx momentum sector in which the entanglement matrix has to be evaluated 
  // kySector = Ky momentum sector in which the entanglement matrix has to be evaluated 
  // groundState = reference on the total system ground state
  // return value = entanglement matrix of the subsytem
  virtual ComplexMatrix EvaluatePartialEntanglementMatrixMomentumSpace (int subsytemSizeX, int subsytemSizeY, int nbrParticleSector, int kxSector, int kySector, ComplexVector& groundState);
  
  // evaluate an entanglement matrix of a subsystem of the whole system described by a given ground state, using particle partition. The entanglement matrix is only evaluated in a given Lz sector.
  // 
  // nbrParticleSector = number of particles that belong to the subsytem 
  // kxSector = Kx momentum sector in which the entanglement matrix has to be evaluated 
  // kySector = Ky momentum sector in which the entanglement matrix has to be evaluated 
  // groundState = reference on the total system ground state
  // removeBinomialCoefficient = remove additional binomial coefficient in case the particle entanglement matrix has to be used for real space cut
  // return value = entanglement matrix of the subsytem (return a wero dimension matrix if the entanglement matrix is equal to zero)
  virtual ComplexMatrix EvaluatePartialEntanglementMatrixParticlePartition (int nbrParticleSector, int kxSector, int kySector, ComplexVector& groundState, bool removeBinomialCoefficient = false);

  // evaluate a density matrix of a subsystem of the whole system described by a given ground state, using particle partition. The density matrix is only evaluated in a given Lz sector.
  // 
  // nbrBosonSector = number of particles that belong to the subsytem 
  // lzSector = Lz sector in which the density matrix has to be evaluated 
  // groundState = reference on the total system ground state
  // architecture = pointer to the architecture to use parallelized algorithm 
  // return value = density matrix of the subsytem (return a wero dimension matrix if the density matrix is equal to zero)
  virtual HermitianMatrix EvaluatePartialDensityMatrixParticlePartition (int nbrParticleSector, int kxSector, int kySector, ComplexVector& groundState, AbstractArchitecture* architecture = 0);

  // evaluate a density matrix of a subsystem of the whole system described by a given sum of projectors, using particle partition. The density matrix is only evaluated in a given Lz sector.
  // 
  // nbrBosonSector = number of particles that belong to the subsytem 
  // lzSector = Lz sector in which the density matrix has to be evaluated 
  // nbrGroundStates = number of projectors
  // groundStates = array of degenerate groundstates associated to each projector
  // weights = array of weights in front of each projector
  // architecture = pointer to the architecture to use parallelized algorithm 
  // return value = density matrix of the subsytem (return a wero dimension matrix if the density matrix is equal to zero)
  virtual HermitianMatrix EvaluatePartialDensityMatrixParticlePartition (int nbrParticleSector, int kxSector, int kySector, 
									 int nbrGroundStates, ComplexVector* groundStates, double* weights, AbstractArchitecture* architecture = 0);
*/
  // find state index from an array
  //
  // stateDescription = array describing the state (stored as kx1,ky1,kx2,ky2,...)
  // return value = corresponding index, -1 if an error occured
  virtual int FindStateIndexFromArray(int* stateDescription);

  // symmetrized a product of two uncoupled states 
  //
  // outputVector = reference on the vector which will contain the symmetrozed state
  // leftVector = reference on the vector associated to the first color
  // rightVector = reference on the vector associated to the second color
  // leftSpace = pointer to the Hilbert space of the first color
  // rightSpace = pointer to the Hilbert space of the second color
  // unnormalizedBasisFlag = assume evrything has to be done in the unnormalized basis
  // return value = symmetrized state

  ComplexVector SymmetrizeU1U1State (ComplexVector& leftVector, ComplexVector& rightVector, BosonOnSquareLatticeMultiBandMomentumSpace* leftSpace, BosonOnSquareLatticeMultiBandMomentumSpace* rightSpace, bool unnormalizedBasisFlag, AbstractArchitecture* architecture);


 protected:

  // evaluate Hilbert space dimension
  //
  // nbrBosons = number of bosons
  // currentKx = current momentum along x for a single particle
  // currentKy = current momentum along y for a single particle
  // currentTotalKx = current total momentum along x
  // currentTotalKy = current total momentum along y
  // return value = Hilbert space dimension
  virtual long EvaluateHilbertSpaceDimension(int nbrBosons, int currentKx, int currentKy, int currentTotalKx, int currentTotalKy, int band);

  // generate all states corresponding to the constraints
  // 
  // stateDescription = array that gives each state description
  // nbrBosons = number of bosons
  // currentKx = current momentum along x for a single particle
  // currentKy = current momentum along y for a single particle
  // currentTotalKx = current total momentum along x
  // currentTotalKy = current total momentum along y
  // currentFermionicPosition = current fermionic position within the state description
  // pos = position in StateDescription array where to store states
  // return value = position from which new states have to be stored
  virtual long GenerateStates(unsigned long* stateDescription, int nbrBosons, int currentKx, int currentKy, int currentTotalKx, int currentTotalKy, int currentFermionicPosition, long pos, int band);
/*
  // core part of the evaluation density matrix particle partition calculation
  // 
  // minIndex = first index to consider in complementary Hilbert space
  // nbrIndex = number of indices to consider in complementary Hilbert space
  // complementaryHilbertSpace = pointer to the complementary Hilbert space (i.e part B)
  // destinationHilbertSpace = pointer to the destination Hilbert space (i.e. part A)
  // groundState = reference on the total system ground state
  // densityMatrix = reference on the density matrix where result has to stored
  // return value = number of components that have been added to the density matrix
  virtual long EvaluatePartialDensityMatrixParticlePartitionCore (int minIndex, int nbrIndex, ParticleOnSphere* complementaryHilbertSpace,  ParticleOnSphere* destinationHilbertSpace,
								  ComplexVector& groundState,  HermitianMatrix* densityMatrix);

  // core part of the evaluation density matrix particle partition calculation involving a sum of projetors 
  // 
  // minIndex = first index to consider in source Hilbert space
  // nbrIndex = number of indices to consider in source Hilbert space
  // complementaryHilbertSpace = pointer to the complementary Hilbert space (i.e. part B)
  // destinationHilbertSpace = pointer to the destination Hilbert space  (i.e. part A)
  // nbrGroundStates = number of projectors
  // groundStates = array of degenerate groundstates associated to each projector
  // weights = array of weights in front of each projector
  // densityMatrix = reference on the density matrix where result has to stored
  // return value = number of components that have been added to the density matrix
  virtual long EvaluatePartialDensityMatrixParticlePartitionCore (int minIndex, int nbrIndex, ParticleOnSphere* complementaryHilbertSpace,  ParticleOnSphere* destinationHilbertSpace,
								  int nbrGroundStates, ComplexVector* groundStates, double* weights, HermitianMatrix* densityMatrix);
*/
  // symmetrized a product of two uncoupled states 
  //
  // outputVector = reference on the vector which will contain the symmetrozed state
  // leftVector = reference on the vector associated to the first color
  // rightVector = reference on the vector associated to the second color
  // leftSpace = pointer to the Hilbert space of the first color
  // rightSpace = pointer to the Hilbert space of the second color
  // unnormalizedBasisFlag = assume evrything has to be done in the unnormalized basis
  // return value = symmetrized state

  void SymmetrizeU1U1StateCore (ComplexVector& symmetrizedVector, ComplexVector& leftVector, ComplexVector& rightVector, BosonOnSquareLatticeMultiBandMomentumSpace* leftSpace, BosonOnSquareLatticeMultiBandMomentumSpace* rightSpace, bool unnormalizedBasisFlag, unsigned long firstComponent, unsigned long nbrComponents);

  // apply a^+_m1 a^+_m2 a_n1 a_n2 operator to a given state (with m1+m2=n1+n2)
  //
  // index = index of the state on which the operator has to be applied
  // m1 = first index for creation operator
  // m2 = second index for creation operator
  // n1 = first index for annihilation operator
  // n2 = second index for annihilation operator
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 
  virtual int AdAdAA (int index, int m1, int m2, int n1, int n2, double& coefficient);

  // apply Prod_i a^+_mi Prod_i a_ni operator to a given state (with Sum_i  mi= Sum_i ni)
  //
  // index = index of the state on which the operator has to be applied
  // m = array containg the indices of the creation operators (first index corresponding to the leftmost operator)
  // n = array containg the indices of the annihilation operators (first index corresponding to the leftmost operator)
  // nbrIndices = number of creation (or annihilation) operators
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 
  virtual int ProdAdProdA (int index, int* m, int* n, int nbrIndices, double& coefficient);

  // apply a_n1 a_n2 operator to a given state. Warning, the resulting state may not belong to the current Hilbert subspace. It will be keep in cache until next AdAd call
  //
  // index = index of the state on which the operator has to be applied
  // n1 = first index for annihilation operator
  // n2 = second index for annihilation operator
  // return value =  multiplicative factor 
  virtual double AA (int index, int n1, int n2);

  // apply Prod_i a_ni operator to a given state. Warning, the resulting state may not belong to the current Hilbert subspace. It will be keep in cache until next ProdA call
  //
  // index = index of the state on which the operator has to be applied
  // n = array containg the indices of the annihilation operators (first index corresponding to the leftmost operator)
  // nbrIndices = number of creation (or annihilation) operators
  // return value =  multiplicative factor 
  virtual double ProdA (int index, int* n, int nbrIndices);

  // apply Prod_i a_mi operator to the state produced using ProdA method (without destroying it)
  // use double when calculating normalization factors to avoid overflow
  //
  // index = index of the state on which the operator has to be applied
  // n = array containg the indices of the annihilation operators (first index corresponding to the leftmost operator)
  // nbrIndices = number of creation (or annihilation) operators
  // return value =  multiplicative factor 

  virtual double ProdAL (int index, int* n, int nbrIndices);

  // apply a^+_m1 a^+_m2 operator to the state produced using AA method (without destroying it)
  //
  // m1 = first index for creation operator
  // m2 = second index for creation operator
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 
  virtual int AdAd (int m1, int m2, double& coefficient);

  // apply Prod_i a^+_mi operator to the state produced using ProdA method (without destroying it)
  //
  // m = array containg the indices of the creation operators (first index corresponding to the leftmost operator)
  // nbrIndices = number of creation (or annihilation) operators
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 
  virtual int ProdAd (int* m, int nbrIndices, double& coefficient);

  // apply Prod_i a^+_mi operator to the state produced using ProdA method (without destroying it)
  // use double when calculating normalization factors to avoid overflow
  //
  // m = array containg the indices of the creation operators (first index corresponding to the leftmost operator)
  // nbrIndices = number of creation (or annihilation) operators
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 

  virtual int ProdAdL (int* m, int nbrIndices, double& coefficient);

  // apply a^+_m a_m operator to a given state 
  //
  // index = index of the state on which the operator has to be applied
  // m = index of the creation and annihilation operator
  // return value = coefficient obtained when applying a^+_m a_m
  virtual double AdA (int index, int m);

  // apply a^+_m a_m operator to a given state 
  //
  // index = index of the state on which the operator has to be applied
  // m = index of the creation and annihilation operator
  // return value = coefficient obtained when applying a^+_m a_m
  virtual double AdA (long index, int m);

  // apply a^+_m a_n operator to a given state 
  //
  // index = index of the state on which the operator has to be applied
  // m = index of the creation operator
  // n = index of the annihilation operator
  // coefficient = reference on the double where the multiplicative factor has to be stored
  // return value = index of the destination state 
  virtual int AdA (int index, int m, int n, double& coefficient);



};


#endif

