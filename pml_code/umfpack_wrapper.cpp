//
//  umfpack_wrapper.cpp
//  pml_code
//
//  Created by Christophe Desceliers on 13/02/2017.
//  Copyright © 2017 Christophe Desceliers. All rights reserved.
//

#include "umfpack_wrapper.hpp"
#include "umfpack.h"
#include <iostream>

using namespace std;
using namespace Eigen;
using namespace FEM;

umfpack_wrapper::~umfpack_wrapper()
{
    umfpack_zl_free_numeric ( &Numeric );
    
    delete[] Values_x;
    delete[] Values_y;
    delete[] OuterStart;
    delete[] InnerIndices;
}



umfpack_wrapper::umfpack_wrapper(SP_mat_complex &MAT)
{
    int counter_outer = 0;
    int counter = 0;
    n_cols = MAT.cols() ;
    n_rows = MAT.rows() ;
    n_nz = MAT.nonZeros() ;
    
    Values_x   = new double[n_nz];
    Values_y   = new double[n_nz];
    OuterStart   = new long[n_cols+1];
    InnerIndices = new long[n_nz];
    
    for (int k=0; k<MAT.outerSize(); ++k)
    {
        OuterStart[counter_outer]=counter;
        counter_outer++ ;
        for (SP_mat_complex::InnerIterator it(MAT,k); it; ++it)
        {
            Values_x[counter] = real(it.value());
            Values_y[counter] = imag(it.value());
            InnerIndices[counter] = it.row();
            counter++ ;
        }
    }
    OuterStart[counter_outer]=counter;
    
    void *Symbolic;
    
    long status;
    
    status = umfpack_zl_symbolic ( n_rows, n_cols, OuterStart,
                                  InnerIndices,
                                  Values_x, Values_y, &Symbolic, NULL, NULL );
    
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_zl_symbolic" << endl;
        std::cout << "Status code is " << status << endl;
    }
    status = umfpack_zl_numeric (OuterStart, InnerIndices, Values_x, Values_y, Symbolic, &Numeric, NULL, NULL );
    
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_zl_numeric" << endl;
        std::cout << "Status code is " << status << endl;
    }
    
    umfpack_zl_free_symbolic ( &Symbolic );
}

umfpack_wrapper::umfpack_wrapper(SP_mat &MAT)
{
    int counter_outer = 0;
    int counter = 0;
    n_cols = MAT.cols() ;
    n_rows = MAT.rows() ;
    n_nz = MAT.nonZeros() ;
    
    Values_x   = new double[n_nz];
    Values_y   = new double[1];
    OuterStart   = new long[n_cols+1];
    InnerIndices = new long[n_nz];
    
    for (int k=0; k<MAT.outerSize(); ++k)
    {
        OuterStart[counter_outer]=counter;
        counter_outer++ ;
        for (SP_mat::InnerIterator it(MAT,k); it; ++it)
        {
            Values_x[counter] = real(it.value());
            InnerIndices[counter] = it.row();
            counter++ ;
        }
    }
    OuterStart[counter_outer]=counter;
    
    void *Symbolic;
    
    long status;
    
    status = umfpack_dl_symbolic ( n_rows, n_cols, OuterStart,
                                  InnerIndices,
                                  Values_x, &Symbolic, NULL, NULL );
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_dl_symbolic" << endl;
        std::cout << "Status code is " << status << endl;
    }
    
    status = umfpack_dl_numeric (OuterStart, InnerIndices, Values_x, Symbolic, &Numeric, NULL, NULL );
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_dl_numeric" << endl;
        std::cout << "Status code is " << status << endl;
    }
    
    umfpack_dl_free_symbolic ( &Symbolic );
    
}


Eigen::VectorXcd umfpack_wrapper::solve(Eigen::VectorXcd &F)
{
    double * b_x   = new double[n_rows];
    double * b_y   = new double[n_rows];
    
    for (int k=0; k<n_rows; ++k)
    {
        b_x[k] = real(F(k));
        b_y[k] = imag(F(k));
    }
    double * x = new double[n_rows];
    double * y = new double[n_rows];
    long status;
    
    status = umfpack_zl_solve ( UMFPACK_A, OuterStart, InnerIndices, Values_x, Values_y,
                               x, y, b_x, b_y, Numeric, NULL, NULL );
    
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_zl_solve" << endl;
        std::cout << "Status code is " << status << endl;
    }
    
    VectorXcd U =  VectorXcd::Zero(n_rows,1);
    
    for (int k=0; k<n_rows; ++k)
        U(k) = x[k] + dcomplex(0,1)*y[k];
    
    delete[] b_x;
    delete[] b_y;
    delete[] x;
    delete[] y;
    
    return U;
}

Eigen::VectorXd umfpack_wrapper::solve(Eigen::VectorXd &F)
{
    double * b_x   = new double[n_rows];
    double * b_y   = new double[1];
    
    for (int k=0; k<n_rows; ++k)
        b_x[k] = real(F(k));
    
    double * x = new double[n_rows];
    double * y = new double[1];
    long status;
    
    status = umfpack_dl_solve ( UMFPACK_A, OuterStart, InnerIndices, Values_x,
                                x, b_x, Numeric, NULL, NULL );
    if (status != UMFPACK_OK)
    {
        std::cout << "Error with umfpack_zl_solve" << endl;
        std::cout << "Status code is " << status << endl;
    }
    
    VectorXd U =  VectorXd::Zero(n_rows,1);
    
    for (int k=0; k<n_rows; ++k)
        U(k) = x[k];
    
    delete[] b_x;
    delete[] b_y;
    delete[] x;
    delete[] y;
    
    return U;
}

