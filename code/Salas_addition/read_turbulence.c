/////////////////////SALAS EDITION OF TURBULENCE --- LAST MOD, MARCH 1,  2017/////////////////////
#include "../allvars.h"
#include "../proto.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <gsl/gsl_rng.h>

///FUNCTION TO READ TURBULENCE FILES and convert the 4d array into 1d array
void read_turbulence(int fileturb, const char *normal_tur_file)
{
    ///Read normalized turbulence grid from file//
    FILE *turb_in = fopen(normal_tur_file, "r");
    
    printf("\n----Reading normalized turbulence file: %s ----\n", normal_tur_file);
   
    int ii, jj, kk; double vxt, vyt,vzt; 
    for (ii=0; ii<grid_size; ii++)
    {
        for (jj=0; jj<grid_size; jj++)
        {
            for (kk=0; kk<grid_size; kk++)
            {
                fscanf(turb_in, "%lf %lf %lf\n", &vxt, &vyt, &vzt) ;
                ///CONVERTING A 2D ARRAY INTO A 1D ARRAY (SINCE BROADCASTING 1D-ARRAYS IS EASIER)
                int index0 = 0 + ii * dim_turb + jj * dim_turb * grid_size + kk * dim_turb * grid_size * grid_size;
                int index1 = 1 + ii * dim_turb + jj * dim_turb * grid_size + kk * dim_turb * grid_size * grid_size;
                int index2 = 2 + ii * dim_turb + jj * dim_turb * grid_size + kk * dim_turb * grid_size * grid_size;
                if (fileturb == 0)
                {
                	turb_data_array0[index0] = vxt;
                	turb_data_array0[index1] = vyt;
                	turb_data_array0[index2] = vzt;
                }
                if (fileturb == 1)
                {
                	turb_data_array1[index0] = vxt;
                	turb_data_array1[index1] = vyt;
                	turb_data_array1[index2] = vzt;
                }
                if (fileturb == 2)
                {
                	turb_data_array2[index0] = vxt;
                	turb_data_array2[index1] = vyt;
                	turb_data_array2[index2] = vzt;
                }
                if (fileturb == 3)
                {
                	turb_data_array3[index0] = vxt;
                	turb_data_array3[index1] = vyt;
                	turb_data_array3[index2] = vzt;
                }
                if (fileturb == 4)
                {
                	turb_data_array4[index0] = vxt;
                	turb_data_array4[index1] = vyt;
                	turb_data_array4[index2] = vzt;
                }
                if (fileturb == 5)
                {
                	turb_data_array5[index0] = vxt;
                	turb_data_array5[index1] = vyt;
                	turb_data_array5[index2] = vzt;
                }
                if (fileturb == 6)
                {
                	turb_data_array6[index0] = vxt;
                	turb_data_array6[index1] = vyt;
                	turb_data_array6[index2] = vzt;
                }
                if (fileturb == 7)
                {
                	turb_data_array7[index0] = vxt;
                	turb_data_array7[index1] = vyt;
                	turb_data_array7[index2] = vzt;
                }
                if (fileturb == 8)
                {
                	turb_data_array8[index0] = vxt;
                	turb_data_array8[index1] = vyt;
                	turb_data_array8[index2] = vzt;
                }
                if (fileturb == 9)
                {
                	turb_data_array9[index0] = vxt;
                	turb_data_array9[index1] = vyt;
                	turb_data_array9[index2] = vzt;
                }
            }
        }
    }
    
    
    fclose(turb_in);
}


///Create holder arrays for easier reading///
///This function creates the physical grid in which the turbulence grid is embedded
void create_phys_grid() 
{
//    #if !defined(GRAVITY_GALAXY) && !defined(GRAVITY_LAUNHARDT)   /// for one single cloud
//        printf("*****Creating holder arrays for easy access*****\n");
//        int pp;
//        cloudgrid.grid[0] = -(cloud_radius); /// values from -3.9375pc to 4 pc, separated by delta =(8/128)pc
//       ////This grid is defined so that there's a grid value defined at (0,0,0)
//        for (pp=1;pp<grid_size;pp++) ///grid_size = 128
//        {
//            cloudgrid.grid[pp] = cloudgrid.grid[pp-1] + (double)delta;
//        }
//
//        cloudgrid.idum = (long)seed_ran3; //// random number seed
//        printf("Grid[0] = %f, Grid[last] = %f\n", cloudgrid.grid[0], cloudgrid.grid[grid_size-1]);
//        printf("****Holder arrays for easy access-DONE*****\n");
//    #else
    
    
    ////This part creates a giant grid, from -galactic_radius to galactic_radius , with dela = delta.
	////This supergrid is divided into (number_of_grids)^3 minigrids of size (2*cloud_radius )^3
	////The supergrid is set so that there's a grid element defined at(0,0,0)
    printf("\nNumber of boxes = %d\n ", number_of_grids );
	printf("Size of each box = %10.5f \n", 2.0*cloud_radius) ;
	printf("Boxes from -%10.5f to %10.5f \n ", galactic_radius, galactic_radius);
	printf("Resolution = %10.5f ==> %10.5f/128 \n\n ", delta, 2.0*cloud_radius) ; 
	printf("*****Creating holder arrays for easy access*****\n");
        int pp;
        galactic_grid[0].grid[0] = -(galactic_radius); //galactic_radius = 256, delta = (8/128)
        galactic_grid[0].idum = (long)seed_ran3; ///random seed
        for (pp=1;pp<grid_size;pp++) ///create first minigrid, from -256 to -256 + 8
        {
            galactic_grid[0].grid[pp] = galactic_grid[0].grid[pp-1] + (double)delta;
        }
        int j;
        for (j=1;j<number_of_grids;j++) ///do all other number_of_grids-1 grids
        {
            galactic_grid[j].grid[0] = galactic_grid[j-1].grid[grid_size-1];
            galactic_grid[j].idum = (long)seed_ran3;
            for (pp=1;pp<grid_size;pp++)
            {
                galactic_grid[j].grid[pp] = galactic_grid[j].grid[pp-1] + (double)delta;
            }
        }
       
	int g;
	for (g=0;g<number_of_grids;g++)
	  {
	    printf("Grid%d = %f, Grid%d_last = %f\n", g, galactic_grid[g].grid[0], g, galactic_grid[g].grid[grid_size-1]);
	  }
        printf("****Holder arrays for easy access-DONE*****\n");
  //  #endif
    
}

void grid_to_file_assignment() ///SALAS MOD MARCH 21, 2019
{
    int i; //long nuuu =0;
    double myran1=0;
    for (i=0; i<number_of_grids3d; i++)
    {
        ///nuuu = ((long)seed_ran3 / 1000) - i ;
        ////ran3( &nuuu  );
        myran1 = gsl_rng_uniform(random_generator) ;  ///random number from 0 to 1
        file_of_each_grid[i] = which_file( Nturbfiles, myran1);
    }
    printf("First five files = %d, %d, %d, %d, %d\n", file_of_each_grid[0],
           file_of_each_grid[1], file_of_each_grid[2], file_of_each_grid[3], file_of_each_grid[4] );
}
