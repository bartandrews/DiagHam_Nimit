////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                                                            //
//                            DiagHam  version 0.01                           //
//                                                                            //
//                    Copyright (C) 2001-2011 Nicolas Regnault                //
//                                                                            //
//                                                                            //
//                   class of fermions on cubic lattice with spin             //
//                                  in momentum space                         //
//                                                                            //
//                        last modification : 23/06/2011                      //
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


#include "config.h"
#include "HilbertSpace/FermionOnCubicLatticeWithSpinMomentumSpace.h"
#include "QuantumNumber/AbstractQuantumNumber.h"
#include "QuantumNumber/SzQuantumNumber.h"
#include "Matrix/ComplexMatrix.h"
#include "Matrix/ComplexLapackDeterminant.h"
#include "Vector/RealVector.h"
#include "FunctionBasis/AbstractFunctionBasis.h"
#include "MathTools/BinomialCoefficients.h"
#include "GeneralTools/UnsignedIntegerTools.h"
#include "MathTools/FactorialCoefficient.h"
#include "GeneralTools/Endian.h"
#include "Architecture/ArchitectureOperation/FQHESphereParticleEntanglementSpectrumOperation.h"

#include <math.h>
#include <cstdlib>
#include <fstream>

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::ofstream;
using std::ifstream;
using std::ios;


// basic constructor
//
// nbrFermions = number of fermions
// nbrSiteX = number of sites in the x direction
// nbrSiteY = number of sites in the y direction
// nbrSiteZ = number of sites in the z direction
// kxMomentum = momentum along the x direction
// kyMomentum = momentum along the y direction
// kzMomentum = momentum along the z direction
// memory = amount of memory granted for precalculations

FermionOnCubicLatticeWithSpinMomentumSpace::FermionOnCubicLatticeWithSpinMomentumSpace (int nbrFermions, int nbrSiteX, int nbrSiteY, int nbrSiteZ, int kxMomentum, int kyMomentum, int kzMomentum, unsigned long memory)
{
    this->NbrFermions = nbrFermions;
    this->IncNbrFermions = this->NbrFermions + 1;
    this->SzFlag = false;
    this->TotalLz = 0;
    this->TotalSpin = 0;
    this->NbrFermionsUp = 0;
    this->NbrFermionsDown = 0;
    this->NbrSiteX = nbrSiteX;
    this->NbrSiteY = nbrSiteY;
    this->NbrSiteZ = nbrSiteZ;
    this->NbrSiteYZ = this->NbrSiteZ * this->NbrSiteY;
    this->KxMomentum = kxMomentum;
    this->KyMomentum = kyMomentum;
    this->KzMomentum = kzMomentum;
    this->LzMax = this->NbrSiteX * this->NbrSiteY * this->NbrSiteZ;
    this->NbrLzValue = this->LzMax + 1;
    this->MaximumSignLookUp = 16;
    this->LargeHilbertSpaceDimension = this->EvaluateHilbertSpaceDimension(this->NbrFermions, this->NbrSiteX - 1, this->NbrSiteY - 1, this->NbrSiteZ -1, 0, 0, 0);
    if (this->LargeHilbertSpaceDimension >= (1l << 30))
        this->HilbertSpaceDimension = 0;
    else
        this->HilbertSpaceDimension = (int) this->LargeHilbertSpaceDimension;
    if ( this->LargeHilbertSpaceDimension > 0l)
    {
        this->Flag.Initialize();
        this->TargetSpace = this;
        this->StateDescription = new unsigned long [this->HilbertSpaceDimension];
        this->StateHighestBit = new int [this->HilbertSpaceDimension];
        long TmpLargeHilbertSpaceDimension = this->GenerateStates(this->NbrFermions, this->NbrSiteX - 1, this->NbrSiteY - 1, this->NbrSiteZ - 1, 0, 0, 0, 0l);
        if (this->LargeHilbertSpaceDimension != TmpLargeHilbertSpaceDimension)
        {
            cout << "error while generating the Hilbert space " << this->LargeHilbertSpaceDimension << " " << TmpLargeHilbertSpaceDimension << endl;
        }
        this->GenerateLookUpTable(memory);

#ifdef __DEBUG__
        long UsedMemory = 0;
        UsedMemory += (long) this->HilbertSpaceDimension * (sizeof(unsigned long) + sizeof(int));
        cout << "memory requested for Hilbert space = ";
        if (UsedMemory >= 1024)
            if (UsedMemory >= 1048576)
                cout << (UsedMemory >> 20) << "Mo" << endl;
            else
                cout << (UsedMemory >> 10) << "ko" <<  endl;
        else
            cout << UsedMemory << endl;
        UsedMemory = this->NbrLzValue * sizeof(int);
        UsedMemory += this->NbrLzValue * this->LookUpTableMemorySize * sizeof(int);
        cout << "memory requested for lookup table = ";
        if (UsedMemory >= 1024)
            if (UsedMemory >= 1048576)
                cout << (UsedMemory >> 20) << "Mo" << endl;
            else
                cout << (UsedMemory >> 10) << "ko" <<  endl;
        else
            cout << UsedMemory << endl;
#endif
    }
}

// basic constructor when Sz is preserved
//
// nbrFermions = number of fermions
// nbrSpinUp = number of particles with spin up
// nbrSiteX = number of sites in the x direction
// nbrSiteY = number of sites in the y direction
// nbrSiteZ = number of sites in the z direction
// kxMomentum = momentum along the x direction
// kyMomentum = momentum along the y direction
// kzMomentum = momentum along the z direction
// nbrSiteX = number of sites in the x direction
// nbrSiteY = number of sites in the y direction
// kxMomentum = momentum along the x direction
// kyMomentum = momentum along the y direction
// memory = amount of memory granted for precalculations

