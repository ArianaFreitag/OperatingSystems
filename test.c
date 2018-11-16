#include <stdio.h>
int main(void) {
  int y;
  int sum = 0;
  float ave;

  for (int x = 0; x < 10; x++) {
    printf("enter int\n");
    scanf("%d", &y);
    sum = y + sum;
    printf("sum now: %d\n", sum);
  }

  ave = sum / 10.0;

  printf("Sum = %d\n", sum);
  printf("average = %.1f\n", ave);
}


// go through quizzes
// go through how to notice bugs
// pair programming
// work on HW assignments
// go over basics
// strategies to start
