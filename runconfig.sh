./configure --with-mpi-cxx="mpic++" --with-mpi-libs="-lmpi" --with-mpi-incdir="/opt/local/include/openmpi-gcc5" --with-mpi-libdir="/opt/local/lib/openmpi-gcc5" \
CXXFLAGS="-O3 -Wall -I/opt/local/include -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7" \
LDFLAGS="-L/opt/local/lib -L/opt/local/lib/openmpi-gcc5 -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib -lboost_python -lpython2.7 -flat_namespace -force_flat_namespace" \
--enable-lapack --with-lapack-libs="-llapack" --with-blas-libs="-lopenblas-r1" --enable-scalapack --with-scalapack-libs="-lscalapack" \
--enable-gsl --with-gsl-libs="-lgsl -lgslcblas" --enable-gmp --with-gmp-libs="-lgmp -lgmpxx" --enable-fqhe --enable-fti
