///LAST MOD .. ,MAY 30 2017

////x0 - point below x
////x1 - poin above x
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../allvars.h"
#include "../proto.h"



/////This function finds the cell index on a grid that corresponds to a given 1d position
int cell_1d(double coord)
{
  double zin = (fabs( (galactic_grid[0].grid[0] - coord) / (2.0*cloud_radius) ) ) ;
    int result = (int) zin;
    return result ;
}

////This function determines the index, in 1D, of a grid velocity element.
///For example suppose x = 2.55. What is the closest (lower) value on the grid?
///Answer = 2.5. What is the index? Answer = 103.
///The upper index is 104, with value 2.5625
///I need upper and lower values for trilinear interpolation (See trilinear_box function below)
int ind_below_box(double coord, int *true_cell)
{
    double delta2 = (double) delta;
    double zin=0; int result=0;
    int temp_cell = 0;
    #if !defined(GRAVITY_GALAXY) && !defined(GRAVITY_LAUNHARDT)
    zin = (fabs( (cloudgrid.grid[0] - coord) / delta2) ) ;
    #else
    temp_cell = cell_1d(coord);
    if ( (temp_cell >= number_of_grids) || (temp_cell < 0) )
        { printf("My_error, cell out of ranged %d\n", temp_cell);
    	  exit(0);
        }
	if (coord > galactic_grid[temp_cell].grid[grid_size-1])
      { 
      	temp_cell++;
        if (temp_cell >= number_of_grids)
        {
            zin = -1;
        }
        else
        {
        	zin = (fabs( (galactic_grid[temp_cell].grid[0] - coord) / delta2) ) ;
        }
       }
	else 
	  {
	    zin = (fabs( (galactic_grid[temp_cell].grid[0] - coord) / delta2) ) ;
	  }
   
    #endif
    
    *true_cell = temp_cell;
    result = (int) zin;
    return result ;
}

///This function determines that if a particle is outside the turbulent grid, 
///no turbulence is added to that particle
int outside_box(double x, double y, double z)
{
    double limit_low=0; double limit_high=0;
    int c=-1;
    
    #if !defined(GRAVITY_GALAXY) && !defined(GRAVITY_LAUNHARDT)
    limit_low = cloudgrid.grid[0];
    limit_high = cloudgrid.grid[grid_size-1];    
    #else
    limit_low = galactic_grid[0].grid[0];
    limit_high = galactic_grid[number_of_grids-1].grid[grid_size-1];
   // printf("limits= %f, %f\n", limit_low, limit_high);
    #endif

    if ( (x <= limit_low) || x >= (limit_high) )
    {
        c = 0;
    }
    else if ( (y <= limit_low) || (y >= limit_high) )
    {
        c = 0;
    }
    else if ( (z <= limit_low) || (z >= limit_high) )
    {
        c = 0;
    }
    else ///inside box
    {
        c = 1;
    }
    
    return c;
}

///This function determines which turbulent grid file is gonna be used to interpolate turbulence
int which_file(int number_of_files, double r) ///RETURNS A RANDOM NUMBER FROM 0 TO NUMBER_OF_FILES
{
    int result;
    int h = (int)(r * number_of_files);
    
    if (number_of_files == 1)
    {
        result = 1;
    }
    if (h == number_of_files)
    {
        result = (number_of_files-1) ;
    }
    else
    {
        result = h;
    }
    
    return result;
}