FermionOnCubicLatticeWithSpinMomentumSpace::FermionOnCubicLatticeWithSpinMomentumSpace (int nbrFermions, int nbrSpinUp, int nbrSiteX, int nbrSiteY, int nbrSiteZ, int kxMomentum, int kyMomentum, int kzMomentum, unsigned long memory)
{
    this->NbrFermions = nbrFermions;
    this->IncNbrFermions = this->NbrFermions + 1;
    this->SzFlag = true;
    this->TotalLz = 0;
    this->TotalSpin = 0;
    this->NbrFermionsUp = nbrSpinUp;
    this->NbrFermionsDown = this->NbrFermions - this->NbrFermionsUp;
    this->NbrSiteX = nbrSiteX;
    this->NbrSiteY = nbrSiteY;
    this->NbrSiteZ = nbrSiteZ;
    this->NbrSiteYZ = this->NbrSiteZ * this->NbrSiteY;
    this->KxMomentum = kxMomentum;
    this->KyMomentum = kyMomentum;
    this->KzMomentum = kzMomentum;
    this->LzMax = this->NbrSiteX * this->NbrSiteY * this->NbrSiteZ;
    this->NbrLzValue = this->LzMax + 1;
    this->MaximumSignLookUp = 16;
    this->LargeHilbertSpaceDimension = this->EvaluateHilbertSpaceDimension(this->NbrFermions, this->NbrSiteX - 1, this->NbrSiteY - 1, this->NbrSiteZ - 1, 0, 0, 0, this->NbrFermionsUp);
    cout << "dim1 " << this->LargeHilbertSpaceDimension << endl;
    if (this->LargeHilbertSpaceDimension >= (1l << 30))
        this->HilbertSpaceDimension = 0;
    else
        this->HilbertSpaceDimension = (int) this->LargeHilbertSpaceDimension;
    if ( this->LargeHilbertSpaceDimension > 0l)
    {
        this->Flag.Initialize();
        this->TargetSpace = this;
        this->StateDescription = new unsigned long [this->HilbertSpaceDimension];
        this->StateHighestBit = new int [this->HilbertSpaceDimension];
        this->LargeHilbertSpaceDimension = this->GenerateStates(this->NbrFermions, this->NbrSiteX - 1, this->NbrSiteY - 1, this->NbrSiteZ - 1, 0, 0, 0, this->NbrFermionsUp, 0l);
        cout << "dim2 " << this->LargeHilbertSpaceDimension << endl;
        this->GenerateLookUpTable(memory);

#ifdef __DEBUG__
        long UsedMemory = 0;
        UsedMemory += (long) this->HilbertSpaceDimension * (sizeof(unsigned long) + sizeof(int));
        cout << "memory requested for Hilbert space = ";
        if (UsedMemory >= 1024)
            if (UsedMemory >= 1048576)
                cout << (UsedMemory >> 20) << "Mo" << endl;
            else
                cout << (UsedMemory >> 10) << "ko" <<  endl;
        else
            cout << UsedMemory << endl;
        UsedMemory = this->NbrLzValue * sizeof(int);
        UsedMemory += this->NbrLzValue * this->LookUpTableMemorySize * sizeof(int);
        cout << "memory requested for lookup table = ";
        if (UsedMemory >= 1024)
            if (UsedMemory >= 1048576)
                cout << (UsedMemory >> 20) << "Mo" << endl;
            else
                cout << (UsedMemory >> 10) << "ko" <<  endl;
        else
            cout << UsedMemory << endl;
#endif
    }
}

// copy constructor (without duplicating datas)
//
// fermions = reference on the hilbert space to copy to copy

FermionOnCubicLatticeWithSpinMomentumSpace::FermionOnCubicLatticeWithSpinMomentumSpace(const FermionOnCubicLatticeWithSpinMomentumSpace& fermions)
{
    this->HilbertSpaceDimension = fermions.HilbertSpaceDimension;
    this->LargeHilbertSpaceDimension = fermions.LargeHilbertSpaceDimension;
    this->Flag = fermions.Flag;
    this->NbrFermions = fermions.NbrFermions;
    this->IncNbrFermions = fermions.IncNbrFermions;
    this->TotalLz = fermions.TotalLz;
    this->NbrSiteX = fermions.NbrSiteX;
    this->NbrSiteY = fermions.NbrSiteY;
    this->NbrSiteZ = fermions.NbrSiteZ;
    this->NbrSiteYZ = fermions.NbrSiteYZ;
    this->KxMomentum = fermions.KxMomentum;
    this->KyMomentum = fermions.KyMomentum;
    this->KzMomentum = fermions.KzMomentum;
    this->LzMax = fermions.LzMax;
    this->NbrLzValue = fermions.NbrLzValue;
    this->TotalSpin = fermions.TotalSpin;
    this->SzFlag = fermions.SzFlag;
    this->NbrFermionsUp = fermions.NbrFermionsUp;
    this->NbrFermionsDown = fermions.NbrFermionsDown;
    this->StateDescription = fermions.StateDescription;
    this->StateHighestBit = fermions.StateHighestBit;
    this->HighestBit = fermions.HighestBit;
    this->MaximumLookUpShift = fermions.MaximumLookUpShift;
    this->LookUpTableMemorySize = fermions.LookUpTableMemorySize;
    this->LookUpTableShift = fermions.LookUpTableShift;
    this->LookUpTable = fermions.LookUpTable;
    this->SignLookUpTable = fermions.SignLookUpTable;
    this->SignLookUpTableMask = fermions.SignLookUpTableMask;
    this->MaximumSignLookUp = fermions.MaximumSignLookUp;
    if (fermions.TargetSpace != &fermions)
        this->TargetSpace = fermions.TargetSpace;
    else
        this->TargetSpace = this;
}

// destructor
//

FermionOnCubicLatticeWithSpinMomentumSpace::~FermionOnCubicLatticeWithSpinMomentumSpace ()
{
}

