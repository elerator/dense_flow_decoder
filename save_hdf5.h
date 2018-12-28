#include "H5Cpp.h"
#include <vector>
#include <string>

using namespace H5;
using namespace std;

typedef std::vector<int> v1d;
typedef std::vector<v1d> v2d;
typedef std::vector<v2d> v3d;

class SaveHdf5
{
public:
  hsize_t      NX;//The size in x direction
  hsize_t      NY;

  H5std_string FILE_NAME;
  H5std_string DATASET_NAME;

  const int      RANK = 4;//the rank of dimensions. here it's 2d
  DataSet dataset;
  DataSpace mspace1;

  int current_frame;

  SaveHdf5(string, string);
  DataSpace selectFrame(DataSet & dataset, int frame_number);
  int init (int, int);
  int append(v3d);
  void print_frame(int);
  int get_current_size();
  int * flatten_v3d(v3d);

};
