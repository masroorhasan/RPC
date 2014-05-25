#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {

  unsigned char *buffer[256];

  int first_parameter = 10;
  char *second_parameter = "Warren";
  int third_parameter = 20;

  // Copy first parameter size into buffer
  int size_first_parameter = sizeof(first_parameter);
  memcpy(buffer, &size_first_parameter, sizeof(size_first_parameter));

  // Copy first parameter into buffer
  memcpy(buffer + sizeof(size_first_parameter), &first_parameter, size_first_parameter);

  // Copy second parameter size into buffer
  int size_second_parameter = sizeof(second_parameter);
  memcpy(buffer + sizeof(size_first_parameter) + size_first_parameter, &size_second_parameter, sizeof(size_second_parameter));

  printf("The size of the second parameter is: %i\n", size_second_parameter);

  // Copy second parameter into buffer
  memcpy(buffer + sizeof(size_first_parameter) + size_first_parameter + sizeof(size_second_parameter), second_parameter, size_second_parameter);

  // Copy third parameter size into buffer
  int size_third_parameter = sizeof(third_parameter);
  memcpy(buffer + sizeof(size_first_parameter) + size_first_parameter + sizeof(size_second_parameter) + size_second_parameter, 
    &size_third_parameter, sizeof(third_parameter));

  // Copy third parameter into buffer
  memcpy(buffer + sizeof(size_first_parameter) + size_first_parameter + sizeof(size_second_parameter) + size_second_parameter + sizeof(third_parameter),
      &third_parameter, size_third_parameter);

  int deserialize_first;
  int deserialize_first_size;
  int deserialize_second_size;
  int deserialize_third_size;
  int deserialize_third;

  // Extract first parameter from buffer
  memcpy(&deserialize_first_size, buffer, sizeof(int));
  printf("The first thing from the buffer is: %i\n", deserialize_first_size);

  memcpy(&deserialize_first, buffer + sizeof(int), deserialize_first_size);
  printf("The second thing from the buffer is %i\n", deserialize_first);

  memcpy(&deserialize_second_size, buffer + sizeof(int) + deserialize_first_size, sizeof(int));
  printf("The third thing from the buffer is %i\n", deserialize_second_size);

  char *deserialize_second = malloc(deserialize_second_size);

  memcpy(deserialize_second, buffer + sizeof(int) + deserialize_first_size + sizeof(int), deserialize_second_size);
  printf("The fourth thing from the buffer is %s\n", deserialize_second);

  memcpy(&deserialize_third_size, buffer + sizeof(int) + deserialize_first_size + sizeof(int) + deserialize_second_size, sizeof(int));
  printf("The fifth thing from the buffer is %i\n", deserialize_third_size);

  memcpy(&deserialize_third, buffer + sizeof(int) + deserialize_first_size + sizeof(int) + deserialize_second_size + sizeof(int),
    deserialize_third_size);

  printf("The sixth thing from the buffer is %i\n", deserialize_third);
}
