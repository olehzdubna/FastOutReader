# FastOutReader
## Build 
1. mkdir build
2. cd build
3. cmake ..
4. make 
## Run 
Let us say input Fast Binary data files are located in the directory /data such as /data/test1 /data/test2 etc. All is needed is to specify the path to the first file such as /data/test1 as input to the fastread  tool. Sample usage is the following
:
>>> ./fastread -i <required: path to input file> -o <required: path to output file> -g <optional: path to custom installation of gtkwave, for display>
