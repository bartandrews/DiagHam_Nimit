////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                                                            //
//                            DiagHam  version 0.01                           //
//                                                                            //
//                  Copyright (C) 2001-2002 Nicolas Regnault                  //
//                                                                            //
//                                                                            //
//                           class of hermitian matrix                        //
//                                                                            //
//                        last modification : 23/01/2001                      //
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


#include "Matrix/HermitianMatrix.h"
#include "Matrix/RealMatrix.h"
#include "Matrix/ComplexMatrix.h"
#include "Vector/ComplexVector.h"
#include "Matrix/RealSymmetricMatrix.h"
#include "Matrix/RealDiagonalMatrix.h"

#include <stdlib.h>


#ifdef __LAPACK__

extern "C" void FORTRAN_NAME(zhpev)(const char* jobz, const char* uplo, const int* dimension, const doublecomplex* matrixAP, const double *eigenvalues, const doublecomplex *eigenvectors, const int* leadingDimension, const doublecomplex *work, const doublereal *rwork, const int* information );
extern "C" void FORTRAN_NAME(zhpevx)(const char* jobz, const char* range, const char* uplo, const int* dimensionN, const doublecomplex* matrixAP, const double *lowerBoundVL, const double *upperBoundVU, const int* lowerIndexIL, const int* upperIndexIU, const double *errorABSTOL, const int* nbrFoundM, const double *eigenvaluesW, const doublecomplex *eigenvectorsZ, const int* leadingDimensionLDZ, const doublecomplex *work, const doublereal *rwork, const int* iwork, const int* ifail, const int* information);
#endif

/* zhpevx workspace requirements:
   WORK    (workspace) COMPLEX*16 array, dimension (2*N)

   RWORK   (workspace) DOUBLE PRECISION array, dimension (7*N)

   IWORK   (workspace) INTEGER array, dimension (5*N)
*/

using std::cout;
using std::endl;


// default constructor
//

