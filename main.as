#include "scriptinclude.as"

void main()
{


float startTime = GetTimeNow();



int a = fib(5);

sleep(5000);

print("fibs : %d ",a);

float endTime = GetTimeNow();


float total = (endTime-startTime);

print("time pass : %f ",total);


print("GetTimeNow : %f ",GetTimeNow());
print("GetTime : %d ",GetTime());
print("GetTimePass : %d ",GetTimePass());







}