// assignement (without duplicating datas)
//
// fermions = reference on the hilbert space to copy to copy
// return value = reference on current hilbert space

FermionOnCubicLatticeWithSpinMomentumSpace& FermionOnCubicLatticeWithSpinMomentumSpace::operator = (const FermionOnCubicLatticeWithSpinMomentumSpace& fermions)
{
    if ((this->HilbertSpaceDimension != 0) && (this->Flag.Shared() == false) && (this->Flag.Used() == true))
    {
        delete[] this->StateDescription;
        delete[] this->StateHighestBit;
    }
    if (fermions.TargetSpace != &fermions)
        this->TargetSpace = fermions.TargetSpace;
    else
        this->TargetSpace = this;
    this->HilbertSpaceDimension = fermions.HilbertSpaceDimension;
    this->LargeHilbertSpaceDimension = fermions.LargeHilbertSpaceDimension;
    this->Flag = fermions.Flag;
    this->NbrFermions = fermions.NbrFermions;
    this->IncNbrFermions = fermions.IncNbrFermions;
    this->TotalLz = fermions.TotalLz;
    this->LzMax = fermions.LzMax;
    this->NbrSiteX = fermions.NbrSiteX;
    this->NbrSiteY = fermions.NbrSiteY;
    this->NbrSiteZ = fermions.NbrSiteZ;
    this->NbrSiteYZ = fermions.NbrSiteYZ;
    this->KxMomentum = fermions.KxMomentum;
    this->KyMomentum = fermions.KyMomentum;
    this->KzMomentum = fermions.KzMomentum;
    this->NbrLzValue = fermions.NbrLzValue;
    this->SzFlag = fermions.SzFlag;
    this->TotalSpin = fermions.TotalSpin;
    this->NbrFermionsUp = fermions.NbrFermionsUp;
    this->NbrFermionsDown = fermions.NbrFermionsDown;
    this->StateDescription = fermions.StateDescription;
    this->StateHighestBit = fermions.StateHighestBit;
    this->MaximumLookUpShift = fermions.MaximumLookUpShift;
    this->LookUpTableMemorySize = fermions.LookUpTableMemorySize;
    this->LookUpTableShift = fermions.LookUpTableShift;
    this->LookUpTable = fermions.LookUpTable;
    return *this;
}

// clone Hilbert space (without duplicating datas)
//
// return value = pointer to cloned Hilbert space

AbstractHilbertSpace* FermionOnCubicLatticeWithSpinMomentumSpace::Clone()
{
    return new FermionOnCubicLatticeWithSpinMomentumSpace(*this);
}

// print a given State
//
// Str = reference on current output stream
// state = ID of the state to print
// return value = reference on current output stream

ostream& FermionOnCubicLatticeWithSpinMomentumSpace::PrintState (ostream& Str, int state)
{
    unsigned long TmpState = this->StateDescription[state];
    unsigned long Tmp;
    Str << "[";
    for (int i = 0; i < this->NbrLzValue; ++i)
    {
        Tmp = (TmpState >> (i << 1));
        int TmpKx = i / this->NbrSiteYZ;
        int TmpKy = i % this->NbrSiteYZ;
        int TmpKz = TmpKy % this->NbrSiteZ;
        TmpKy /= this->NbrSiteZ;
        if ((Tmp & 0x1l) != 0ul)
            Str << "(" << TmpKx << "," << TmpKy << "," << TmpKz << ",-)";
        if ((Tmp & 0x2l) != 0ul)
            Str << "(" << TmpKx << "," << TmpKy << "," << TmpKz << ",+)";
    }
    Str << "]";
//   Str << " " << TmpState;
//   Str << " " << hex << TmpState << dec;
//   Str << " " << state;
//   int TmpLzMax = (this->LzMax << 1) + 1;
//   while (((TmpState >> TmpLzMax) & 0x1ul) == 0x0ul)
//     --TmpLzMax;
//   Str << " " << this->FindStateIndex(TmpState, TmpLzMax);
    return Str;
}

// generate all states corresponding to the constraints
//
// nbrFermions = number of fermions
// currentKx = current momentum along x for a single particle
// currentKy = current momentum along y for a single particle
// currentTotalKx = current total momentum along x
// currentTotalKy = current total momentum along y
// pos = position in StateDescription array where to store states
// return value = position from which new states have to be stored