HermitianMatrix::HermitianMatrix()
{
    this->DiagonalElements = 0;
    this->RealOffDiagonalElements = 0;
    this->ImaginaryOffDiagonalElements = 0;
    this->NbrRow = 0;
    this->NbrColumn = 0;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = 0;
    this->MatrixType = Matrix::ComplexElements | Matrix::Hermitian;
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// constructor for an empty matrix
//
// dimension = matrix dimension
// zero = true if matrix has to be filled with zeros

HermitianMatrix::HermitianMatrix(int dimension, bool zero)
{
    this->NbrRow = dimension;
    this->NbrColumn = dimension;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->Flag.Initialize();
    this->MatrixType = Matrix::ComplexElements | Matrix::Hermitian;
    this->DiagonalElements = new double [this->NbrRow];
    this->RealOffDiagonalElements = new double [(((long) this->NbrRow) * (((long) this->NbrRow) - 1)) / 2l];
    this->ImaginaryOffDiagonalElements = new double [(((long) this->NbrRow) * (((long) this->NbrRow) - 1)) / 2l];
    if (zero == true)
    {
        int pos = 0;
        for (int i = 0; i < this->NbrRow; i++)
        {
            this->DiagonalElements[i] = 0.0;
            for (int j = i + 1; j < this->NbrRow; j++)
            {
                this->RealOffDiagonalElements[pos] = 0.0;
                this->ImaginaryOffDiagonalElements[pos] = 0.0;
                pos++;
            }
        }
    }
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// constructor from matrix elements (without duplicating datas)
//
// diagonal = pointer to diagonal element array
// realOffDiagonal = pointer to real part of off-diagonal elements
// imaginaryOffDiagonal = pointer to imaginary part of off-diagonal elements
// dimension = matrix dimension

HermitianMatrix::HermitianMatrix(double* diagonal, double* realOffDiagonal, double* imaginaryOffDiagonal, int dimension)
{
    this->DiagonalElements = diagonal;
    this->RealOffDiagonalElements = realOffDiagonal;
    this->ImaginaryOffDiagonalElements = imaginaryOffDiagonal;
    this->Flag.Initialize();
    this->NbrRow = dimension;
    this->NbrColumn = dimension;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->MatrixType = Matrix::ComplexElements | Matrix::Hermitian;
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// copy constructor (without duplicating datas)
//
// M = matrix to copy

HermitianMatrix::HermitianMatrix(const HermitianMatrix& M)
{
    this->DiagonalElements = M.DiagonalElements;
    this->Flag = M.Flag;
    this->RealOffDiagonalElements = M.RealOffDiagonalElements;
    this->ImaginaryOffDiagonalElements = M.ImaginaryOffDiagonalElements;
    this->NbrRow = M.NbrRow;
    this->NbrColumn = M.NbrColumn;
    this->TrueNbrRow = M.TrueNbrRow;
    this->TrueNbrColumn = M.TrueNbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->MatrixType = Matrix::ComplexElements | Matrix::Hermitian;
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// copy constructor from a real tridiagonal symmetric matrix (without duplicating diagonal elements)
//
// M = matrix to copy

HermitianMatrix::HermitianMatrix(const RealTriDiagonalSymmetricMatrix& M)
{
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// copy constructor from a complex matrix, keeping only the upper triangular part  (duplicating all data)
//
// M = matrix to copy

HermitianMatrix::HermitianMatrix(const ComplexMatrix& M)
{
    if (M.NbrRow >= M.NbrColumn)
    {
        this->NbrRow = M.NbrColumn;
    }
    else
    {
        this->NbrRow = M.NbrRow;
    }
    this->NbrColumn = this->NbrRow;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->Flag.Initialize();
    this->MatrixType = Matrix::ComplexElements | Matrix::Hermitian;
    this->DiagonalElements = new double [this->NbrRow];
    this->RealOffDiagonalElements = new double [(((long) this->NbrRow) * (((long) this->NbrRow) - 1)) / 2l];
    this->ImaginaryOffDiagonalElements = new double [(((long) this->NbrRow) * (((long) this->NbrRow) - 1)) / 2l];
    Complex Tmp;
    long Index = 0l;
    for (int i = 0; i < this->NbrRow; ++i)
    {
        M.GetMatrixElement(i, i, Tmp);
        this->DiagonalElements[i] = Tmp.Re;
        for (int j = i + 1; j < this->NbrRow; ++j)
        {
            M.GetMatrixElement(i, j, Tmp);
            this->RealOffDiagonalElements[Index] = Tmp.Re;
            this->ImaginaryOffDiagonalElements[Index] = Tmp.Im;
            ++Index;
        }
    }
#ifdef __LAPACK__
    this->LapackWorkAreaDimension=0;
    this->LapackEVsRequested=0;
    this->LapackEVMatrix=NULL;
    this->LapackWorkAreaForPartialDiag=false;
#endif
}

// destructor
//

HermitianMatrix::~HermitianMatrix()
{
    if ((this->RealOffDiagonalElements != 0) && (this->ImaginaryOffDiagonalElements != 0)
            && (this->DiagonalElements != 0) && (this->Flag.Used() == true))
        if (this->Flag.Shared() == false)
        {
            delete[] this->DiagonalElements;
            delete[] this->RealOffDiagonalElements;
            delete[] this->ImaginaryOffDiagonalElements;
        }
#ifdef __LAPACK__
    if (this->LapackWorkAreaDimension>0)
    {
        delete [] LapackMatrix;
        delete [] LapackWorkingArea;
        delete [] LapackRealWorkingArea;
        if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
        if (this->LapackWorkAreaForPartialDiag)
        {
            delete [] this->LapackIntWorkingArea;
            delete [] this->LapackFailedToConverge;
        }
    }
#endif
}

// assignement (without duplicating datas)
//
// M = matrix to copy
// return value = reference on modified matrix

HermitianMatrix& HermitianMatrix::operator = (const HermitianMatrix& M)
{
    if ((this->RealOffDiagonalElements != 0) && (this->ImaginaryOffDiagonalElements != 0)
            && (this->DiagonalElements != 0) && (this->Flag.Used() == true))
        if (this->Flag.Shared() == false)
        {
            delete[] this->DiagonalElements;
            delete[] this->RealOffDiagonalElements;
            delete[] this->ImaginaryOffDiagonalElements;
        }
    this->Flag = M.Flag;
    this->DiagonalElements = M.DiagonalElements;
    this->RealOffDiagonalElements = M.RealOffDiagonalElements;
    this->ImaginaryOffDiagonalElements = M.ImaginaryOffDiagonalElements;
    this->NbrRow = M.NbrRow;
    this->NbrColumn = M.NbrColumn;
    this->TrueNbrRow = M.TrueNbrRow;
    this->TrueNbrColumn = M.TrueNbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
#ifdef __LAPACK__
    if (this->LapackWorkAreaDimension>0)
    {
        delete [] LapackMatrix;
        delete [] LapackWorkingArea;
        delete [] LapackRealWorkingArea;
        if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
        if (this->LapackWorkAreaForPartialDiag)
        {
            delete [] this->LapackIntWorkingArea;
            delete [] this->LapackFailedToConverge;
        }
    }
    this->LapackWorkAreaDimension=0;
    this->LapackWorkAreaForPartialDiag=false;
#endif
    return *this;
}

// assignement from  a real tridiagonal symmetric matrix (without duplicating datas)
//
// M = matrix to copy
// return value = reference on modified matrix

HermitianMatrix& HermitianMatrix::operator = (const RealTriDiagonalSymmetricMatrix& M)
{
    return *this;
}

// return pointer on a clone matrix (without duplicating datas)
//
// retrun value = pointer on new matrix

Matrix* HermitianMatrix::Clone ()
{
    return ((Matrix*) new HermitianMatrix (*this));
}

// copy matrix
//
// M = matrix to copy
// return value = refence on current matrix

HermitianMatrix& HermitianMatrix::Copy (HermitianMatrix& M)
{
    if (this->NbrRow != M.NbrRow)
        this->Resize(M.NbrRow, M.NbrColumn);
    int Pos1 = 0;
    int Pos2 = 0;
    for (int i = 0; i < M.NbrColumn; i++)
    {
        for (int j = i + 1; j < M.NbrColumn; ++j)
        {
            this->RealOffDiagonalElements[Pos1] = M.RealOffDiagonalElements[Pos2];
            this->ImaginaryOffDiagonalElements[Pos1] = M.ImaginaryOffDiagonalElements[Pos2];
            ++Pos1;
            ++Pos2;
        }
        Pos1 += this->Increment;
        Pos2 += M.Increment;
        this->DiagonalElements[i] = M.DiagonalElements[i];
    }
    return *this;
}


// set a matrix element
//
// i = line position
// j = column position
// x = new value for matrix element

void HermitianMatrix::SetMatrixElement(int i, int j, double x)
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    if (i == j)
    {
        this->DiagonalElements[i] = x;
    }
    else
    {
        if (i > j)
        {
            int tmp = j;
            j = i;
            i = tmp;
        }
        long Tmp = (long) j;
        Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
        this->RealOffDiagonalElements[Tmp] = x;
        this->ImaginaryOffDiagonalElements[Tmp] = 0.0;
    }
}

// set a matrix element
//
// i = line position
// j = column position
// x = new value for matrix element

void HermitianMatrix::SetMatrixElement(int i, int j, const Complex& x)
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    if (i == j)
    {
        this->DiagonalElements[i] = x.Re;
        return;
    }
    else
    {
        if (i > j)
        {
            long Tmp = (long) i;
            Tmp -= ((long) j) * ((long) (j - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
            this->RealOffDiagonalElements[Tmp] = x.Re;
            this->ImaginaryOffDiagonalElements[Tmp] = -x.Im;
        }
        else
        {
            long Tmp = (long) j;
            Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
            this->RealOffDiagonalElements[Tmp] = x.Re;
            this->ImaginaryOffDiagonalElements[Tmp] = x.Im;
        }
    }
}

// get a matrix element (real part if complex)
//
// i = line position
// j = column position
// x = reference on the variable where to store the requested matrix element

void HermitianMatrix::GetMatrixElement(int i, int j, double& x) const
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    if (i == j)
    {
        x = this->DiagonalElements[i];
    }
    else
    {
        if (i > j)
        {
            int tmp = j;
            j = i;
            i = tmp;
        }
        long Tmp = (long) j;
        Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
        x = this->RealOffDiagonalElements[Tmp];
    }
}

// get a matrix element
//
// i = line position
// j = column position
// x = reference on the variable where to store the requested matrix element

void HermitianMatrix::GetMatrixElement(int i, int j, Complex& x) const
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    if (i == j)
    {
        x = this->DiagonalElements[i];
    }
    else
    {
        if (i > j)
        {
            long Tmp = (long) i;
            Tmp -= ((long) j) * ((long) (j - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
            x.Re = this->RealOffDiagonalElements[Tmp];
            x.Im = -this->ImaginaryOffDiagonalElements[Tmp];
        }
        else
        {
            long Tmp = (long) j;
            Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
            x.Re = this->RealOffDiagonalElements[Tmp];
            x.Im = this->ImaginaryOffDiagonalElements[Tmp];
        }
    }
}

// add a value to a matrix element
//
// i = line position
// j = column position
// x = value to add to matrix element

void HermitianMatrix::AddToMatrixElement(int i, int j, double x)
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    if (i == j)
    {
        this->DiagonalElements[i] += x;
    }
    else
    {
        if (i > j)
        {
            int tmp = j;
            j = i;
            i = tmp;
        }
        long Tmp = (long) j;
        Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
        this->RealOffDiagonalElements[Tmp] += x;
    }
}

// add a value  a matrix element
//
// i = line position
// j = column position
// x = value to add to matrix element

void HermitianMatrix::AddToMatrixElement(int i, int j, const Complex& x)
{
    if ((i >= this->NbrRow) || (j >= this->NbrColumn))
        return;
    else
    {
        if (i == j)
        {
            this->DiagonalElements[i] += x.Re;
        }
        else
        {
            if (i > j)
            {
                long Tmp = (long) i;
                Tmp -= ((long) j) * ((long) (j - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
                this->RealOffDiagonalElements[Tmp] += x.Re;
                this->ImaginaryOffDiagonalElements[Tmp] -= x.Im;
            }
            else
            {
                long Tmp = (long) j;
                Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
                this->RealOffDiagonalElements[Tmp] += x.Re;
                this->ImaginaryOffDiagonalElements[Tmp] += x.Im;
            }
        }
    }
}

// Resize matrix
//
// nbrRow = new number of rows
// nbrColumn = new number of columns

void HermitianMatrix::Resize (int nbrRow, int nbrColumn)
{
    if (nbrRow != nbrColumn)
        return;
    if (nbrRow <= this->TrueNbrRow)
    {
        this->NbrRow = nbrRow;
        this->NbrColumn = nbrColumn;
        this->Increment = (this->TrueNbrRow - this->NbrRow);
        return;
    }
    double* TmpDiag = new double [nbrRow];
    long Tot = (((long) nbrRow) * ((long) (nbrRow - 1))) / 2l;
    double* TmpRealOffDiag = new double [Tot];
    double* TmpImaginaryOffDiag = new double [Tot];
    for (int i = 0; i < this->NbrRow; i++)
        TmpDiag [i] = this->DiagonalElements[i];
    for (int i = this->NbrRow; i < nbrRow; i++)
        TmpDiag [i]  = 0.0;
    int k = 0;
    int l = 0;
    for (int i = 0; i < (this->NbrRow - 1); ++i)
    {
        for (int j = 0; j < (this->NbrRow - 1 - i); ++j)
        {
            TmpRealOffDiag[k] = this->RealOffDiagonalElements[l];
            TmpImaginaryOffDiag[k] = this->ImaginaryOffDiagonalElements[l];
            ++l;
            ++k;
        }
        l += this->Increment;
        for (int j = this->NbrRow; j < nbrRow; j++)
        {
            TmpRealOffDiag[k] = 0.0;
            TmpImaginaryOffDiag[k] = 0.0;
            ++k;
        }
    }
    for (long i = (((long) this->NbrRow) * ((long) (this->NbrRow - 1))) / 2l; i < Tot; ++i)
    {
        TmpRealOffDiag[i] = 0.0;
        TmpImaginaryOffDiag[i] = 0.0;
    }
    if ((this->RealOffDiagonalElements != 0) && (this->ImaginaryOffDiagonalElements != 0)
            && (this->DiagonalElements != 0) && (this->Flag.Used() == true))
        if (this->Flag.Shared() == false)
        {
            delete[] this->DiagonalElements;
            delete[] this->RealOffDiagonalElements;
            delete[] this->ImaginaryOffDiagonalElements;
        }
    this->NbrRow = nbrRow;
    this->NbrColumn = nbrColumn;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->Flag.Initialize();
    this->DiagonalElements = TmpDiag;
    this->RealOffDiagonalElements = TmpRealOffDiag;
    this->ImaginaryOffDiagonalElements = TmpImaginaryOffDiag;
}

// Resize matrix and set to zero all elements that have been added
//
// nbrRow = new number of rows
// nbrColumn = new number of columns

void HermitianMatrix::ResizeAndClean (int nbrRow, int nbrColumn)
{
    if (nbrRow != nbrColumn)
        return;
    if (nbrRow <= this->TrueNbrRow)
    {
        if (this->NbrRow < nbrRow)
        {
            long Tot = (((long) nbrRow) * ((long) (nbrRow - 1))) / 2l;
            for (int i = this->NbrRow; i < nbrRow; ++i)
                this->DiagonalElements[i] = 0.0;
            long k = 2l * (this->NbrRow - 1);
            for (int i = 0; i < (this->NbrRow - 1); ++i)
            {
                for (int j = this->NbrRow; j < nbrRow; ++j)
                {
                    this->RealOffDiagonalElements[k] = 0.0;
                    this->ImaginaryOffDiagonalElements[k] = 0.0;
                    ++k;
                }
                k += (this->NbrRow - 2 - i);
            }
            for (long i = (((long) this->NbrRow) * ((long) (this->NbrRow - 1))) / 2l; i < Tot; i++)
            {
                this->RealOffDiagonalElements[i] = 0.0;
                this->ImaginaryOffDiagonalElements[i] = 0.0;
            }
        }
        this->NbrRow = nbrRow;
        this->NbrColumn = nbrColumn;
        this->Increment = (this->TrueNbrRow - this->NbrRow);
        return;
    }
    double* TmpDiag = new double [nbrRow];
    long Tot = (((long) nbrRow) * (((long) nbrRow) - 1l)) / 2l;
    double* TmpRealOffDiag = new double [Tot];
    double* TmpImaginaryOffDiag = new double [Tot];
    for (int i = 0; i < this->NbrRow; i++)
        TmpDiag [i] = this->DiagonalElements[i];
    for (int i = this->NbrRow; i < nbrRow; i++)
        TmpDiag [i]  = 0.0;
    long k = 0l;
    long l = 0l;
    for (int i = 0; i < (this->NbrRow - 1); i++)
    {
        for (int j = 0; j < (this->NbrRow - 1 - i); j++)
        {
            TmpRealOffDiag[k] = this->RealOffDiagonalElements[l];
            TmpImaginaryOffDiag[k] = this->ImaginaryOffDiagonalElements[l];
            ++l;
            ++k;
        }
        for (int j = this->NbrRow; j < nbrRow; j++)
        {
            TmpRealOffDiag[k] = 0.0;
            TmpImaginaryOffDiag[k] = 0.0;
            ++k;
        }
    }
    for (long i = (((long) this->NbrRow) * (((long) this->NbrRow) - 1l)) / 2l; i < Tot; i++)
    {
        TmpRealOffDiag[i] = 0.0;
        TmpImaginaryOffDiag[i] = 0.0;
    }
    if ((this->RealOffDiagonalElements != 0) && (this->ImaginaryOffDiagonalElements != 0)
            && (this->DiagonalElements != 0) && (this->Flag.Used() == true))
        if (this->Flag.Shared() == false)
        {
            delete[] this->DiagonalElements;
            delete[] this->RealOffDiagonalElements;
            delete[] this->ImaginaryOffDiagonalElements;
        }
    this->NbrRow = nbrRow;
    this->NbrColumn = nbrColumn;
    this->TrueNbrRow = this->NbrRow;
    this->TrueNbrColumn = this->NbrColumn;
    this->Increment = (this->TrueNbrRow - this->NbrRow);
    this->Flag.Initialize();
    this->DiagonalElements = TmpDiag;
    this->RealOffDiagonalElements = TmpRealOffDiag;
    this->ImaginaryOffDiagonalElements = TmpImaginaryOffDiag;
}

// Set all entries in matrix to zero
//

void HermitianMatrix::ClearMatrix ()
{
    int pos = 0.0;
    for (int i = 0; i < this->NbrRow; i++)
    {
        this->DiagonalElements[i] = 0.0;
        for (int j = i + 1; j < this->NbrRow; j++)
        {
            this->RealOffDiagonalElements[pos] = 0.0;
            this->ImaginaryOffDiagonalElements[pos] = 0.0;
            pos++;
        }
    }
    return;
}

// set matrix to identity
//

void HermitianMatrix::SetToIdentity()
{
    int pos = 0.0;
    for (int i = 0; i < this->NbrRow; i++)
    {
        this->DiagonalElements[i] = 1.0;
        for (int j = i + 1; j < this->NbrRow; j++)
        {
            this->RealOffDiagonalElements[pos] = 0.0;
            this->ImaginaryOffDiagonalElements[pos] = 0.0;
            pos++;
        }
    }
    return;
}


// return reference on real part of a given matrix element
// to access the full complex valued matrix element, use GetMatrixElement
//
// i = line position
// j = column position
// return value = reference on real part

double& HermitianMatrix::operator () (int i, int j)
{
    if (i == j)
    {
        return this->DiagonalElements[i];
    }
    else
    {
        if (i > j)
        {
            int tmp = j;
            j = i;
            i = tmp;
        }
        long Tmp = (long) j;
        Tmp -= ((long) i) * ((long) (i - 2 * (this->NbrRow + this->Increment) + 3)) / 2l + 1l;
        return this->RealOffDiagonalElements[Tmp];
    }
}


// add two matrices
//
// M1 = first matrix
// M2 = second matrix
// return value = sum of the two matrices

HermitianMatrix operator + (const HermitianMatrix& M1, const HermitianMatrix& M2)
{
    if (M1.NbrRow != M2.NbrRow)
        return HermitianMatrix();
    double* Diagonal = new double [M1.NbrRow];
    int ReducedNbr = M1.NbrRow - 1;
    long max = (((long) ReducedNbr) * ((long) M1.NbrRow)) / 2l;
    double* RealOffDiagonal = new double [max];
    double* ImaginaryOffDiagonal = new double [max];
    for (int i = 0; i < M1.NbrRow; i++)
    {
        Diagonal[i] = M1.DiagonalElements[i] + M2.DiagonalElements[i];
    }
    for (long i = 0l; i < max; ++i)
    {
        RealOffDiagonal[i] = M1.RealOffDiagonalElements[i] + M2.RealOffDiagonalElements[i];
        ImaginaryOffDiagonal[i] = M1.ImaginaryOffDiagonalElements[i] + M2.ImaginaryOffDiagonalElements[i];
    }
    return HermitianMatrix(Diagonal, RealOffDiagonal, ImaginaryOffDiagonal, M1.NbrRow);
}

// add two matrices where the left one is a real tridiagonal symmetric matrix
//
// M1 = left matrix
// M2 = right matrix
// return value = sum of the two matrices

HermitianMatrix operator + (const RealTriDiagonalSymmetricMatrix& M1, const HermitianMatrix& M2)
{
    return HermitianMatrix();
}

// add two matrices where the right one is a real tridiagonal symmetric matrix
//
// M1 = left matrix
// M2 = right matrix
// return value = sum of the two matrices

HermitianMatrix operator + (const HermitianMatrix& M1, const RealTriDiagonalSymmetricMatrix& M2)
{
    return HermitianMatrix();
}

// substract two matrices
//
// M1 = first matrix
// M2 = matrix to substract to M1
// return value = difference of the two matrices

HermitianMatrix operator - (const HermitianMatrix& M1, const HermitianMatrix& M2)
{
    if (M1.NbrRow != M2.NbrRow)
        return HermitianMatrix();
    double* Diagonal = new double [M1.NbrRow];
    int ReducedNbr = M1.NbrRow - 1;
    long max = (((long) ReducedNbr) * ((long) M1.NbrRow)) / 2l;
    double* RealOffDiagonal = new double [max];
    double* ImaginaryOffDiagonal = new double [max];
    for (int i = 0; i < M1.NbrRow; ++i)
    {
        Diagonal[i] = M1.DiagonalElements[i] - M2.DiagonalElements[i];
    }
    for (long i = 0l; i < max; ++i)
    {
        RealOffDiagonal[i] = M1.RealOffDiagonalElements[i] - M2.RealOffDiagonalElements[i];
        ImaginaryOffDiagonal[i] = M1.ImaginaryOffDiagonalElements[i] - M2.ImaginaryOffDiagonalElements[i];
    }
    return HermitianMatrix(Diagonal, RealOffDiagonal, ImaginaryOffDiagonal, M1.NbrRow);
}

// substract two matrices where the left one is a real tridiagonal symmetric matrix
//
// M1 = first matrix
// M2 = matrix to substract to M1
// return value = difference of the two matrices

HermitianMatrix operator - (const RealTriDiagonalSymmetricMatrix& M1, const HermitianMatrix& M2)
{
    return HermitianMatrix();
}

// substract two matrices where the right one is a real tridiagonal symmetric matrix
//
// M1 = first matrix
// M2 = matrix to substract to M1
// return value = difference of the two matrices

HermitianMatrix operator - (const HermitianMatrix& M1,  const RealTriDiagonalSymmetricMatrix& M2)
{
    return HermitianMatrix();
}

// multiply a matrix by a real number (right multiplication)
//
// M = source matrix
// x = real number to use
// return value = product result

HermitianMatrix operator * (const HermitianMatrix& M, double x)
{
    double* Diagonal = new double [M.NbrRow];
    int ReducedNbr = M.NbrRow - 1;
    long max = (((long) ReducedNbr) * ((long) M.NbrRow)) / 2l;
    double* RealOffDiagonal = new double [max];
    double* ImaginaryOffDiagonal = new double [max];
    for (int i = 0; i < M.NbrRow; ++i)
    {
        Diagonal[i] = M.DiagonalElements[i] * x;
    }
    for (long i = 0l; i < max; ++i)
    {
        RealOffDiagonal[i] = M.RealOffDiagonalElements[i] * x;
        ImaginaryOffDiagonal[i] = M.ImaginaryOffDiagonalElements[i] * x;
    }
    return HermitianMatrix(Diagonal, RealOffDiagonal, ImaginaryOffDiagonal, M.NbrRow);
}

// multiply a matrix by a real number (left multiplication)
//
// M = source matrix
// x = real number to use
// return value = product result

HermitianMatrix operator * (double x, const HermitianMatrix& M)
{
    double* Diagonal = new double [M.NbrRow];
    int ReducedNbr = M.NbrRow - 1;
    long max = (((long) ReducedNbr) * ((long) M.NbrRow)) / 2l;
    double* RealOffDiagonal = new double [max];
    double* ImaginaryOffDiagonal = new double [max];
    for (int i = 0; i < M.NbrRow; ++i)
    {
        Diagonal[i] = M.DiagonalElements[i] * x;
    }
    for (long i = 0l; i < max; ++i)
    {
        RealOffDiagonal[i] = M.RealOffDiagonalElements[i] * x;
        ImaginaryOffDiagonal[i] = M.ImaginaryOffDiagonalElements[i] * x;
    }
    return HermitianMatrix(Diagonal, RealOffDiagonal, ImaginaryOffDiagonal, M.NbrRow);
}

// divide a matrix by a real number (right multiplication)
//
// M = source matrix
// x = real number to use
// return value = division result

HermitianMatrix operator / (const HermitianMatrix& M, double x)
{
    double* Diagonal = new double [M.NbrRow];
    int ReducedNbr = M.NbrRow - 1;
    long max = (((long) ReducedNbr) * ((long) M.NbrRow)) / 2l;
    double* RealOffDiagonal = new double [max];
    double* ImaginaryOffDiagonal = new double [max];
    double inv = 1.0 / x;
    for (int i = 0; i < M.NbrRow; ++i)
    {
        Diagonal[i] = M.DiagonalElements[i] * inv;
    }
    for (long i = 0l; i < max; ++i)
    {
        RealOffDiagonal[i] = M.RealOffDiagonalElements[i] * inv;
        ImaginaryOffDiagonal[i] = M.ImaginaryOffDiagonalElements[i] * inv;
    }
    return HermitianMatrix(Diagonal, RealOffDiagonal, ImaginaryOffDiagonal, M.NbrRow);
}

// add two matrices
//
// M = matrix to add to current matrix
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator += (const HermitianMatrix& M)
{
    if (this->NbrRow == 0)
        return *this;
    for (int i = 0; i < this->NbrRow; ++i)
    {
        this->DiagonalElements[i] += M.DiagonalElements[i];
    }
    long max = (((long) (this->NbrRow - 1)) * ((long) this->NbrRow)) >> 1;
    for (long i = 0l; i < max; ++i)
    {
        this->RealOffDiagonalElements[i] += M.RealOffDiagonalElements[i];
        this->ImaginaryOffDiagonalElements[i] += M.ImaginaryOffDiagonalElements[i];
    }
    return *this;
}

// add two matrices where the right one is a real tridiagonal symmetric matrix
//
// M = matrix to add to current matrix
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator += (const RealTriDiagonalSymmetricMatrix& M)
{
    return *this;
}

// substract two matrices
//
// M = matrix to substract to current matrix
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator -= (const HermitianMatrix& M)
{
    if (this->NbrRow == 0)
        return *this;
    for (int i = 0; i < this->NbrRow; ++i)
    {
        this->DiagonalElements[i] -= M.DiagonalElements[i];
    }
    long max = (((long) (this->NbrRow - 1)) * ((long) this->NbrRow)) / 2l;
    for (long i = 0l; i < max; ++i)
    {
        this->RealOffDiagonalElements[i] -= M.RealOffDiagonalElements[i];
        this->ImaginaryOffDiagonalElements[i] -= M.ImaginaryOffDiagonalElements[i];
    }
    return *this;
}

// substract two matrices where the right one is a real tridiagonal symmetric matrix
//
// M = matrix to substract to current matrix
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator -= (const RealTriDiagonalSymmetricMatrix& M)
{
    return *this;
}

// multiply a matrix by a real number
//
// x = real number to use
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator *= (double x)
{
    if (this->NbrRow == 0)
        return *this;
    for (int i = 0; i < this->NbrRow; ++i)
    {
        this->DiagonalElements[i] *= x;
    }
    long max = (((long) (this->NbrRow - 1)) * ((long) this->NbrRow)) / 2l;
    for (long i = 0l; i < max; ++i)
    {
        this->ImaginaryOffDiagonalElements[i] *= x;
        this->RealOffDiagonalElements[i] *= x;
    }
    return *this;
}

// divide a matrix by a real number
//
// x = real number to use
// return value = reference on current matrix

HermitianMatrix& HermitianMatrix::operator /= (double x)
{
    if (this->NbrRow == 0)
        return *this;
    x = 1.0 / x;
    for (int i = 0; i < this->NbrRow; i++)
    {
        this->DiagonalElements[i] *= x;
    }
    long max = (((long) (this->NbrRow - 1)) * ((long) this->NbrRow)) / 2l;
    for (long i = 0l; i < max; ++i)
    {
        this->RealOffDiagonalElements[i] *= x;
        this->ImaginaryOffDiagonalElements[i] *= x;
    }
    return *this;
}

// evaluate matrix element
//
// V1 = vector to left multiply with current matrix
// V2 = vector to right multiply with current matrix
// return value = corresponding matrix element

Complex HermitianMatrix::MatrixElement (ComplexVector& V1, ComplexVector& V2)
{
    Complex Z(0.0, 0.0);
    if ((V1.Dimension != this->NbrRow) || (V2.Dimension != this->NbrColumn))
        return Z;
    for (int i = 0; i < this->NbrRow ; ++i)
    {
        Complex Z2(this->DiagonalElements[i] * V2.Components[i].Re, this->DiagonalElements[i] * V2.Components[i].Im);
        int l = (i - 1);
        for (int k = 0; k < i; ++k)
        {
            Z2.Re += (this->RealOffDiagonalElements[l] * V2.Components[k].Re +
                      this->ImaginaryOffDiagonalElements[l] * V2.Components[k].Im);
            Z2.Im += (this->RealOffDiagonalElements[l] * V2.Components[k].Im -
                      this->ImaginaryOffDiagonalElements[l] * V2.Components[k].Re);
            l += (this->NbrColumn - 2 - k) + this->Increment;
        }
        ++l;
        for (int k = i + 1; k < this->NbrColumn; ++k)
        {
            Z2.Re += (this->RealOffDiagonalElements[l] * V2.Components[k].Re -
                      this->ImaginaryOffDiagonalElements[l] * V2.Components[k].Im);
            Z2.Im += (this->RealOffDiagonalElements[l] * V2.Components[k].Im +
                      this->ImaginaryOffDiagonalElements[l] * V2.Components[k].Re);
            ++l;
        }
        Z += Z2 * Complex (V1.Components[i].Re, -V1.Components[i].Im);
    }
    return Z;
}

// conjugate matrix with an unitary matrix (Ut M U)
//
// UnitaryM = unitary matrix to use
// return value = pointer to conjugated matrix

Matrix* HermitianMatrix::Conjugate(RealMatrix& UnitaryM)
{
    if (UnitaryM.NbrRow != this->NbrColumn)
        return 0;
    double* TmpDiag = new double [UnitaryM.NbrColumn];
    int NbrOffDiag = (UnitaryM.NbrColumn * (UnitaryM.NbrColumn - 1)) / 2;
    double* TmpRealOffDiag = new double [NbrOffDiag];
    double* TmpImaginaryOffDiag = new double [NbrOffDiag];
    for (int i = 0; i < UnitaryM.NbrColumn; i++)
    {
        TmpDiag[i] = 0.0;
        for (int j = 0; j < this->NbrColumn; ++j)
        {
            double tmp = 0.0;
            int k = 0;
            int l = (j - 1);
            for (; k < j; ++k)
            {
                tmp += this->RealOffDiagonalElements[l] * UnitaryM.Columns[i].Components[k];
                l += (this->NbrColumn - 2 - k) + this->Increment;
            }
            ++l;
            tmp += this->DiagonalElements[j] * UnitaryM.Columns[i].Components[j];
            ++k;
            for (; k < this->NbrColumn; k++)
            {
                tmp += this->RealOffDiagonalElements[l] * UnitaryM.Columns[i].Components[k];
                ++l;
            }
            TmpDiag[i] += tmp * UnitaryM.Columns[i].Components[j];
        }
    }
    int ReducedNbrColumn = UnitaryM.NbrColumn - 1;
    for (int i = 0; i < ReducedNbrColumn; i++)
        for (int m = i + 1; m < UnitaryM.NbrColumn; m++)
        {
            TmpRealOffDiag[i] = 0.0;
            TmpImaginaryOffDiag[i] = 0.0;
            for (int j = 0; j < this->NbrColumn; j++)
            {
                double tmp1 = 0.0;
                double tmp2 = 0.0;
                int k = 0;
                int l = (j - 1);
                for (; k < j; k++)
                {
                    tmp1 += this->RealOffDiagonalElements[l] * UnitaryM.Columns[m].Components[k];
                    tmp2 -= this->ImaginaryOffDiagonalElements[l] * UnitaryM.Columns[m].Components[k];
                    l += (this->NbrColumn - 3 - k) + this->Increment + 1;
                }
                ++l;
                tmp1 += this->DiagonalElements[j] * UnitaryM.Columns[m].Components[j];
                ++k;
                for (; k < this->NbrColumn; ++k)
                {
                    tmp1 += this->RealOffDiagonalElements[l] * UnitaryM.Columns[m].Components[k];
                    tmp2 += this->ImaginaryOffDiagonalElements[l] * UnitaryM.Columns[m].Components[k];
                    ++l;
                }
                TmpRealOffDiag[i] += tmp1 * UnitaryM.Columns[i].Components[j];
                TmpImaginaryOffDiag[i] += tmp2 * UnitaryM.Columns[i].Components[j];
            }
        }
    return new HermitianMatrix(TmpDiag, TmpRealOffDiag, TmpImaginaryOffDiag, UnitaryM.NbrColumn);
}

// conjugate an hermitian matrix with an unitary matrix (Ut M U)
//
// UnitaryM = unitary matrix to use
// return value = conjugated matrix

HermitianMatrix HermitianMatrix::Conjugate(ComplexMatrix& UnitaryM)
{
    if (UnitaryM.NbrRow != this->NbrColumn)
        return HermitianMatrix();
    HermitianMatrix TmpMatrix (UnitaryM.NbrRow, true);
    Complex Tmp2;
    for (int i = 0; i < UnitaryM.NbrRow; ++i)
        for (int j = i; j < UnitaryM.NbrRow; ++j)
        {
            Complex Tmp = 0.0;
            for (int k = 0; k < UnitaryM.NbrRow; ++k)
            {
                Complex Tmp3 = 0.0;
                for (int l = 0; l < UnitaryM.NbrRow; ++l)
                {
                    this->GetMatrixElement(k, l, Tmp2);
                    Tmp3 +=  Tmp2 * UnitaryM.Columns[j][l];
                }
                Tmp += Tmp3 * Conj(UnitaryM.Columns[i][k]);
            }
            TmpMatrix.SetMatrixElement(i, j, Tmp);
        }
    return TmpMatrix;
}

// conjugate an hermitian matrix with an hermitian transposed unitary matrix (U M Ut)
//
// UnitaryM = unitary matrix to use
// return value = conjugated matrix

HermitianMatrix HermitianMatrix::InvConjugate(ComplexMatrix& UnitaryM)
{
    if (UnitaryM.NbrRow != this->NbrColumn)
        return HermitianMatrix();
    HermitianMatrix TmpMatrix (UnitaryM.NbrRow, true);
    Complex Tmp2;
    for (int i = 0; i < UnitaryM.NbrRow; ++i)
        for (int j = 0; j < UnitaryM.NbrRow; ++j)
        {
            Complex Tmp = 0.0;
            for (int k = 0; k < UnitaryM.NbrRow; ++k)
            {
                Complex Tmp3 = 0.0;
                for (int l = 0; l < UnitaryM.NbrRow; ++l)
                {
                    this->GetMatrixElement(l, k, Tmp2);
                    Tmp3 +=  Tmp2 * Conj(UnitaryM.Columns[l][j]);
                }
                Tmp += Tmp3 * UnitaryM.Columns[k][i];
            }
            TmpMatrix.SetMatrixElement(i, j, Tmp);
        }
    return TmpMatrix;
}

// evaluate matrix trace
//
// return value = matrix trace

double HermitianMatrix::Tr ()
{
    if (this->NbrRow == 0)
        return 0.0;
    double x = this->DiagonalElements[0];
    for (int i = 1; i < this->NbrRow; i++)
    {
        x += this->DiagonalElements[i];
    }
    return x;
}

// evaluate matrix determinant
//
// return value = matrix determinant

double HermitianMatrix::Det ()
{
    return 1.0;
}

// Output Stream overload
//
// Str = reference on output stream
// P = matrix to print
// return value = reference on output stream

ostream& operator << (ostream& Str, const HermitianMatrix& P)
{
    for (int i = 0; i < P.NbrRow; i++)
    {
        int pos = (i - 1);
        for (int j = 0; j < i; ++j)
        {
            Str << P.RealOffDiagonalElements[pos];
            if (P.ImaginaryOffDiagonalElements[pos] > 0.0)
                Str << -P.ImaginaryOffDiagonalElements[pos] << "i    ";
            else if (P.ImaginaryOffDiagonalElements[pos] != 0.0)
                Str << "+" << -P.ImaginaryOffDiagonalElements[pos] << "i    ";
            else
                Str << "    ";
            pos += (P.NbrRow - j - 2) + P.Increment;
        }
        Str << P.DiagonalElements[i] << "    ";
        ++pos;
        for (int j = i + 1; j < P.NbrRow; ++j)
        {
            Str << P.RealOffDiagonalElements[pos];
            if (P.ImaginaryOffDiagonalElements[pos] < 0.0)
                Str << P.ImaginaryOffDiagonalElements[pos] << "i    ";
            else if (P.ImaginaryOffDiagonalElements[pos] != 0.0)
                Str << "+" << P.ImaginaryOffDiagonalElements[pos] << "i    ";
            else
                Str << "    ";
            ++pos;
        }
        Str << endl;
    }
    return Str;
}

#ifdef USE_OUTPUT
// Mathematica Output Stream overload
//
// Str = reference on Mathematica output stream
// P = matrix to print
// return value = reference on output stream

MathematicaOutput& operator << (MathematicaOutput& Str, const HermitianMatrix& P)
{
    Str << "{";
    for (int i = 0; i < P.NbrRow; ++i)
    {
        Str << "{";
        int pos = i - 1;
        for (int j = 0; j < i; ++j)
        {
            if ((P.RealOffDiagonalElements[pos] != 0) || (P.ImaginaryOffDiagonalElements[pos] == 0))
                Str << P.RealOffDiagonalElements[pos];
            if (P.ImaginaryOffDiagonalElements[pos] > 0.0)
                Str << -P.ImaginaryOffDiagonalElements[pos] << "I";
            else if (P.ImaginaryOffDiagonalElements[pos] != 0.0)
                Str << "+" << -P.ImaginaryOffDiagonalElements[pos] << "I";
            Str << ",";
            pos += (P.NbrRow - j - 2) + P.Increment;
        }
        Str << P.DiagonalElements[i];
        if (i != (P.NbrRow - 1))
        {
            Str << ",";
            ++pos;
            for (int j = i + 1; j < (P.NbrRow - 1); ++j)
            {
                if ((P.RealOffDiagonalElements[pos] != 0) || (P.ImaginaryOffDiagonalElements[pos] == 0))
                    Str << P.RealOffDiagonalElements[pos];
                if (P.ImaginaryOffDiagonalElements[pos] < 0.0)
                    Str << P.ImaginaryOffDiagonalElements[pos] << "I";
                else if (P.ImaginaryOffDiagonalElements[pos] != 0.0)
                    Str << "+" << P.ImaginaryOffDiagonalElements[pos] << "I";
                Str << ",";
                ++pos;
            }
            Str << P.RealOffDiagonalElements[pos];
            if (P.ImaginaryOffDiagonalElements[pos] < 0.0)
                Str << P.ImaginaryOffDiagonalElements[pos] << "I";
            else if (P.ImaginaryOffDiagonalElements[pos] != 0.0)
                Str << "+" << P.ImaginaryOffDiagonalElements[pos] << "I";
            Str << "},";
        }
        else
            Str << "}";
    }
    Str << "}";
    return Str;
}
#endif

// Tridiagonalize an hermitian matrix using Lanczos algorithm without re-orthogonalizing base at each step
//
// dimension = maximum iteration number
// M = reference on real tridiagonal symmetric matrix where result has to be stored
// V1 = reference on complex vector used as first vector (will contain last produced vector at the end)
// return value = reference on complex tridiagonal hermitian matrix

RealTriDiagonalSymmetricMatrix& HermitianMatrix::Lanczos (int dimension, RealTriDiagonalSymmetricMatrix& M, ComplexVector& V1)
{
    int Index = 0;
    V1 /= V1.Norm();
    ComplexVector V2(this->NbrRow);
    ComplexVector V3(this->NbrRow);
    V2.Multiply(*this, V1);
    M.DiagonalElements[Index] = (V1 * V2).Re;
    V2.AddLinearCombination(-M.DiagonalElements[Index], V1);
    V2 /= V2.Norm();
    dimension -= 2;
    for (int i = 0; i < dimension; i++)
    {
        V3.Multiply(*this, V2);
        M.UpperDiagonalElements[Index] = (V1 * V3).Re;
        M.DiagonalElements[Index + 1] = (V2 * V3).Re;
        V3.AddLinearCombination(-M.DiagonalElements[Index + 1], V2);
        V3.AddLinearCombination(-M.UpperDiagonalElements[Index], V1);
        V3 /= V3.Norm();
        Index++;
        ComplexVector TmpV = V1;
        V1 = V2;
        V2 = V3;
        V3 = TmpV;
    }
    V3.Multiply(*this, V2);
    M.UpperDiagonalElements[Index] = (V1 * V3).Re;
    M.DiagonalElements[Index + 1] = (V2 * V3).Re;
    ComplexVector TmpV = V1;
    V1 = V2;
    V2 = TmpV;
    return M;
}

// Tridiagonalize an hermitian matrix using Lanczos algorithm without re-orthogonalizing base at each step
//
// dimension = maximum iteration number
// M = reference on real tridiagonal symmetric matrix where result has to be stored
// Q = matrix where new orthonormalized base will be stored (first column is used as first vector)
// return value = reference on complex tridiagonal hermitian matrix

RealTriDiagonalSymmetricMatrix& HermitianMatrix::Lanczos (int dimension, RealTriDiagonalSymmetricMatrix& M, ComplexMatrix& Q)
{
    int Index = 0;
    if ((Q.NbrColumn != dimension) || (Q.NbrRow != this->NbrRow))
        Q.Resize(this->NbrRow, dimension);
    if (M.NbrRow != dimension)
        M.Resize(dimension, dimension);
    Q.Columns[0] /= Q.Columns[0].Norm();
    Q.Columns[1].Multiply(*this, Q.Columns[0]);
    M.DiagonalElements[Index] = (Q.Columns[0] * Q.Columns[1]).Re;
    Q.Columns[1].AddLinearCombination(-M.DiagonalElements[Index], Q.Columns[0]);
    Q.Columns[1] /= Q.Columns[1].Norm();
    for (int i = 2; i < dimension; i++)
    {
        Q.Columns[i].Multiply(*this, Q.Columns[i - 1]);
        M.UpperDiagonalElements[Index] = (Q.Columns[i - 2] * Q.Columns[i]).Re;
        M.DiagonalElements[Index + 1] = (Q.Columns[i - 1] * Q.Columns[i]).Re;
        Q.Columns[i].AddLinearCombination(-M.DiagonalElements[Index + 1], Q.Columns[i - 1]);
        Q.Columns[i].AddLinearCombination(-M.UpperDiagonalElements[Index], Q.Columns[i - 2]);
        Q.Columns[i] /= Q.Columns[i].Norm();
        Index++;
    }
    M.UpperDiagonalElements[Index] = this->MatrixElement(Q.Columns[dimension - 2], Q.Columns[dimension - 1]).Re;
    M.DiagonalElements[Index + 1] = this->MatrixElement(Q.Columns[dimension - 1], Q.Columns[dimension - 1]).Re;
    return M;
}

// Tridiagonalize an hermitian matrix using Lanczos algorithm without re-orthogonalizing base at each step, if during process a
// null vector appears then new random vector is evaluated
//
// dimension = maximum iteration number
// M = reference on real tridiagonal symmetric matrix where result has to be stored
// Q = matrix where new orthonormalized base will be stored (first column is used as first vector)
// err = absolute error on vector norm
// return value = reference on complex tridiagonal hermitian matrix

RealTriDiagonalSymmetricMatrix& HermitianMatrix::OrthoLanczos (int dimension, RealTriDiagonalSymmetricMatrix& M, ComplexMatrix& Q,
        double err)
{
    /*  int Index = 0;
      if ((Q.NbrColumn != dimension) || (Q.NbrRow != this->NbrRow))
        Q.Resize(this->NbrRow, dimension);
      if (M.NbrRow != dimension)
        M.Resize(dimension, dimension);
      Q.Columns[0] /= Q.Columns[0].Norm();
      Q.Columns[1].Multiply(*this, Q.Columns[0]);
      M.DiagonalElements[Index] = (Q.Columns[0] * Q.Columns[1]).Re;
      Q.Columns[1].AddLinearCombination(-M.DiagonalElements[Index], Q.Columns[0]);
      Q.Columns[1] /= Q.Columns[1].Norm();
      for (int i = 2; i < dimension; i++)
        {
          Q.Columns[i].Multiply(*this, Q.Columns[i - 1]);
          M.UpperDiagonalElements[Index] = (Q.Columns[i - 2] * Q.Columns[i]).Re;
          M.DiagonalElements[Index + 1] = (Q.Columns[i - 1] * Q.Columns[i]).Re;
          Q.Columns[i].AddLinearCombination(-M.DiagonalElements[Index + 1], Q.Columns[i - 1]);
          Q.Columns[i].AddLinearCombination(-M.UpperDiagonalElements[Index], Q.Columns[i - 2]);
          double VectorNorm = Q.Columns[i].Norm();
          while (VectorNorm < err)
    	{
    	  double tmp = 0;
    	  for (int j = 0; j < Q.NbrRow; j++)
    	    {
    	      Q.Columns[i].Components[2 * j] = rand ();
    	      tmp += Q.Columns[i].Components[2 * j] * Q.Columns[i].Components[2 * j];
    	    }
    	  tmp = sqrt(tmp);
    	  Q.Columns[i] /= tmp;
    	  Q.Columns[i] *= *this;
    	  for (int j = 0; j < i; j++)
    	    {
    	      Q.Columns[i].AddLinearCombination(- (Q.Columns[j] * Q.Columns[i]).Re, Q.Columns[j]);
    	    }
    	  VectorNorm = Q.Columns[i].Norm();
    	}
          Q.Columns[i] /= VectorNorm;
          Index++;
        }
      M.UpperDiagonalElements[Index] = this->MatrixElement(Q.Columns[dimension - 2], Q.Columns[dimension - 1]).Re;
      M.DiagonalElements[Index + 1] = this->MatrixElement(Q.Columns[dimension - 1], Q.Columns[dimension - 1]).Re;*/
    return M;
}

// Tridiagonalize a hermitian matrix using Householder algorithm and evaluate transformation matrix
//
// M = reference on real tridiagonal symmetric matrix where result has to be stored
// err = absolute error on matrix element
// Q = matrix where transformation matrix has to be stored
// return value = reference on real tridiagonal symmetric matrix

RealTriDiagonalSymmetricMatrix& HermitianMatrix::Householder (RealTriDiagonalSymmetricMatrix& M, double err, ComplexMatrix& Q)
{
    if (M.NbrRow != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    if ((Q.NbrRow != M.NbrRow) || (Q.NbrColumn != M.NbrColumn))
        Q.Resize(this->NbrRow, this->NbrColumn);
    for (int i = 0; i < Q.NbrRow; i++)
    {
        for (int j = 0; j < i; j++)
        {
            Q.Columns[j].Components[i].Re = 0.0;
            Q.Columns[j].Components[i].Im = 0.0;
        }
        Q.Columns[i].Components[i].Re = 1.0;
        Q.Columns[i].Components[i].Im = 0.0;
        for (int j = i + 1; j < Q.NbrColumn; j++)
        {
            Q.Columns[j].Components[i].Re = 0.0;
            Q.Columns[j].Components[i].Im = 0.0;
        }
    }
    int ReducedNbrRow = this->NbrRow -1;
    int ReducedNbrRow2 ;
    double* TmpVRe = new double [ReducedNbrRow];
    double* TmpVIm = new double [ReducedNbrRow];
//   double* TmpCoefRe = new double [this->NbrRow];
//   double* TmpCoefIm = new double [this->NbrRow];
    double TmpNorm;
    double Coef;
    int Pos = 0;
    int TmpPos;
//   int TmpPos2;
//   int TmpPos3;
//   int TmpPos4;
//   int TmpPos5;
//   int TmpPos6;
//   int Inc;
    M.DiagonalElement(0) = this->DiagonalElements[0];
    for (int i = 1; i < ReducedNbrRow; ++i)
    {
        ReducedNbrRow2 = this->NbrRow - i;
        TmpNorm = 0.0;
        TmpPos = Pos;
        // construct vector for Householder transformation
        for (int j = 0; j < ReducedNbrRow2; ++j)
        {
            TmpNorm += this->RealOffDiagonalElements[TmpPos] * this->RealOffDiagonalElements[TmpPos];
            TmpNorm += this->ImaginaryOffDiagonalElements[TmpPos] * this->ImaginaryOffDiagonalElements[TmpPos];
            TmpVRe[j] = 0.0;
            TmpVIm[j] = 0.0;
            ++TmpPos;
        }
        TmpPos = Pos;
        Coef = sqrt(TmpNorm);
        M.UpperDiagonalElement(i- 1) = Coef;
        /*      TmpNorm = sqrt(TmpNorm - Coef * this->OffDiagonalElements[Pos]);
              if  (TmpNorm > err)
        	{
        	  TmpNorm = 1.0 / TmpNorm;
         	  this->OffDiagonalElements[Pos] -= Coef;
         	  for (int j = 0; j < ReducedNbrRow2; ++j)
         	    {
         	      this->OffDiagonalElements[TmpPos] *= TmpNorm;
        	      ++TmpPos;
        	    }

        	  // store result of Hamiltonian applied to Householder vector, evaluate coefficients for Q
        	  Coef = 0.0;
                                                                                                                                                           	  TmpPos2 = 0;
        	  TmpPos5 = 1;
        	  TmpPos4 = i * (this->NbrRow + this->Increment - 1) - (i * (i + 1)) / 2 - 1;
        	  Inc = this->NbrRow - 2 + this->Increment;
        	  for (int j = 1; j < i; j++)
        	    {
        	      TmpPos = Pos;
        	      TmpCoefRe[TmpPos5] = 0.0;
        	      TmpCoefIm[TmpPos5] = 0.0;
        	      for (int k = i; k < this->NbrRow; ++k)
        		{
        	 	  TmpCoefRe[TmpPos5] += Q.Columns[k].Components[j].Re * this->RealOffDiagonalElements[TmpPos]
        		    + Q.Columns[k].Components[j].Im * this->ImaginaryOffDiagonalElements[TmpPos];
        	 	  TmpCoefIm[TmpPos5] += Q.Columns[k].Components[j].Re * this->ImaginaryOffDiagonalElements[TmpPos]
        		    -  Q.Columns[k].Components[j].Im * this->RealOffDiagonalElements[TmpPos];
        		  ++TmpPos;
        		}
        	      ++TmpPos5;
         	    }
        	  for (int j = i; j < this->NbrRow; j++)
        	    {
        	      TmpPos = Pos;
        	      TmpPos3 = j + TmpPos4;
        	      int k = i;
        	      TmpCoef[TmpPos5] = 0.0;
        	      for (; k < j; k++)
        		{
        		  TmpCoef[TmpPos5] += Q.Columns[k].Components[j] * this->OffDiagonalElements[TmpPos];
        		  TmpV[TmpPos2] +=  this->OffDiagonalElements[TmpPos++] * this->OffDiagonalElements[TmpPos3];
        		  TmpPos3 += Inc - k;
        		}
        	      TmpCoef[TmpPos5] += Q.Columns[j].Components[j] * this->OffDiagonalElements[TmpPos];
        	      TmpV[TmpPos2] +=  this->DiagonalElements[k] * this->OffDiagonalElements[TmpPos++];
        	      k++;
        	      TmpPos3++;
        	      for (; k < this->NbrRow; k++)
        		{
        		  TmpCoef[TmpPos5] += Q.Columns[k].Components[j] * this->OffDiagonalElements[TmpPos];
        		  TmpV[TmpPos2] +=  this->OffDiagonalElements[TmpPos++] * this->OffDiagonalElements[TmpPos3++];
        		}
        	      Coef += TmpV[TmpPos2] * this->OffDiagonalElements[Pos + TmpPos2];
        	      TmpPos2++;
        	      TmpPos5++;
        	    }
        	  TmpPos = Pos;
        	  Coef *= 0.5;
        	  for (int j = 0; j < ReducedNbrRow2; j++)
        	    {
        	      TmpV[j] -=  this->OffDiagonalElements[TmpPos++] * Coef;
        	    }

        	  // conjugate Hermitian matrix
        	  TmpPos2 = i * (this->NbrRow + this->Increment) - (i * (i + 1)) / 2;
        	  TmpPos = Pos + 1;
        	  TmpPos6 = Pos;
        	  for (int j = 1; j < i; j++)
        	    {
        	      TmpPos5 = TmpPos6;
        	      Coef = TmpCoef[j];
        	      for (int k = i; k < this->NbrRow; ++k)
        		{
        		  Q.Columns[k].Components[j] -=  this->OffDiagonalElements[TmpPos5] * Coef;
        		  ++TmpPos5;
        		}
        	    }
        	  for (int j = i; j < this->NbrRow; ++j)
        	    {
        	      TmpPos4 = j - i;
        	      TmpPos5 = TmpPos6;
        	      Coef = TmpCoef[j];
        	      for (int k = i; k < j; ++k)
        		{
        		  Q.Columns[k].Components[j] -=  this->OffDiagonalElements[TmpPos5++] * Coef;
        		}
        	      this->DiagonalElements[j] -=  this->OffDiagonalElements[Pos] * 2.0 * TmpV[TmpPos4];
        	      Q.Columns[j].Components[j] -=  this->OffDiagonalElements[TmpPos5++] * Coef;
        	      TmpPos3 = TmpPos;
        	      for (int k = j + 1; k < this->NbrRow; ++k)
        		{
        		  this->OffDiagonalElements[TmpPos2++] -=  (this->OffDiagonalElements[TmpPos3] * TmpV[TmpPos4]
        							    + this->OffDiagonalElements[Pos] *  TmpV[k - i]);
        		  Q.Columns[k].Components[j] -=  this->OffDiagonalElements[TmpPos5++] * Coef;
        		  TmpPos3++;
        		}
        	      TmpPos2 += this->Increment;
        	      TmpPos++;
        	      Pos++;
        	    }
        	}
              else
        	{
        	  Pos += (this->NbrRow - i);
        	}
              M.DiagonalElement(i) = this->DiagonalElements[i];
              Pos += this->Increment;*/
    }
//  M.UpperDiagonalElement(ReducedNbrRow - 1) = this->OffDiagonalElements[Pos - this->Increment];
    M.DiagonalElement(ReducedNbrRow) = this->DiagonalElements[ReducedNbrRow];
    delete[] TmpVRe;
    delete[] TmpVIm;
    return M;
}

// store hermitian matrix into a real symmetric matrix (real part as block diagonal element and imaginary part as block off-diagonal element )
//
// return value = real symmetric matrix associated to the hermitian matrix

RealSymmetricMatrix HermitianMatrix::ConvertToSymmetricMatrix()
{
    RealSymmetricMatrix TmpMatrix (this->NbrRow * 2, this->NbrColumn * 2);
    int PosReal = 0;
    int PosIm = 0;
    for (int i = 0; i < this->NbrRow; ++i)
    {
        TmpMatrix(i, i) = this->DiagonalElements[i];
        TmpMatrix(i + this->NbrRow, i + this->NbrRow) = this->DiagonalElements[i];
        for (int j = i + 1; j < this->NbrColumn; ++j)
        {
            TmpMatrix(i, j) = this->RealOffDiagonalElements[PosReal];
            TmpMatrix(i + this->NbrRow, j + this->NbrRow) = this->RealOffDiagonalElements[PosReal];
            TmpMatrix(i, j + this->NbrRow) = -this->ImaginaryOffDiagonalElements[PosIm];
            TmpMatrix(j, i + this->NbrRow) = this->ImaginaryOffDiagonalElements[PosIm];
            ++PosReal;
            ++PosIm;
        }
        PosReal += this->Increment;
        PosIm += this->Increment;
    }
    return TmpMatrix;
}


// Diagonalize an hermitian matrix (modifying current matrix)
//
// M = reference on real diagonal matrix where result has to be stored
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real tridiagonal symmetric matrix

RealDiagonalMatrix& HermitianMatrix::Diagonalize (RealDiagonalMatrix& M, double err, int maxIter)
{
#ifdef __LAPACKONLY__
    return this->LapackDiagonalize(M, err, maxIter);
#endif
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    RealSymmetricMatrix TmpMatrix1 (this->ConvertToSymmetricMatrix());
    RealDiagonalMatrix TmpMatrix2(2 * this->NbrRow);
    TmpMatrix1.Diagonalize(TmpMatrix2, err, maxIter);
    TmpMatrix2.SortMatrixUpOrder();
    for (int i = 0; i < M.GetNbrRow(); ++i)
        M[i] = TmpMatrix2[2 * i];
    return M;
}

// Diagonalize an hermitian matrix and evaluate transformation matrix (modifying current matrix)
//
// M = reference on real diagonal matrix where result has to be stored
// Q = matrix where transformation matrix has to be stored
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real tridiagonal symmetric matrix

RealDiagonalMatrix& HermitianMatrix::Diagonalize (RealDiagonalMatrix& M, ComplexMatrix& Q, double err, int maxIter)
{
#ifdef __LAPACKONLY__
    return this->LapackDiagonalize(M, Q, err, maxIter);
#endif
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    if (Q.GetNbrRow() != this->NbrRow)
        Q.Resize(this->NbrRow, this->NbrColumn);
    ComplexVector TmpVector(this->NbrRow);
    RealSymmetricMatrix TmpMatrix1 (this->ConvertToSymmetricMatrix());
    RealDiagonalMatrix TmpMatrix2(2 * this->NbrRow);
    RealMatrix TmpMatrix3(2 * this->NbrRow, 2 * this->NbrRow);
    TmpMatrix1.Diagonalize(TmpMatrix2, TmpMatrix3, err, maxIter);
    TmpMatrix2.SortMatrixUpOrder(TmpMatrix3);
    double LastEV=-1e300;
    int NewEVIndex=-1;
    for (int i = M.GetNbrRow()-1; i >= 0; --i)
    {
        M[i] = TmpMatrix2[2 * i];
        if (fabs(M[i]-LastEV)>1e-10)
        {
            LastEV=M[i];
            NewEVIndex=i;
            for (int j = 0; j < M.GetNbrRow(); ++j)
            {
                Q[i].Re(j) = TmpMatrix3[2 * i][j];
                Q[i].Im(j) = -TmpMatrix3[2 * i][j + M.GetNbrRow()];
            }
        }
        else
        {
            // Orthogonalize degenerate eigenvectors
            for (int j = 0; j < M.GetNbrRow(); ++j)
            {
                TmpVector.Re(j) = TmpMatrix3[2 * i][j];
                TmpVector.Im(j) = -TmpMatrix3[2 * i][j + M.GetNbrRow()];
            }
            for (int k=NewEVIndex; k>i; --k)
            {
                TmpVector -= (Q[k]*TmpVector) * Q[k];
            }
            TmpVector/=TmpVector.Norm();
            for (int j = 0; j < M.GetNbrRow(); ++j)
            {
                Q[i].Re(j) = TmpVector.Re(j);
                Q[i].Im(j) = TmpVector.Im(j);
            }
        }
    }

    return M;
}


#ifdef __LAPACK__

// Diagonalize a complex skew symmetric matrix using the LAPACK library (modifying current matrix)
//
// M = reference on real diagonal matrix of eigenvalues
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real matrix consisting of eigenvalues
RealDiagonalMatrix& HermitianMatrix::LapackDiagonalize (RealDiagonalMatrix& M, double err, int maxIter)
{
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    Complex Tmp;
    if (this->LapackWorkAreaDimension<this->NbrRow)
    {
        if (this->LapackWorkAreaDimension>0)
        {
            delete [] LapackMatrix;
            delete [] LapackWorkingArea;
            delete [] LapackRealWorkingArea;
            if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
            if (this->LapackWorkAreaForPartialDiag)
            {
                delete [] LapackIntWorkingArea;
                delete [] this->LapackFailedToConverge;
            }
        }
        this->LapackMatrix = new doublecomplex [((long) this->NbrRow) * ((long) (this->NbrRow+1)) / 2l];
        this->LapackEVMatrix = NULL;
        this->LapackWorkingArea = new doublecomplex [2*this->NbrRow-1];
        this->LapackRealWorkingArea = new double [3*this->NbrRow-2];
        this->LapackWorkAreaDimension=this->NbrRow;
        this->LapackWorkAreaForPartialDiag=false;
    }

    int Information = 0;
    const char* Jobz = "N";
    const char* UpperLower = "U";
    long TotalIndex = 0l;
    for (int j = 0; j < this->NbrRow; ++j)
    {
        for (int i = 0; i < j; ++i)
        {
            this->GetMatrixElement(i,j,Tmp);
            LapackMatrix[TotalIndex].r = Tmp.Re;
            LapackMatrix[TotalIndex].i = Tmp.Im;
            ++TotalIndex;
        }
        LapackMatrix[TotalIndex].r = this->DiagonalElements[j];
        LapackMatrix[TotalIndex].i = 0.0;
        ++TotalIndex;
    }
    FORTRAN_NAME(zhpev)(Jobz, UpperLower, &this->NbrRow, LapackMatrix, M.DiagonalElements, LapackEVMatrix, &this->NbrRow, LapackWorkingArea, LapackRealWorkingArea, &Information);
    return M;
}


// Diagonalize a complex skew symmetric matrix using the LAPACK library (modifying current matrix)
//
// M = reference on real diagonal matrix of eigenvalues
// nMin = index of lowest eigenvalue to be calculated (numbered in C-conventions, from 0,...,d-1)
// nMax = index of highest eigenvalue to be calculated (numbered in C-conventions, from 0,...,d-1)
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real matrix consisting of eigenvalues
RealDiagonalMatrix& HermitianMatrix::LapackPartialDiagonalize (RealDiagonalMatrix& M, int nMin, int nMax, double err, int maxIter)
{
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    Complex Tmp;
    if ((this->LapackWorkAreaDimension<this->NbrRow)||(this->LapackWorkAreaForPartialDiag==false))
    {
        if (this->LapackWorkAreaDimension>0)
        {
            delete [] LapackMatrix;
            delete [] LapackWorkingArea;
            delete [] LapackRealWorkingArea;
            if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
            if (this->LapackWorkAreaForPartialDiag)
            {
                delete [] this->LapackIntWorkingArea;
                delete [] this->LapackFailedToConverge;
            }
        }
        this->LapackMatrix = new doublecomplex [((long) this->NbrRow) * ((long) (this->NbrRow+1)) / 2l];
        this->LapackEVMatrix = NULL;
        this->LapackWorkingArea = new doublecomplex [2*this->NbrRow];
        this->LapackRealWorkingArea = new double [7*this->NbrRow];
        this->LapackIntWorkingArea = new int [5*this->NbrRow];
        this->LapackFailedToConverge = new int [this->NbrRow];
        this->LapackWorkAreaDimension=this->NbrRow;
        this->LapackWorkAreaForPartialDiag=true;
    }

    int Information = 0;
    const char* Jobz = "N";
    const char* Range = "I";
    const char* UpperLower = "U";
    ++nMin;
    ++nMax;
    if (nMin<1) nMin=1;
    if (nMax>this->NbrRow) nMax=this->NbrRow;
    int NbrFound=0;
    long TotalIndex = 0l;
    for (int j = 0; j < this->NbrRow; ++j)
    {
        for (int i = 0; i < j; ++i)
        {
            this->GetMatrixElement(i,j,Tmp);
            LapackMatrix[TotalIndex].r = Tmp.Re;
            LapackMatrix[TotalIndex].i = Tmp.Im;
            ++TotalIndex;
        }
        LapackMatrix[TotalIndex].r = this->DiagonalElements[j];
        LapackMatrix[TotalIndex].i = 0.0;
        ++TotalIndex;
    }
    FORTRAN_NAME(zhpevx)(Jobz, Range, UpperLower, &this->NbrRow, LapackMatrix, /*const double *lowerBoundVL*/ NULL, /*const double *upperBoundVU*/ NULL, &nMin, &nMax, &err, &NbrFound, M.DiagonalElements, LapackEVMatrix, &this->NbrRow, LapackWorkingArea, LapackRealWorkingArea, LapackIntWorkingArea, LapackFailedToConverge, &Information);
    if (Information<0)
    {
        cout << "Illegal argument no. "<<-Information<<" in call to zhpevx"<<endl;
        std::exit(1);
    }
    if (Information>0)
    {
        cout << Information << " eigenvalues failed to converge in call to zhpevx; their indices are: "<<this->LapackFailedToConverge[0];
        for (int i=1; i<Information; ++i)
            cout <<", "<<this->LapackFailedToConverge[i];
        cout <<endl;
    }

    return M;
}


// Diagonalize a complex skew symmetric matrix and evaluate transformation matrix using the LAPACK library (modifying current matrix)
//
// M = reference on real diagonal matrix of eigenvalues
// Q = matrix where transformation matrix has to be stored
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real matrix consisting of eigenvalues
RealDiagonalMatrix& HermitianMatrix::LapackDiagonalize (RealDiagonalMatrix& M, ComplexMatrix& Q, double err, int maxIter)
{
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    if (Q.GetNbrRow() != this->NbrRow)
        Q.Resize(this->NbrRow, this->NbrColumn);
    Complex Tmp;
    if (this->LapackWorkAreaDimension<this->NbrRow)
    {
        if (this->LapackWorkAreaDimension>0)
        {
            delete [] LapackMatrix;
            delete [] LapackWorkingArea;
            delete [] LapackRealWorkingArea;
            if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
            if (this->LapackWorkAreaForPartialDiag)
            {
                delete [] this->LapackIntWorkingArea;
                delete [] this->LapackFailedToConverge;
            }
        }
        this->LapackMatrix = new doublecomplex [((long) this->NbrRow) * ((long) (this->NbrRow+1)) / 2l];
        this->LapackEVMatrix = NULL;
        this->LapackWorkingArea = new doublecomplex [2*this->NbrRow-1];
        this->LapackRealWorkingArea = new double [3*this->NbrRow-2];
        this->LapackWorkAreaDimension=this->NbrRow;
        this->LapackWorkAreaForPartialDiag=false;
    }
    if (LapackEVMatrix==NULL)
        LapackEVMatrix = new doublecomplex[((long) this->NbrRow) * ((long) this->NbrRow)];
    int Information = 0;
    const char* Jobz = "V";
    const char* UpperLower = "U";
    int TotalIndex = 0;
    for (int j = 0; j < this->NbrRow; ++j)
    {
        for (int i = 0; i < j; ++i)
        {
            this->GetMatrixElement(i,j,Tmp);
            LapackMatrix[TotalIndex].r = Tmp.Re;
            LapackMatrix[TotalIndex].i = Tmp.Im;
            ++TotalIndex;
        }
        LapackMatrix[TotalIndex].r = this->DiagonalElements[j];
        LapackMatrix[TotalIndex].i = 0.0;
        ++TotalIndex;
    }
    FORTRAN_NAME(zhpev)(Jobz, UpperLower, &this->NbrRow, LapackMatrix, M.DiagonalElements, LapackEVMatrix, &this->NbrRow, LapackWorkingArea, LapackRealWorkingArea, &Information);

    TotalIndex=0;
    for (int i = 0; i < this->NbrRow; ++i)
        for (int j = 0; j < this->NbrRow; ++j)
        {
            Tmp.Re = LapackEVMatrix[TotalIndex].r;
            Tmp.Im = -LapackEVMatrix[TotalIndex].i;
            Q.SetMatrixElement(j, i, Tmp);
            ++TotalIndex;
        }
    return M;
}

// Diagonalize selected eigenvalues of a hermitian matrix and evaluate transformation matrix using the LAPACK library (modifying current matrix)
//
// M = reference on real diagonal matrix of eigenvalues
// Q = matrix where transformation matrix has to be stored
// nMin = index of lowest eigenvalue to be calculated (numbered in C-conventions, from 0,...,d-1)
// nMax = index of highest eigenvalue to be calculated (numbered in C-conventions, from 0,...,d-1)
// err = absolute error on matrix element
// maxIter = maximum number of iteration to fund an eigenvalue
// return value = reference on real matrix consisting of eigenvalues
RealDiagonalMatrix& HermitianMatrix::LapackPartialDiagonalize (RealDiagonalMatrix& M, ComplexMatrix& Q, int nMin, int nMax, double err, int maxIter)
{
    if (M.GetNbrRow() != this->NbrRow)
        M.Resize(this->NbrRow, this->NbrColumn);
    if (Q.GetNbrRow() != this->NbrRow)
        Q.Resize(this->NbrRow, nMax-nMin+1);
    Complex Tmp;
    if ((this->LapackWorkAreaDimension<this->NbrRow)||(this->LapackWorkAreaForPartialDiag==false))
    {
        if (this->LapackWorkAreaDimension>0)
        {
            delete [] LapackMatrix;
            delete [] LapackWorkingArea;
            delete [] LapackRealWorkingArea;
            if (LapackEVMatrix!=0) delete [] LapackEVMatrix;
            if (this->LapackWorkAreaForPartialDiag)
            {
                delete [] this->LapackIntWorkingArea;
                delete [] this->LapackFailedToConverge;
            }
        }
        this->LapackMatrix = new doublecomplex [((long) this->NbrRow) * ((long) (this->NbrRow+1)) / 2l];
        this->LapackEVMatrix = NULL;
        this->LapackWorkingArea = new doublecomplex [2*this->NbrRow];
        this->LapackRealWorkingArea = new double [7*this->NbrRow];
        this->LapackIntWorkingArea = new int [5*this->NbrRow];
        this->LapackFailedToConverge = new int [this->NbrRow];
        this->LapackWorkAreaDimension=this->NbrRow;
        this->LapackWorkAreaForPartialDiag=true;
    }
    if ((LapackEVMatrix!=NULL)&&(LapackEVsRequested<nMax-nMin+1))
    {
        delete [] LapackEVMatrix;
        LapackEVMatrix=NULL;
    }
    if (LapackEVMatrix==NULL)
    {
        LapackEVsRequested=nMax-nMin+1;
        LapackEVMatrix = new doublecomplex[((long) this->NbrRow) * ((long) this->LapackEVsRequested)];
    }
    int Information = 0;
    const char* Jobz = "V";
    const char* Range = "I";
    const char* UpperLower = "U";
    ++nMin;
    ++nMax;
    if (nMin<1) nMin=1;
    if (nMax>this->NbrRow) nMax=this->NbrRow;
    int NbrFound=0;
    long TotalIndex = 0l;
    for (int j = 0; j < this->NbrRow; ++j)
    {
        for (int i = 0; i < j; ++i)
        {
            this->GetMatrixElement(i,j,Tmp);
            LapackMatrix[TotalIndex].r = Tmp.Re;
            LapackMatrix[TotalIndex].i = Tmp.Im;
            ++TotalIndex;
        }
        LapackMatrix[TotalIndex].r = this->DiagonalElements[j];
        LapackMatrix[TotalIndex].i = 0.0;
        ++TotalIndex;
    }
    FORTRAN_NAME(zhpevx)(Jobz, Range, UpperLower, &this->NbrRow, LapackMatrix, /*const double *lowerBoundVL*/ NULL, /*const double *upperBoundVU*/ NULL, &nMin, &nMax, &err, &NbrFound, M.DiagonalElements, LapackEVMatrix, &this->NbrRow, LapackWorkingArea, LapackRealWorkingArea, LapackIntWorkingArea, LapackFailedToConverge, &Information);
    if (Information<0)
    {
        cout << "Illegal argument no. "<<-Information<<" in call to zhpevx"<<endl;
        std::exit(1);
    }

    if (Information>0)
    {
        cout << Information << " eigenvalues failed to converge in call to zhpevx; their indices are: "<<this->LapackFailedToConverge[0];
        for (int i=1; i<Information; ++i)
            cout <<", "<<this->LapackFailedToConverge[i];
        cout <<endl;
    }


    TotalIndex=0;
    for (int i = 0; i < nMax-nMin+1; ++i)
        for (int j = 0; j < this->NbrRow; ++j)
        {
            Tmp.Re = LapackEVMatrix[TotalIndex].r;
            Tmp.Im = -LapackEVMatrix[TotalIndex].i;
            Q.SetMatrixElement(j, i, Tmp); // second index is the column index (numbering the state)
            ++TotalIndex;
        }


    return M;
}

#endif



