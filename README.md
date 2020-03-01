# FastOutReader
## Build 
1. mkdir build
2. cd build
3. cmake ..
4. make 
### Build Notes
- For user convenience there is an optional configuration setting **BUILD_GTKWAVE** provided in case a user would like to display binary Fast data and they do not happen to have *gtkwave* on their system. So before kicking off the build via step 4. from the above instructions a user would run step 3. as follows:
>>> cmake -D BUILD_GTKWAVE=true ..
## Run 
Let us say input Fast Binary data files are located in the directory /data such as /data/test1 /data/test2 etc. All is needed is to specify the path to the first file such as /data/test1 as input to the fastread  tool. Sample usage is the following
:
>>>./fastread -i <required: path to input file> -o <required: path to output file> -g <optional: path to custom installation of gtkwave, for display>
