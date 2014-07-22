CS 5348 
Operating Systems Concepts
Project 2
Threads

Name: Madhusudan Pranav Venugopal
UTD id: 2021163441
netid: mxv130430

The project folder contains the folowing documents in it:

1. Source code ( project2.cpp)
2. movies.txt - (text file containing the movies name and number of seats available)
3 Design document ( design.docx)
4. Summary document ( summary.docx)
5. Readme file ( readme.txt)

Steps for compiling and executing the project:

Step 1: Open a text editor and paste the source code in it

Step 2: Save the file as <filename.cpp>
eg: 
project2.cpp

Step 3: Compile the file using the instruction 

g++  <filename.cpp> -pthread  -o <filename>

eg:
 g++ project2.cpp -pthread -o project2

Step 4: Run the project using the instruction
./<filename>  <sample filename.txt> 

or
<filename>  <sample filename.txt> 
 
eg:

./project2 movies.txt

or 
project2 movies.txt
