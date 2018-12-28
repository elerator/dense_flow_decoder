#include <iostream>
#include <string>
#include <cstdlib>


#include "save_hdf5.h"
#include "H5Cpp.h"
using namespace H5;
using std::cout;
using std::endl;


// This is what i used to compile:
// g++ 3d_hdf5.cpp -I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lhdf5 -lhdf5_cpp -o test
// alternative (gcc requires adding -lstdc++ manually)
// gcc save_hdf5.cpp -I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp -o test

int SaveHdf5::init (int NY, int NX)
{
  //* Initializes the file and the 4D dataspace*//
  this->NX = NX;//Set x and y dimesnions
  this->NY = NY;

   try
   {
      Exception::dontPrint();//Turn off the auto-printing when failure occurs so that we can handle the errors appropriately

      //1. CREATE DATASPACE OF FRAME SIZE NY NX  (unlimited length)
      hsize_t      dims[RANK]  = {1, NY, NX, 2};  // dataset dimensions at creation
      hsize_t      maxdims[RANK] = {H5S_UNLIMITED, NY, NX, 2};//Create the data space with unlimited dimensions.
      this->mspace1 = DataSpace( RANK, dims, maxdims);

      //2. CREATE A FILE (THAT WILL INCLUDE OUR DATASET)
      H5File file( FILE_NAME, H5F_ACC_TRUNC );// Create a new file. If file exists its contents will be overwritten.

      //3. CREATE A DATASET WITHIN THE FILE
      DSetCreatPropList cparms;// NEEDED FOR DATASET CREATION
      hsize_t      chunk_dims[RANK] ={1, NY, NX, 2};
      cparms.setChunk( RANK, chunk_dims );//enable chunking
      //int fill_val = 0;      //Set fill value for the dataset
      //cparms.setFillValue(PredType::NATIVE_INT, &fill_val);

      this->dataset = file.createDataSet( DATASET_NAME, PredType::NATIVE_INT, mspace1, cparms);  // Create dataset here using cparms (sse 3.) and myspace (see 1.)

   }  // end of try block

   // catch failure caused by the H5File operations
   catch( FileIException error )
   {
      error.printErrorStack();
      return -1;
   }

   // catch failure caused by the DataSet operations
   catch( DataSetIException error )
   {
      error.printErrorStack();
      return -1;
   }

   // catch failure caused by the DataSpace operations
   catch( DataSpaceIException error )
   {
      error.printErrorStack();
      return -1;
   }

   // catch failure caused by the DataSpace operations
   catch( DataTypeIException error )
   {
      error.printErrorStack();
      return -1;
   }
   return 0;
}

int SaveHdf5::get_current_size(){
  /* Returns the number of ints stored in the HDF5 file*/
  return this->NX * this->NY * this->current_frame *2 +1;
}

int * SaveHdf5::flatten_v3d(v3d v){
  //* Flatten v3d to continuous data. *//
    int nz = v[0][0].size();
    int nx = v[0].size();
    int ny = v.size();

    int * data = (int*)malloc( nx*ny*nz*sizeof(int) );
    int pos_in_1d= 0;

    for(int y = 0; y<ny; y++){
        for(int x =0;x<nx;x++){
            for(int z=0; z<nz; z++){
                data[pos_in_1d] = v[y][x][z];
                pos_in_1d++;
            }
        }
    }
    return data;
}

DataSpace SaveHdf5::selectFrame(DataSet & dataset, int frame_number){
      /** Select a hyperslab. Namely one frame in the video i.e. a slice along the third dimension*/
      DataSpace fspace1 = dataset.getSpace ();
      hsize_t     offset[4] = {0,0,0,0};
      offset[0] = frame_number;
      hsize_t      dims1[4] = {1,NY,NX,2};            /* data1 dimensions */
      fspace1.selectHyperslab( H5S_SELECT_SET, dims1, offset );
      return fspace1;
}


int SaveHdf5::append (v3d data)
{//** Appends a threedimensional vector to the HDF file**//
  int * data2 = flatten_v3d(data);

      //5. Extend the dataset to write another frame.
      hsize_t   new_size[4] = {this->current_frame+1,NY,NX,2};            // data2 dimensions  insted of 4,4,1,2 NX TODO NY SWAPPED?
      dataset.extend( new_size );

      //4. Select a frame and write
      DataSpace frame = selectFrame(dataset, this->current_frame);
      dataset.write(data2, PredType::NATIVE_INT, this->mspace1, frame);

      free(data2);
      //print_frame(0);
      current_frame++;

}

void SaveHdf5::print_frame(int frame){
  //** Prints one frame in the video. In other words: One slice of the 4d Tensor alomg the third axis.**//

  //7. Read dataset in an appropriate integer array
  int i, j;
  //int size = 1;//this->get_current_size();//TODO
  int size = this->current_frame;
  int data_out[size][NX][NY][2];//
  dataset.read( data_out, PredType::NATIVE_INT );

  //8. Frame
  for (i=0; i < NX; i++)
  {
      for(j=0; j < NY; j++)
         cout << data_out[frame][i][j][0] << "," << data_out[frame][i][j][1] << "  ";
      cout << endl;
  }

  cout << endl;
}

SaveHdf5::SaveHdf5(string filename, string dataset_name){
  this->FILE_NAME = filename;
  this->DATASET_NAME = dataset_name;
  this->current_frame = 0;
}

/*

int main(void){
  int initval = 8;
  int NY = 67;
  int NX = 120;

  v3d data(NY, v2d(NX, v1d(2, initval)));

  initval = 9;
  v3d data2(NY, v2d(NX, v1d(2, initval)));

  initval = 2;
  v3d data3(NY, v2d(NX, v1d(2, initval)));

  SaveHdf5 * saver = new SaveHdf5("test.h5","motion_tensor");

  saver->init(NY,NX);
  for(int x = 0 ; x < 1; x++) saver->append(data);
  saver->print_frame(0);

  saver->append(data2);
  //saver->print_frame(100);
  std::cout<< saver->current_frame<<std::endl;

  //std::cout<<saver->flatten_v3d(data)[0]<<std::endl;

  return 1;
}*/
