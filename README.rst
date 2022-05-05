DiagHam_Nimit
=============

DiagHam version used by Nimit for the FCI stability project.

- DiagHam wiki: https://nick-ux.org/diagham/index.php/Main_Page
- DiagHam website: http://www.phys.ens.fr/~regnault/diagham/

Getting started with DiagHam
----------------------------

These instructions are in addition to those listed on the wiki.

1) Intel libraries need to be installed for optimal performance. This is non-free software but can be downloaded for free using an academic license. At the time of writing, you need the Intel oneAPI Base Toolkit for the C/C++ compiler and MKL library and the Intel oneAPI HPC Toolkit for the Fortran compiler and MPI library. This academic license will time out after one year. It is not renewable and will need to be applied for and installed again every year. You just need an academic email to be approved but it can take up to 2 business days. After the installation, you can remove the installer if desired, which is in e.g. /tmp/root/ or ~/Downloads/. You can also add the following two lines to your ~/.bashrc: source /opt/intel/oneapi/compiler/latest/env/vars.sh; source /opt/intel/oneapi/mkl/latest/env/vars.sh. This saves some time when starting a shell, compared to sourcing the entire /opt/intel/oneapi/setvars.sh.

2. The configure command to automatically use the latest instruction set (-xHost) is:

../configure --enable-fqhe --enable-fti --enable-lapack --enable-gmp --enable-lapack-only --with-lapack-libs="" --with-blas-libs="-mkl" CC=icc CXX=icpc --enable-debug CFLAGS="-O3 -xHOST" CXXFLAGS="-O3 -xHOST"

...or for using both AVX and AVX2 instruction sets...

../configure --enable-fqhe --enable-fti --enable-lapack --enable-gmp --enable-lapack-only --with-lapack-libs="" --with-blas-libs="-mkl" CC=icc CXX=icpc --enable-debug CFLAGS="-O3 -xAVX -axCORE-AVX2" CXXFLAGS="-O3 -xAVX -axCORE-AVX2"

NB: BLAS will always be called when LAPACK is called, and it contains LAPACK, so no need to duplicate mkl flags. Since MKL is provided by both BLAS and LAPACK, it’s sufficient to give one or the other – but both options are required so one can also cope with separate libraries.

A guide to Intel compiler flags can be found here: https://www.bu.edu/tech/support/research/software-and-programming/programming/compilers/intel-compiler-flags/

Example Commands
----------------

- counting states in the Hilbert-space:

``DiagHam/build/FTI/src/Programs/FTI/FTIGetDimension -p 12 -x 4 -y 6 —bosons``

- calculating spectrum for the Hofstadter model with interactions:

``DiagHam/build/FTI/src/Programs/FCI/FCIHofstadterModel -p 10 -x 4 -y 5 --boson -X 3 -Y 3 -q 1 -m 10000 -S --processors 10 -n 1 --lanczos-precision 1e-10 —eigenstate``

- same with some more useful options:

``DiagHam/build/FTI/src/Programs/FCI/FCIHofstadterModel -p 10 -x 4 -y 5 --boson -X 3 -Y 3 -q 1 -m 10000 -S --processors 10 -n 2 --lanczos-precision 1e-10 --eigenstate  --auto-addprojector --only-kx 0 --only-ky 0 --fast-disk``

- overlaps between vectors:

``DiagHam/build/src/Programs/GenericOverlap vector1.vec vector2.vec``

- David mostly used a Python script to run DiagHam in batches. You can find this script in the Dropbox folder for the project: quartic/June 2018/Many-body code/batch_diagham.py. This script took CSV files with parameter lists as input, and you can find these batch files in the folders corresponding to the individual batches. A typical call from the script would be something like:

``DiagHam/build/FTI/src/Programs/FCI/FCIHofstadterModel --flat-band -n 2 -p 8 -x 4 -y 6 -X 3 -Y 2 --t2 -0.25``

This leaves out some flags relating to memory usage, numerical precision, etc.

- plot scripts are found in ``DiagHam_Nimit/scripts_bart/``

-FindLatticeGap.pl
Find the many-body gap from a .dat output file containing a many-body spectrum. Needs the expected ground state degeneracy as an input via -d

-PlotHofstadterSpectrum.pl
Generates a plot from a given spectrum file, using gnuplot. The -s flag is used to complete the spectrum by adding symmetry related momentum sectors to the bare data in the input file.

References
----------

`[Bauer2022] <https://arxiv.org/abs/2110.09565>`__ "Fractional Chern insulators with a non-Landau level continuum limit", by David Bauer et al., PRB **105**, 045144 (2022).
