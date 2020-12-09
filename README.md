# Final Project Writeup

## Running Code
From the root directory, `cd` into the build folder, then run `cmake ..` and `make`, which will produce the binary called "assignment6". Then you can run the test cases like in the handout. Also, there is a pre-compiled binary of my final code in the root directory called "assignment6". I have only tested my code on Macos which is the OS that i run. 

You can specify can indexer using the "-indexer" flag like the following command: 
```bash
./assignment6 -input scene06_bunny_1k.txt -output 06_bvh.png -size 2000 2000 -bounces 30 -indexer bvh
```
where the "-indexer" flag takes in either "octree", "bvh", or "bvh_sah". Bvh is the Bounding Volume Hierarchy indexer with a naive splitting algorithm and bvh_sah is the BVH with a Surface Area Heuristic splitting algorithm.