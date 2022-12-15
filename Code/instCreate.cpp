#include<cstdlib> 
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
using namespace std;
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    stringstream ss;
    ss.str(s);
    string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
 // type: r=0, i=1, sw=2; lw=3; beq=4; 
// rtype: add=0, sub=1, and=2, or=3;
// itype: addi=0, andi=1, ori=2

 int main()
 {
 	
  	ofstream myfile;
  	myfile.open ("instMem-test.txt");
  	ifstream myAsm;
  	myAsm.open ("22test.txt");
 	string line;
 	getline(myAsm,line); // getting rid of first line
 	vector<string> parts;
 	string inst; 
 	string myByte;
//   split(line,'\t',parts);
    while(getline(myAsm,line))
    	{
    		
    		split(line,'\t',parts);
    		if( parts.size() >3 )
    		{
    			inst = parts[3]; 
    			for (int i=0; i<7; i+=2)
    			{
    				myByte.push_back(inst.at(6-i));
    				myByte.push_back(inst.at(7-i));
    				myfile <<stoi(myByte,nullptr,16)  << endl;
    				myByte.clear();
    			}
    		}
    		parts.clear();

    	}
  	
 	
 	myfile.close();
 	myAsm.close();
 }
 