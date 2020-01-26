#include <stdio.h>
#include <stdint.h>

struct seconds { uint32_t val; };
struct milliseconds { uint32_t val; }; 
 
struct seconds add_seconds(struct seconds a, struct seconds b) {
  return (struct seconds) { a.val + b.val };
}
int main() {
  struct seconds x = { 10 };
  struct milliseconds y = { 20 };
 
  // oops!
  struct seconds result = add_seconds(x, y); 
  printf("seconds: %u\n", result.val);
}