////This function takes a turbulent grid and a particle inside that grid, and interpolates
////the turbulent velocity. See https://en.wikipedia.org/wiki/Trilinear_interpolation
double trilinear_box(double x, double y, double z, int v_dim, int write) ////v_dim is 0,1 or 2 for x, y ,z
{
    int which_cell_x = 0; /// a cell here is actually the grid
    int which_cell_y = 0;
    int which_cell_z = 0;
    
    double delta2 = (double) delta; double c=0.0;
    ///indices
    int index_x0 = ind_below_box( x , &which_cell_x );
    int index_x1 = index_x0 + 1;
    int index_y0 = ind_below_box( y , &which_cell_y );
    int index_y1 = index_y0 + 1;
    int index_z0 = ind_below_box( z , &which_cell_z );
    int index_z1 = index_z0 + 1;
    
    ///printf("%d, %d, %d\n", index_x0, index_y0, index_z0);
    
    
    ////Some flags to see if there's any errors
    if ( (index_x0 > grid_size-1) || (index_y0 > grid_size-1) || (index_z0 > grid_size-1) )
    {
        printf("My_error, check here. Index bigger than grid_size---. x = %f, y = %f, z = %f\n", x,y,z);
        c = 0.0;
        exit(0);
    }
    else if ( (index_x0 == -1) || (index_y0 == -1) || (index_z0 == -1) )
      {
        printf("My_error, check here. Particle out of bounds-- no turb---\n");
        c = 0.0;
        
      }
    else if ( (index_x0 < 0) || (index_y0 < 0) || (index_z0 < 0) )
    {
        printf("My_error, check here. Negative index\n");
        c = 0.0;
        exit(0);
    }
    /*
    else if (x == 0.0 || y == 0.0 || z == 0.0)
    {
        c = 0.0;
        printf("My_error, Particles in the plane, %f, %f, %f\n", x,y,z);
    }
     */
    else /// if no errors
    {
        ///values
        double x0 =0; double y0 = 0; double z0=0;
        /*
        #ifndef GRAVITY_GALAXY
       
            x0 = cloudgrid.grid[index_x0];
            y0 = cloudgrid.grid[index_y0];
            z0 = cloudgrid.grid[index_z0];
        #else
         */
        	if (    (which_cell_x > number_of_grids) || (which_cell_x < 0)
                ||  (which_cell_y > number_of_grids) || (which_cell_y < 0)
                ||  (which_cell_z > number_of_grids) || (which_cell_z < 0 ) ) /// cell is the grid
        	{
                printf("My_error, cell out of range %dd, %d, %d\n", which_cell_x, which_cell_y, which_cell_z);
        		exit(0);
        	}
	     
            x0 = galactic_grid[which_cell_x].grid[index_x0];
            y0 = galactic_grid[which_cell_y].grid[index_y0];
            z0 = galactic_grid[which_cell_z].grid[index_z0];
        
        int file = file_of_each_grid[which_cell_x*number_of_grids*number_of_grids +  which_cell_y*number_of_grids + which_cell_z] ;
    
        if (write == 1)
        {
            printf("Calculated x0, y0, z0 = %f, %f, %f\n", x0, y0, z0);
            printf("Real x, y, z = %f, %f, %f\n", x, y, z);
        }
       // #endif
       
        // this is wrong
        // int index0 = 0 + ii * dim + jj * dim * grid_size + kk * dim * grid_size * grid_size;
        // int index1 = 1 + ii * dim + jj * dim * grid_size + kk * dim * grid_size * grid_size;
        // int index2 = 2 + ii * dim + jj * dim * grid_size + kk * dim * grid_size * grid_size;
        
        double pt_x0_y0_z0=0;
        
        double pt_x1_y0_z0=0;
        double pt_x0_y1_z0=0;
        double pt_x0_y0_z1=0;
        
        double pt_x1_y1_z0=0;
        double pt_x1_y0_z1=0;
        double pt_x0_y1_z1=0;
        double pt_x1_y1_z1=0;

	int dim = dim_turb ;
        
        if (file == 0)
        {
         pt_x0_y0_z0 = turb_data_array0[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
       
         pt_x1_y0_z0 = turb_data_array0[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
         pt_x0_y1_z0 = turb_data_array0[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
         pt_x0_y0_z1 = turb_data_array0[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        
         pt_x1_y1_z0 = turb_data_array0[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
         pt_x1_y0_z1 = turb_data_array0[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
         pt_x0_y1_z1 = turb_data_array0[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
         pt_x1_y1_z1 = turb_data_array0[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 1)
        {
             pt_x0_y0_z0 = turb_data_array1[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array1[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array1[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array1[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array1[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array1[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array1[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array1[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 2)
        {
             pt_x0_y0_z0 = turb_data_array2[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array2[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array2[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array2[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array2[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array2[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array2[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array2[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 3)
        {
             pt_x0_y0_z0 = turb_data_array3[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array3[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array3[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array3[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array3[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array3[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array3[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array3[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 4)
        {
             pt_x0_y0_z0 = turb_data_array4[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array4[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array4[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array4[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array4[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array4[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array4[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array4[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 5)
        {
             pt_x0_y0_z0 = turb_data_array5[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array5[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array5[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array5[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array5[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array5[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array5[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array5[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 6)
        {
             pt_x0_y0_z0 = turb_data_array6[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array6[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array6[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array6[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array6[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array6[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array6[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array6[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 7)
        {
             pt_x0_y0_z0 = turb_data_array7[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array7[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array7[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array7[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array7[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array7[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array7[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array7[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 8)
        {
             pt_x0_y0_z0 = turb_data_array8[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array8[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array8[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array8[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array8[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array8[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array8[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array8[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        if (file == 9)
        {
             pt_x0_y0_z0 = turb_data_array9[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
            
             pt_x1_y0_z0 = turb_data_array9[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y1_z0 = turb_data_array9[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x0_y0_z1 = turb_data_array9[v_dim + index_x0 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
            
             pt_x1_y1_z0 = turb_data_array9[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z0 * dim * grid_size * grid_size];
             pt_x1_y0_z1 = turb_data_array9[v_dim + index_x1 * dim + index_y0 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x0_y1_z1 = turb_data_array9[v_dim + index_x0 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
             pt_x1_y1_z1 = turb_data_array9[v_dim + index_x1 * dim + index_y1 * dim * grid_size + index_z1 * dim * grid_size * grid_size];
        }
        
        
        
        ////trilinear algorithm, from https://en.wikipedia.org/wiki/Trilinear_interpolation
        double xd = (x - x0)/ delta2;
        double yd = (y - y0)/ delta2;
        double zd = (z - z0)/ delta2;
        /*
         if (x < x0 || y < y0 || z < z0)
         {
         printf("Error, check here\n");
         printf("x=%f, x0=%f, index = %d, index1 = %d\n", x, x0, index_x0, index_x1);
         printf("y=%f, y0=%f, index = %d, index1 = %d\n", y, y0, index_y0, index_y1);
         printf("z=%f, z0=%f, index = %d, index1 = %d\n", z, z0, index_z0, index_z1);
         exit(0);
         }
         */
        double one_minus_xd = (1.0 - xd);
        
        double c00 = (pt_x0_y0_z0 * one_minus_xd) + (pt_x1_y0_z0 * xd);
        double c10 = (pt_x0_y1_z0 * one_minus_xd) + (pt_x1_y1_z0 * xd);
        double c01 = (pt_x0_y0_z1 * one_minus_xd) + (pt_x1_y0_z1 * xd);
        double c11 = (pt_x0_y1_z1 * one_minus_xd) + (pt_x1_y1_z1 * xd);
        
        double one_minus_yd = 1.0 - yd;
        
        double c0 = (c00*one_minus_yd) + (c10*yd);
        double c1 = (c01*one_minus_yd) + (c11*yd);
        
        c = (c0*(1.0-zd)) + (c1*zd);
    }
    return c;
}

///Solves the quadratic equation, returns the bigger solution
double solve_quadratic(double a, double b, double c)
{
    if (a==0)
    {
        printf("Caution, a = %e", a);
        exit(0);
    }
    if (b==0)
    {
        printf("Caution, b = %e", b);
        exit(0);
    }
    
    double sol=0;
    double small = 1.0e-6;
    double fraction = fabs((4.0*a*c)/(b*b));
    
    if (fraction <= small)///if c is small, no turbulence
    {
        if (ThisTask == 0)
        {
            printf("Negligible turbulence. A=0, since |4ac/b^2|= %e is small\n",fraction);
        }
        sol = 0.0;
    }
    
    double x1 = ( (-1.0*b) + sqrt( (b*b) - (4.0*a*c) ) ) / (2.0*a) ;
    double x2 = ( (-1.0*b) - sqrt( (b*b) - (4.0*a*c) ) ) / (2.0*a) ;
  
    if (fraction > small)
    {
        if (ThisTask == 0)
        {
            printf("Turbulence added. |4ac/b^2|= %e\n",fraction);
        }
        if ( (x1 == 0.0) && (x2 == 0.0) )
        {
            if (ThisTask == 0)
            {
                printf("My_error. Both results are zero! %e", sol);
            }
            exit(0);
        }
        if ( (x1 == 0.0) || (x2 == 0.0) )
        {
            if (ThisTask == 0)
            {
                printf("Caution. One result is zero!\n");
            }
            if (x1 >= x2)
            {
                sol = x1;
            }
            if (x2 > x1)
            {
                sol = x2;
            }

        }
        else
        {
            if (x1 >= x2)
            {
                sol = x1; //// original x1
            }
            if (x2 > x1)
            {
                sol = x2;  //// original x2
            }
           
        }
    }
     return sol;
}

/////////////SALAS MODIFIED June 2, 2017////////////////////////
///////////////////DRIVING TURBULENCE/////////////////////
#ifdef DRIVING
void add_turbulence()
{ 
    //double myran1=0;
   // #ifdef GRAVITY_GALAXY
   //     myran1 = ran3(&galactic_grid[0].idum);
  //  #else
   //     myran1 = ran3(&cloudgrid.idum);
   // #endif
    
	// int whatfile = which_file( Nturbfiles, myran1);
    
    if (ThisTask == 0)
    {  
        printf("DriveFlag should be 0 --> %d\n", DriveFlag);
        printf("Old turb should be 0: %e, %e, %e\n", P[10].TURB_VEL[0],
        									P[10].TURB_VEL[1], P[10].TURB_VEL[2]);
        printf("\tFirst loop\n"); //. File %d\n", whatfile);
    }
   
    int count_quad; 
    double local_b=0; double local_a=0; double c=0;// double local_epot_gc = 0;
    
   // double local_vx =0; double local_vy = 0; double local_vz=0;
    //double global_vx =0; double global_vy=0; double global_vz = 0;
    
    double scaling = 0.0;
    double scaling_again = 0.0;
    
    if ( All.DensityDepenTurb == 0)
    {
        scaling = 1.0;
        scaling_again = 1.0;
    }
    
    if ( (All.DensityDepenTurb == 0) && (ThisTask == 0)  )
    {
        printf("scaling should be equal to 1 = %e\n", scaling);
        printf("scaling_again should be equal to 1 = %e\n", scaling_again);
    }
    
    for (count_quad=0; count_quad<NumPart; count_quad++)
    {
        //if(P[count_quad].Ti_endstep == All.Ti_Current)
      //  {
      // int whatfile = which_file( Nturbfiles, myran1);
        double tempx =  P[count_quad].Pos[0] ;
        double tempy =  P[count_quad].Pos[1] ;
        double tempz =  P[count_quad].Pos[2] ;
        
        int is_it_out = outside_box(tempx, tempy, tempz);
        
        if (is_it_out == -1)
        {	printf("My_error...is it out?\n");
            exit(0);
        }
        if (is_it_out == 1)
        {
            P[count_quad].TURB_VEL[0] = trilinear_box(tempx, tempy, tempz, 0, 0) ;
            P[count_quad].TURB_VEL[1] = trilinear_box(tempx, tempy, tempz, 1, 0) ;
            P[count_quad].TURB_VEL[2] = trilinear_box(tempx, tempy, tempz, 2, 0) ;
        }
    
        
        if ( All.DensityDepenTurb == 1 )
        {
	    	//if (SphP[i].Density > targetrho_code)
            scaling = sqrt(All.G * SphP[count_quad].Density) ;//All.UnitLength_in_cm / All.UnitVelocity_in_cm_per_s;
            //  else
            //scaling =1.0;
        }
        
        local_a += ( (scaling*scaling) * (     pow( P[count_quad].TURB_VEL[0], 2.0) +
                                               pow( P[count_quad].TURB_VEL[1], 2.0) +
                                               pow( P[count_quad].TURB_VEL[2], 2.0) ) );
        
        local_b += ( 2.0*scaling * (      (P[count_quad].Vel[0]*P[count_quad].TURB_VEL[0]) +
                                          (P[count_quad].Vel[1]*P[count_quad].TURB_VEL[1]) +
                                          (P[count_quad].Vel[2]*P[count_quad].TURB_VEL[2])  )  ) ;
           
    }
    
    double A_target=0;
    /*
    #ifndef GRAVITY_GALAXY
    if (All.TurbEnergyIn == 0.0 )
    {
        if (ThisTask == 0)
        {
            printf("Zero energy. Energy conservation activated\n");
        }
        c = - (2.0 * (etot_sys - (SysState.EnergyInt + SysState.EnergyPot + SysState.EnergyKin) ) /  P[10].Mass ) ;
        if (c == 0.0)
        {
            if (ThisTask == 0)
            {
                printf("My_error. Something wrong with energies");
            }
            exit(0);
        }
    }
     
    if (All.TurbEnergyIn > 0.0 )
    {
        if (ThisTask == 0)
        {
            printf("The turb energy input is %g ergs\n", All.TurbEnergyIn*All.UnitEnergy_in_cgs);
        }
        c = - (2.0 * All.TurbEnergyIn/P[10].Mass) ;
    }
    */
    //#else
    c = - (2.0 * All.TurbEnergyIn/P[10].Mass) ;
    if (ThisTask == 0)
    {
        printf("The turb energy input is %g ergs\n",  All.TurbEnergyIn*All.UnitEnergy_in_cgs);
    }
   // #endif
    
    double global_a=0; double global_b = 0;
    MPI_Allreduce(&local_a, &global_a, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&local_b, &global_b, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    A_target = solve_quadratic(global_a,global_b,c);
        
    if (A_target <= 0.0)
    {
        if (ThisTask == 0)
        {   printf("Warning, A<=0\n");
        }
       // exit(0);
    }
    if (ThisTask == 0)
    { printf("\tFinal loop\n");
    }
    
    int lll;
    for (lll=0;lll<NumPart;lll++)
    {
       //if(P[lll].Ti_endstep == All.Ti_Current)
       //{
        
        if ( All.DensityDepenTurb == 1 )
        {
            scaling_again = sqrt(All.G * SphP[lll].Density) ;//All.UnitLength_in_cm / All.UnitVelocity_in_cm_per_s;
         
        }
     
        P[lll].TURB_VEL[0] *= (A_target*scaling_again)  ;
        P[lll].TURB_VEL[1] *= (A_target*scaling_again)  ;
        P[lll].TURB_VEL[2] *= (A_target*scaling_again)  ;
        
      // }
    }
   
    DriveFlag=1;
    MPI_Bcast(&DriveFlag, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (ThisTask == 0)
    {   printf("Total number of particles = %lld\n", Ntype[0]);
        //printf("Total energy is %e ergs\n", etot_sys*All.UnitEnergy_in_cgs);
        printf("Turbulence Calculated. %e, %e, %e\n", P[10].TURB_VEL[0], P[10].TURB_VEL[1], P[10].TURB_VEL[2]);
        printf("global_a = %e\n", global_a);
        printf("global_b = %e\n", global_b);
        printf("c = %e\n", c);
        printf("A = %e\n", A_target);
        printf("In case turb is zero, Pos= %e, %e, %e\n",P[10].Pos[0], P[10].Pos[1], P[10].Pos[2] );
        printf("Alternating Flag = %d \n",Alternating);
        printf("Now DriveFlag should be 1 :: %d\n", DriveFlag);
       // double sample1 = trilinear_box(P[10].Pos[0], P[10].Pos[1], P[10].Pos[2], 0, 1) ;
        //double sample2 = trilinear_box(P[10].Pos[0], P[10].Pos[1], P[10].Pos[2], 1, 1) ;
        //double sample3 = trilinear_box(P[10].Pos[0], P[10].Pos[1], P[10].Pos[2], 2, 1) ;
        
    }
    //    printf("global_a = %e\n", global_a);
    //printf("global_b = %e\n", global_b);
    //printf("c = %e\n", c);  
    /// printf("A = %e\n", A_target);
    
}
#endif


/*
 #ifdef TURB_FILES
 char buf[500];
 if(ThisTask == 0)
 { printf("\tOpening turb file\n");
 }
 
 sprintf(buf, "%s%s_%03d", All.OutputDir, All.OutTestTurbFile, All.Time);
 FILE *turb_out = fopen(buf, "w") ;
 if(!(turb_out = fopen(buf, "w")))
 {
 printf("error in opening file '%s'\n", buf);
 endrun(1);
 }
 
 int which_task ;
 for (which_task = 0; which_task<NTask; which_task++)
 {
 int lll2;
 if(ThisTask == which_task)
 {
 
 for (lll2=0;lll2<NumPart;lll2++)
 {
 fprintf(turb_out, "%g, %g, %g, %g, %g, %g\n", P[lll2].Pos[0], P[lll2].Pos[1], P[lll2].Pos[2],
 P[lll2].TURB_VEL[0], P[lll2].TURB_VEL[1], P[lll2].TURB_VEL[2] ) ;
 }
 }
 }
 
 fflush(turb_out);
 #endif
 */