long FermionOnCubicLatticeWithSpinMomentumSpace::GenerateStates(int nbrFermions, int currentKx, int currentKy, int currentKz, int currentTotalKx, int currentTotalKy, int currentTotalKz, long pos)
{
    if (currentKz < 0)
    {
        currentKz = this->NbrSiteZ - 1;
        currentKy--;
        if (currentKy < 0)
        {
            currentKy = this->NbrSiteY - 1;
            currentKx--;
        }
    }
    if (nbrFermions == 0)
    {
        if (((currentTotalKx % this->NbrSiteX) == this->KxMomentum) && ((currentTotalKy % this->NbrSiteY) == this->KyMomentum)
                && ((currentTotalKz % this->NbrSiteZ) == this->KzMomentum))
        {
            this->StateDescription[pos] = 0x0ul;
            return (pos + 1l);
        }
        else
            return pos;
    }
    if (currentKx < 0)
        return pos;
    if (nbrFermions == 1)
    {
        for (int k = currentKz; k >= 0; --k)
        {
            if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((currentKy + currentTotalKy) % this->NbrSiteY) == this->KyMomentum) &&
                    (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
            {
                this->StateDescription[pos] = 0x2ul << ((((currentKx * this->NbrSiteY) + currentKy) * this->NbrSiteZ + k) << 1);
                ++pos;
                this->StateDescription[pos] = 0x1ul << ((((currentKx * this->NbrSiteY) + currentKy) * this->NbrSiteZ + k) << 1);
                ++pos;
            }
        }
        for (int j = currentKy - 1; j >= 0; --j)
        {
            for (int k = this->NbrSiteZ - 1; k >= 0; --k)
            {
                if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum)
                        && (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
                {
                    this->StateDescription[pos] = 0x2ul << ((((currentKx * this->NbrSiteY) + j) * this->NbrSiteZ + k) << 1);
                    ++pos;
                    this->StateDescription[pos] = 0x1ul << ((((currentKx * this->NbrSiteY) + j) * this->NbrSiteZ + k) << 1);
                    ++pos;
                }
            }
        }
        for (int i = currentKx - 1; i >= 0; --i)
        {
            for (int j = this->NbrSiteY - 1; j >= 0; --j)
            {
                for (int k = this->NbrSiteZ - 1; k >= 0; --k)
                {
                    if ((((i + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum)
                            && (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
                    {
                        this->StateDescription[pos] = 0x2ul << ((((i * this->NbrSiteY) + j) * this->NbrSiteZ + k) << 1);
                        ++pos;
                        this->StateDescription[pos] = 0x1ul << ((((i * this->NbrSiteY) + j) * this->NbrSiteZ + k) << 1);
                        ++pos;
                    }
                }
            }
        }
        return pos;
    }
    long TmpPos = this->GenerateStates(nbrFermions - 2, currentKx, currentKy, currentKz - 1, currentTotalKx + (2 * currentKx), currentTotalKy + (2 * currentKy), currentTotalKz + (2 * currentKz), pos);
    unsigned long Mask = 0x3ul << ((((currentKx * this->NbrSiteY) + currentKy) * this->NbrSiteZ + currentKz) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    TmpPos = this->GenerateStates(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, pos);
    Mask = 0x2ul << ((((currentKx * this->NbrSiteY) + currentKy) * this->NbrSiteZ + currentKz) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    TmpPos = this->GenerateStates(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, pos);
    Mask = 0x1ul << ((((currentKx * this->NbrSiteY) + currentKy) * this->NbrSiteZ + currentKz) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    return this->GenerateStates(nbrFermions, currentKx, currentKy, currentKz - 1, currentTotalKx, currentTotalKy, currentTotalKz, pos);
};

// generate all states corresponding to the constraints
//
// nbrFermions = number of fermions
// currentKx = current momentum along x for a single particle
// currentKy = current momentum along y for a single particle
// currentTotalKx = current total momentum along x
// currentTotalKy = current total momentum along y
// nbrSpinUp = number of fermions with spin up
// pos = position in StateDescription array where to store states
// return value = position from which new states have to be stored

long FermionOnCubicLatticeWithSpinMomentumSpace::GenerateStates(int nbrFermions, int currentKx, int currentKy, int currentKz, int currentTotalKx, int currentTotalKy, int currentTotalKz, int nbrSpinUp, long pos)
{
    if (currentKy < 0)
    {
        currentKy = this->NbrSiteY - 1;
        currentKx--;
    }

    if ((nbrSpinUp < 0) || (nbrSpinUp > nbrFermions))
        return 0l;

    if (nbrFermions == 0)
    {
        if (((currentTotalKx % this->NbrSiteX) == this->KxMomentum) && ((currentTotalKy % this->NbrSiteY) == this->KyMomentum))
        {
            this->StateDescription[pos] = 0x0ul;
            return (pos + 1l);
        }
        else
            return pos;
    }
    if (currentKx < 0)
        return pos;
    if (nbrFermions == 1)
    {
        if (nbrSpinUp == 1)
        {
            for (int j = currentKy; j >= 0; --j)
            {
                if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                {
                    this->StateDescription[pos] = 0x2ul << (((currentKx * this->NbrSiteY) + j) << 1);
                    ++pos;
                }
                for (int i = currentKx - 1; i >= 0; --i)
                {
                    for (int j = this->NbrSiteY - 1; j >= 0; --j)
                    {
                        if ((((i + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                        {
                            this->StateDescription[pos] = 0x2ul << (((i * this->NbrSiteY) + j) << 1);
                            ++pos;
                        }
                    }
                }
            }
        }
        else
        {
            for (int j = currentKy; j >= 0; --j)
            {
                if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                {
                    this->StateDescription[pos] = 0x1ul << (((currentKx * this->NbrSiteY) + j) << 1);
                    ++pos;
                }
                for (int i = currentKx - 1; i >= 0; --i)
                {
                    for (int j = this->NbrSiteY - 1; j >= 0; --j)
                    {
                        if ((((i + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                        {
                            this->StateDescription[pos] = 0x1ul << (((i * this->NbrSiteY) + j) << 1);
                            ++pos;
                        }
                    }
                }
            }
        }
        return pos;
    }
    long TmpPos = this->GenerateStates(nbrFermions - 2, currentKx, currentKy, currentKz - 1, currentTotalKx + (2 * currentKx), currentTotalKy + (2 * currentKy), currentTotalKz + (2 * currentKz), nbrSpinUp - 1, pos);
    unsigned long Mask = 0x3ul << (((currentKx * this->NbrSiteY) + currentKy) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    TmpPos = this->GenerateStates(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, nbrSpinUp - 1, pos);
    Mask = 0x2ul << (((currentKx * this->NbrSiteY) + currentKy) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    TmpPos = this->GenerateStates(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, nbrSpinUp, pos);
    Mask = 0x1ul << (((currentKx * this->NbrSiteY) + currentKy) << 1);
    for (; pos < TmpPos; ++pos)
        this->StateDescription[pos] |= Mask;
    return this->GenerateStates(nbrFermions, currentKx, currentKy, currentKz - 1, currentTotalKx, currentTotalKy, currentTotalKz, nbrSpinUp, pos);
}

// evaluate Hilbert space dimension
//
// nbrFermions = number of fermions
// currentKx = current momentum along x for a single particle
// currentKy = current momentum along y for a single particle
// currentKz = current momentum along z for a single particle
// currentTotalKx = current total momentum along x
// currentTotalKy = current total momentum along y
// currentTotalKz = current total momentum along z
// return value = Hilbert space dimension

long FermionOnCubicLatticeWithSpinMomentumSpace::EvaluateHilbertSpaceDimension(int nbrFermions, int currentKx, int currentKy, int currentKz, int currentTotalKx, int currentTotalKy, int currentTotalKz)
{
    if (currentKz < 0)
    {
        currentKz = this->NbrSiteZ - 1;
        currentKy--;
        if (currentKy < 0)
        {
            currentKy = this->NbrSiteY - 1;
            currentKx--;
        }
    }
    if (nbrFermions == 0)
    {
        if (((currentTotalKx % this->NbrSiteX) == this->KxMomentum) && ((currentTotalKy % this->NbrSiteY) == this->KyMomentum)
                && ((currentTotalKz % this->NbrSiteZ) == this->KzMomentum))
            return 1l;
        else
            return 0l;
    }
    if (currentKx < 0)
        return 0l;
    long Count = 0;
    if (nbrFermions == 1)
    {
        for (int k = currentKz; k >= 0; --k)
        {
            if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((currentKy + currentTotalKy) % this->NbrSiteY) == this->KyMomentum) &&
                    (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
                Count += 2l;
        }
        for (int j = currentKy - 1; j >= 0; --j)
        {
            for (int k = this->NbrSiteZ - 1; k >= 0; --k)
            {
                if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum)
                        && (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
                    Count += 2l;
            }
        }
        for (int i = currentKx - 1; i >= 0; --i)
        {
            for (int j = this->NbrSiteY - 1; j >= 0; --j)
            {
                for (int k = this->NbrSiteZ - 1; k >= 0; --k)
                {
                    if ((((i + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum)
                            && (((k + currentTotalKz) % this->NbrSiteZ) == this->KzMomentum))
                        Count += 2l;
                }
            }
        }
        return Count;
    }
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions - 2, currentKx, currentKy, currentKz - 1, currentTotalKx + (2 * currentKx), currentTotalKy + (2 * currentKy), currentTotalKz + (2 * currentKz));
    Count += (2 * this->EvaluateHilbertSpaceDimension(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz));
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions, currentKx, currentKy, currentKz - 1, currentTotalKx, currentTotalKy, currentTotalKz);
    return Count;
}


// evaluate Hilbert space dimension with a fixed number of fermions with spin up
//
// nbrFermions = number of fermions
// currentKx = current momentum along x for a single particle
// currentKy = current momentum along y for a single particle
// currentKz = current momentum along z for a single particle
// currentTotalKx = current total momentum along x
// currentTotalKy = current total momentum along y
// currentTotalKz = current total momentum along z
// nbrSpinUp = number of fermions with spin up
// return value = Hilbert space dimension

long FermionOnCubicLatticeWithSpinMomentumSpace::EvaluateHilbertSpaceDimension(int nbrFermions, int currentKx, int currentKy, int currentKz, int currentTotalKx, int currentTotalKy, int currentTotalKz, int nbrSpinUp)
{
    if (currentKy < 0)
    {
        currentKy = this->NbrSiteY - 1;
        currentKx--;
    }
    if ((nbrSpinUp < 0) || (nbrSpinUp > nbrFermions))
        return 0l;

    if (nbrFermions == 0)
    {
        if (((currentTotalKx % this->NbrSiteX) == this->KxMomentum) && ((currentTotalKy % this->NbrSiteY) == this->KyMomentum))
            return 1l;
        else
            return 0l;
    }
    if (currentKx < 0)
        return 0l;
    long Count = 0;
    if (nbrFermions == 1)
    {
        for (int j = currentKy; j >= 0; --j)
        {
            if ((((currentKx + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                Count++;
        }
        for (int i = currentKx - 1; i >= 0; --i)
        {
            for (int j = this->NbrSiteY - 1; j >= 0; --j)
            {
                if ((((i + currentTotalKx) % this->NbrSiteX) == this->KxMomentum) && (((j + currentTotalKy) % this->NbrSiteY) == this->KyMomentum))
                    Count++;
            }
        }
        return Count;
    }
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions - 2, currentKx, currentKy, currentKz - 1, currentTotalKx + (2 * currentKx), currentTotalKy + (2 * currentKy), currentTotalKz + (2 * currentKz), nbrSpinUp - 1);
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, nbrSpinUp);
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions - 1, currentKx, currentKy, currentKz - 1, currentTotalKx + currentKx, currentTotalKy + currentKy, currentTotalKz + currentKz, nbrSpinUp - 1);
    Count += this->EvaluateHilbertSpaceDimension(nbrFermions, currentKx, currentKy, currentKz - 1, currentTotalKx, currentTotalKy, currentTotalKz, nbrSpinUp);
    return Count;
}

// evaluate a density matrix of a subsystem of the whole system described by a given ground state, using particle partition. The density matrix is only evaluated in a given momentum sector.
//
// nbrParticleSector = number of particles that belong to the subsytem
// kxSector = kx sector in which the density matrix has to be evaluated
// kySector = kx sector in which the density matrix has to be evaluated
// kzSector = kx sector in which the density matrix has to be evaluated
// groundState = reference on the total system ground state
// architecture = pointer to the architecture to use parallelized algorithm
// return value = density matrix of the subsytem (return a wero dimension matrix if the density matrix is equal to zero)

HermitianMatrix FermionOnCubicLatticeWithSpinMomentumSpace::EvaluatePartialDensityMatrixParticlePartition (int nbrParticleSector, int kxSector, int kySector, int kzSector, ComplexVector& groundState, AbstractArchitecture* architecture)
{
    if (nbrParticleSector == 0)
    {
        if ((kxSector == 0) && (kySector == 0) && (kzSector == 0))
        {
            HermitianMatrix TmpDensityMatrix(1, true);
            TmpDensityMatrix(0, 0) = 1.0;
            return TmpDensityMatrix;
        }
        else
        {
            HermitianMatrix TmpDensityMatrix;
            return TmpDensityMatrix;
        }
    }
    if (nbrParticleSector == this->NbrFermions)
    {
        if ((kxSector == this->KxMomentum) && (kySector == this->KyMomentum) && (kzSector == this->KzMomentum))
        {
            HermitianMatrix TmpDensityMatrix(1, true);
            TmpDensityMatrix(0, 0) = 1.0;
            return TmpDensityMatrix;
        }
        else
        {
            HermitianMatrix TmpDensityMatrix;
            return TmpDensityMatrix;
        }
    }
    int ComplementaryNbrParticles = this->NbrFermions - nbrParticleSector;
    int ComplementaryKxMomentum = (this->KxMomentum - kxSector) % this->NbrSiteX;
    int ComplementaryKyMomentum = (this->KyMomentum - kySector) % this->NbrSiteY;
    int ComplementaryKzMomentum = (this->KzMomentum - kzSector) % this->NbrSiteZ;
    if (ComplementaryKxMomentum < 0)
        ComplementaryKxMomentum += this->NbrSiteX;
    if (ComplementaryKyMomentum < 0)
        ComplementaryKyMomentum += this->NbrSiteY;
    if (ComplementaryKzMomentum < 0)
        ComplementaryKzMomentum += this->NbrSiteZ;
    cout << "kx = " << this->KxMomentum << " " << kxSector << " " << ComplementaryKxMomentum << endl;
    cout << "ky = " << this->KyMomentum << " " << kySector << " " << ComplementaryKyMomentum << endl;
    cout << "kz = " << this->KzMomentum << " " << kzSector << " " << ComplementaryKzMomentum << endl;
    FermionOnCubicLatticeWithSpinMomentumSpace SubsytemSpace (nbrParticleSector, this->NbrSiteX, this->NbrSiteY, this->NbrSiteZ, kxSector, kySector, kzSector);
    HermitianMatrix TmpDensityMatrix (SubsytemSpace.GetHilbertSpaceDimension(), true);
    FermionOnCubicLatticeWithSpinMomentumSpace ComplementarySpace (ComplementaryNbrParticles, this->NbrSiteX, this->NbrSiteY, this->NbrSiteZ, ComplementaryKxMomentum, ComplementaryKyMomentum, ComplementaryKzMomentum);
    cout << "subsystem Hilbert space dimension = " << SubsytemSpace.HilbertSpaceDimension << endl;


    FQHESphereParticleEntanglementSpectrumOperation Operation(this, &SubsytemSpace, &ComplementarySpace, groundState, TmpDensityMatrix);
    Operation.ApplyOperation(architecture);
    if (Operation.GetNbrNonZeroMatrixElements() > 0)
        return TmpDensityMatrix;
    else
    {
        HermitianMatrix TmpDensityMatrixZero;
        return TmpDensityMatrixZero;
    }
}

// evaluate a density matrix of a subsystem of the whole system described by a given sum of projectors, using particle partition. The density matrix is only evaluated in a given momentum sector.
//
// nbrParticleSector = number of particles that belong to the subsytem
// kxSector = kx sector in which the density matrix has to be evaluated
// kySector = kx sector in which the density matrix has to be evaluated
// kzSector = kx sector in which the density matrix has to be evaluated
// nbrGroundStates = number of projectors
// groundStates = array of degenerate groundstates associated to each projector
// weights = array of weights in front of each projector
// architecture = pointer to the architecture to use parallelized algorithm
// return value = density matrix of the subsytem (return a wero dimension matrix if the density matrix is equal to zero)

HermitianMatrix FermionOnCubicLatticeWithSpinMomentumSpace::EvaluatePartialDensityMatrixParticlePartition (int nbrParticleSector, int kxSector, int kySector, int kzSector,
        int nbrGroundStates, ComplexVector* groundStates, double* weights, AbstractArchitecture* architecture)
{
    if (nbrParticleSector == 0)
    {
        if ((kxSector == 0) && (kySector == 0) && (kzSector == 0))
        {
            HermitianMatrix TmpDensityMatrix(1, true);
            TmpDensityMatrix(0, 0) = 0.0;
            for (int i = 0; i < nbrGroundStates; ++i)
                TmpDensityMatrix(0, 0) += weights[i];
            return TmpDensityMatrix;
        }
        else
        {
            HermitianMatrix TmpDensityMatrix;
            return TmpDensityMatrix;
        }
    }
    if (nbrParticleSector == this->NbrFermions)
    {
        if ((kxSector == this->KxMomentum) && (kySector == this->KyMomentum) && (kzSector == this->KzMomentum))
        {
            HermitianMatrix TmpDensityMatrix(1, true);
            TmpDensityMatrix(0, 0) = 0.0;
            for (int i = 0; i < nbrGroundStates; ++i)
                TmpDensityMatrix(0, 0) += weights[i];
            return TmpDensityMatrix;
        }
        else
        {
            HermitianMatrix TmpDensityMatrix;
            return TmpDensityMatrix;
        }
    }
    int ComplementaryNbrParticles = this->NbrFermions - nbrParticleSector;
    int ComplementaryKxMomentum = (this->KxMomentum - kxSector) % this->NbrSiteX;
    int ComplementaryKyMomentum = (this->KyMomentum - kySector) % this->NbrSiteY;
    int ComplementaryKzMomentum = (this->KzMomentum - kzSector) % this->NbrSiteZ;
    if (ComplementaryKxMomentum < 0)
        ComplementaryKxMomentum += this->NbrSiteX;
    if (ComplementaryKyMomentum < 0)
        ComplementaryKyMomentum += this->NbrSiteY;
    if (ComplementaryKzMomentum < 0)
        ComplementaryKzMomentum += this->NbrSiteZ;
    cout << "kx = " << this->KxMomentum << " " << kxSector << " " << ComplementaryKxMomentum << endl;
    cout << "ky = " << this->KyMomentum << " " << kySector << " " << ComplementaryKyMomentum << endl;
    cout << "kz = " << this->KzMomentum << " " << kzSector << " " << ComplementaryKzMomentum << endl;
    FermionOnCubicLatticeWithSpinMomentumSpace SubsytemSpace (nbrParticleSector, this->NbrSiteX, this->NbrSiteY, this->NbrSiteZ,
            kxSector, kySector, kzSector);
    HermitianMatrix TmpDensityMatrix (SubsytemSpace.GetHilbertSpaceDimension(), true);
    FermionOnCubicLatticeWithSpinMomentumSpace ComplementarySpace (ComplementaryNbrParticles, this->NbrSiteX, this->NbrSiteY, this->NbrSiteZ,
            ComplementaryKxMomentum, ComplementaryKyMomentum, ComplementaryKzMomentum);
    cout << "subsystem Hilbert space dimension = " << SubsytemSpace.HilbertSpaceDimension << endl;


    FQHESphereParticleEntanglementSpectrumOperation Operation(this, &SubsytemSpace, &ComplementarySpace, nbrGroundStates, groundStates, weights, TmpDensityMatrix);
    Operation.ApplyOperation(architecture);
    if (Operation.GetNbrNonZeroMatrixElements() > 0)
        return TmpDensityMatrix;
    else
    {
        HermitianMatrix TmpDensityMatrixZero;
        return TmpDensityMatrixZero;
    }
}

// core part of the evaluation density matrix particle partition calculation
//
// minIndex = first index to consider in complementary Hilbert space
// nbrIndex = number of indices to consider in complementary Hilbert space
// complementaryHilbertSpace = pointer to the complementary Hilbert space (i.e part B)
// destinationHilbertSpace = pointer to the destination Hilbert space (i.e. part A)
// groundState = reference on the total system ground state
// densityMatrix = reference on the density matrix where result has to stored
// return value = number of components that have been added to the density matrix

long FermionOnCubicLatticeWithSpinMomentumSpace::EvaluatePartialDensityMatrixParticlePartitionCore (int minIndex, int nbrIndex, ParticleOnSphere* complementaryHilbertSpace,  ParticleOnSphere* destinationHilbertSpace,
        ComplexVector& groundState,  HermitianMatrix* densityMatrix)
{
    FermionOnCubicLatticeWithSpinMomentumSpace* TmpHilbertSpace =  (FermionOnCubicLatticeWithSpinMomentumSpace*) complementaryHilbertSpace;
    FermionOnCubicLatticeWithSpinMomentumSpace* TmpDestinationHilbertSpace =  (FermionOnCubicLatticeWithSpinMomentumSpace*) destinationHilbertSpace;
    int* TmpStatePosition = new int [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    int* TmpStatePosition2 = new int [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    Complex* TmpStateCoefficient = new Complex [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    int MaxIndex = minIndex + nbrIndex;
    long TmpNbrNonZeroElements = 0l;
    BinomialCoefficients TmpBinomial (this->NbrFermions);
    double TmpInvBinomial = 1.0 / sqrt(TmpBinomial(this->NbrFermions, TmpDestinationHilbertSpace->NbrFermions));

    for (; minIndex < MaxIndex; ++minIndex)
    {
        int Pos = 0;
        unsigned long TmpState = TmpHilbertSpace->StateDescription[minIndex];
        for (int j = 0; j < TmpDestinationHilbertSpace->HilbertSpaceDimension; ++j)
        {
            unsigned long TmpState2 = TmpDestinationHilbertSpace->StateDescription[j];
            if ((TmpState & TmpState2) == 0x0ul)
            {
                int TmpLzMax = (this->LzMax << 1) + 1;
                unsigned long TmpState3 = TmpState | TmpState2;
                while ((TmpState3 >> TmpLzMax) == 0x0ul)
                    --TmpLzMax;
                int TmpPos = this->FindStateIndex(TmpState3, TmpLzMax);
                if (TmpPos != this->HilbertSpaceDimension)
                {
                    double Coefficient = TmpInvBinomial;
                    unsigned long Sign = 0x0ul;
                    int Pos2 = TmpDestinationHilbertSpace->LzMax;
                    while ((Pos2 > 0) && (TmpState2 != 0x0ul))
                    {
                        while (((TmpState2 >> Pos2) & 0x1ul) == 0x0ul)
                            --Pos2;
                        TmpState3 = TmpState & ((0x1ul << (Pos2 + 1)) - 1ul);
#ifdef  __64_BITS__
                        TmpState3 ^= TmpState3 >> 32;
#endif
                        TmpState3 ^= TmpState3 >> 16;
                        TmpState3 ^= TmpState3 >> 8;
                        TmpState3 ^= TmpState3 >> 4;
                        TmpState3 ^= TmpState3 >> 2;
                        TmpState3 ^= TmpState3 >> 1;
                        Sign ^= TmpState3;
                        TmpState2 &= ~(0x1ul << Pos2);
                        --Pos2;
                    }
                    if ((Sign & 0x1ul) == 0x0ul)
                        Coefficient *= 1.0;
                    else
                        Coefficient *= -1.0;
                    TmpStatePosition[Pos] = TmpPos;
                    TmpStatePosition2[Pos] = j;
                    TmpStateCoefficient[Pos] = Coefficient;
                    ++Pos;
                }
            }
        }
        if (Pos != 0)
        {
            ++TmpNbrNonZeroElements;
            for (int j = 0; j < Pos; ++j)
            {
                int Pos2 = TmpStatePosition2[j];
                Complex TmpValue = Conj(groundState[TmpStatePosition[j]]) * TmpStateCoefficient[j];
                for (int k = 0; k < Pos; ++k)
                    if (TmpStatePosition2[k] >= Pos2)
                    {
                        densityMatrix->AddToMatrixElement(Pos2, TmpStatePosition2[k], TmpValue * groundState[TmpStatePosition[k]] * TmpStateCoefficient[k]);
                    }
            }
        }
    }
    delete[] TmpStatePosition;
    delete[] TmpStatePosition2;
    delete[] TmpStateCoefficient;
    return TmpNbrNonZeroElements;
}

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

long FermionOnCubicLatticeWithSpinMomentumSpace::EvaluatePartialDensityMatrixParticlePartitionCore (int minIndex, int nbrIndex, ParticleOnSphere* complementaryHilbertSpace,  ParticleOnSphere* destinationHilbertSpace,
        int nbrGroundStates, ComplexVector* groundStates, double* weights, HermitianMatrix* densityMatrix)
{
    FermionOnCubicLatticeWithSpinMomentumSpace* TmpHilbertSpace =  (FermionOnCubicLatticeWithSpinMomentumSpace*) complementaryHilbertSpace;
    FermionOnCubicLatticeWithSpinMomentumSpace* TmpDestinationHilbertSpace =  (FermionOnCubicLatticeWithSpinMomentumSpace*) destinationHilbertSpace;
    int* TmpStatePosition = new int [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    int* TmpStatePosition2 = new int [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    Complex* TmpStateCoefficient = new Complex [TmpDestinationHilbertSpace->HilbertSpaceDimension];
    int MaxIndex = minIndex + nbrIndex;
    long TmpNbrNonZeroElements = 0l;
    BinomialCoefficients TmpBinomial (this->NbrFermions);
    double TmpInvBinomial = 1.0 / sqrt(TmpBinomial(this->NbrFermions, TmpDestinationHilbertSpace->NbrFermions));
    Complex* TmpValues = new Complex[nbrGroundStates];

    for (; minIndex < MaxIndex; ++minIndex)
    {
        int Pos = 0;
        unsigned long TmpState = TmpHilbertSpace->StateDescription[minIndex];
        for (int j = 0; j < TmpDestinationHilbertSpace->HilbertSpaceDimension; ++j)
        {
            unsigned long TmpState2 = TmpDestinationHilbertSpace->StateDescription[j];
            if ((TmpState & TmpState2) == 0x0ul)
            {
                int TmpLzMax = (this->LzMax << 1) + 1;
                unsigned long TmpState3 = TmpState | TmpState2;
                while ((TmpState3 >> TmpLzMax) == 0x0ul)
                    --TmpLzMax;
                int TmpPos = this->FindStateIndex(TmpState3, TmpLzMax);
                if (TmpPos != this->HilbertSpaceDimension)
                {
                    double Coefficient = TmpInvBinomial;
                    unsigned long Sign = 0x0ul;
                    int Pos2 = TmpDestinationHilbertSpace->LzMax;
                    while ((Pos2 > 0) && (TmpState2 != 0x0ul))
                    {
                        while (((TmpState2 >> Pos2) & 0x1ul) == 0x0ul)
                            --Pos2;
                        TmpState3 = TmpState & ((0x1ul << (Pos2 + 1)) - 1ul);
#ifdef  __64_BITS__
                        TmpState3 ^= TmpState3 >> 32;
#endif
                        TmpState3 ^= TmpState3 >> 16;
                        TmpState3 ^= TmpState3 >> 8;
                        TmpState3 ^= TmpState3 >> 4;
                        TmpState3 ^= TmpState3 >> 2;
                        TmpState3 ^= TmpState3 >> 1;
                        Sign ^= TmpState3;
                        TmpState2 &= ~(0x1ul << Pos2);
                        --Pos2;
                    }
                    if ((Sign & 0x1ul) == 0x0ul)
                        Coefficient *= 1.0;
                    else
                        Coefficient *= -1.0;
                    TmpStatePosition[Pos] = TmpPos;
                    TmpStatePosition2[Pos] = j;
                    TmpStateCoefficient[Pos] = Coefficient;
                    ++Pos;
                }
            }
        }
        if (Pos != 0)
        {
            ++TmpNbrNonZeroElements;
            for (int j = 0; j < Pos; ++j)
            {
                int Pos2 = TmpStatePosition2[j];
                for (int l = 0; l < nbrGroundStates; ++l)
                    TmpValues[l] = weights[l] * Conj(groundStates[l][TmpStatePosition[j]]) * TmpStateCoefficient[j];
                for (int k = 0; k < Pos; ++k)
                    if (TmpStatePosition2[k] >= Pos2)
                    {
                        for (int l = 0; l < nbrGroundStates; ++l)
                            densityMatrix->AddToMatrixElement(Pos2, TmpStatePosition2[k],
                                                              TmpValues[l] * groundStates[l][TmpStatePosition[k]] * TmpStateCoefficient[k]);
                    }
            }
        }
    }
    delete[] TmpValues;
    delete[] TmpStatePosition;
    delete[] TmpStatePosition2;
    delete[] TmpStateCoefficient;
    return TmpNbrNonZeroElements;
}

// apply the inversion symmetry i.e (k_x,k_y,k_z)->(-k_x,-k_y,-k_z) to a state
//
// inputstate = reference on the input state
// inputSpace = pointer to the Hilbert space associated to the input state
// return value = resulting state

ComplexVector FermionOnCubicLatticeWithSpinMomentumSpace::InversionSymmetry(ComplexVector& state, FermionOnCubicLatticeWithSpinMomentumSpace* inputSpace)
{
    ComplexVector OutputState (this->HilbertSpaceDimension, true);
    return OutputState;
}
