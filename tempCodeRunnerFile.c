#include<stdio.h>
unsigned long recfib(unsigned int n){
if(n==0){
	return 0;
}
else if	(n==1){
       	return 1;
}
else{
return recfib(n-1) + recfib(n-2);
}
}
unsigned long fibonacci(unsigned int n){
    if(n == 0){
        return 0;
    }
    else if (n == 1){
        return 1;
    }
    else{
        unsigned long t1 = 0, t2 = 1;
        unsigned long next = 0;
        for(int i = 2; i <=n; i++){
            next = t1 + t2;
            t1 = t2;
            t2 = next;
        }
        return next;
    }

}

int main(){
printf("Which fibonacci number n do you want ? ");
unsigned int n;
scanf("%d",&n);
unsigned long result = fibonacci(n);
printf("Die %d-te Fibonacci Zahl lautet: %ld\n", n, result);
return 0;
}
