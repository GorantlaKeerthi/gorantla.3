Program for solving Problem 1 (Partition the n integers into n/2 pairs) is in bin_adder1_1 folder

Program for solving Problem 2 (Divide the n integers into n/log n groups of log n numbers each) is in bin_adder1_2 folder 

we can compile Both programs  with make command and are executed the following way:
  $ ./master 100.txt
Result = 100

$ ./master 646.txt
Result = 646
You can also check the adder_log.txt file for the results.
Comapring methods:
The child process in method1 took more time than in method, 
By calculating this i got nearly 0.78 microseconds difference in child  execution of child process of two methods.
Merging the pairs in each method  method1 took more time than method2.
I pushed all these files in to  github.
/classes/OS/gorantla/gorantla.3/log
