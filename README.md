"# T9 over Salt and Pepper"
 
The repository contains an implementation for word prediction for T9 keypad based on a dictionary. 
Google's Native Client(NaCl) aka Salt and Pepper Plugin was used to provide an interface over the browser.


The contents include
1. predictor_new.cc - a stand alone C based 9-way tree implementation for T9
2. hello_tutorial.cc - an adaptation from native client/pepper library to interface with predictor
3. hello_tutorial.html - to show a preliminary UI for T9 keyboard
4. dict.js - the javascript file used to interface between C code and html
5. *.csv files - dictionary files that contains words separated by commas
6. *.nexe files - binaries compiled for x86 using NaCl tool chain
7. nmf - Native Client Manifest file
