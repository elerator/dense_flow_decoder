# dense_flow_decoder

# Description

A tool to extract motion vectors from H264 videos and save them as a tensor of size n_frames x width x height x 2 (for dx and dy) in HDF5 format (One vector for each 16x16 pixels macroblock. Vectors for 8x8 blocks are summerized as this yields a more dense representation. The unit of dx and dy are pixels of the original frames)              

# Input 

Video encoded in H264. Other formats may work as well.                

# Output 

HDF5 file named motion_vectors.h5 containing motion tensor at node motion_tensor.                                                        

# Usage: 

./decode_motion filepath_to_video.mp4 motion_vectors.h5  
See jupyter notebook for sample run and loading HDF5 via pytables in python 3.

# Version History

If using please cite.

This version is based on MV-Tractus by Jishnu (2018). See license for version history and full list of authors of previous versions.

# Installation

To install lHDF5 see: http://micro.stanford.edu/wiki/Install_HDF5

For installation advice for ffmpeg see:  https://github.com/jishnujayakumar/MV-Tractus

Makefile provided for build from source. Binary for Ubuntu 16.04. provided.

Installation was tested on Ubuntu 16.04. If using Windows consider https://docs.microsoft.com/en-us/windows/wsl/install-win10

# Sources:
Jishnu P. (2018, October 21). MV-Tractus:  A simple and fast tool to extract motion vectors from H264 encoded video streams. (Version 1.0). Zenodo. http://doi.org/10.5281/zenodo.1467851
