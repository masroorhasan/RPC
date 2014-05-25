#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {

  unsigned char *buffer[256];

  int first_parameter = 10;
  char *second_parameter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int third_parameter = 20;

  int serialize_offset = 0;

  // Copy first parameter size into buffer
  int size_first_parameter = sizeof(first_parameter);
  memcpy(buffer, &size_first_parameter, sizeof(size_first_parameter));
  serialize_offset += sizeof(size_first_parameter);

  // Copy first parameter into buffer
  memcpy(buffer + serialize_offset, &first_parameter, size_first_parameter);
  serialize_offset += size_first_parameter;

  // Copy second parameter size into buffer
  int size_second_parameter = sizeof(char) * strlen(second_parameter);
  memcpy(buffer + serialize_offset, &size_second_parameter, sizeof(size_second_parameter));
  serialize_offset += sizeof(size_second_parameter);

  printf("The size of the second parameter is: %i\n", size_second_parameter);

  // Copy second parameter into buffer
  memcpy(buffer + serialize_offset, second_parameter, size_second_parameter);
  serialize_offset += size_second_parameter;

  // Copy third parameter size into buffer
  int size_third_parameter = sizeof(third_parameter);
  memcpy(buffer + serialize_offset, &size_third_parameter, sizeof(third_parameter));
  serialize_offset += sizeof(third_parameter);

  // Copy third parameter into buffer
  memcpy(buffer+serialize_offset, &third_parameter, size_third_parameter);

  int deserialize_first;
  int deserialize_first_size;
  int deserialize_second_size;
  int deserialize_third_size;
  int deserialize_third;
  int deserialize_offset = 0;

  // Extract first parameter from buffer
  memcpy(&deserialize_first_size, buffer, sizeof(int));
  deserialize_offset += sizeof(int);
  printf("deserialize: The first thing from the buffer is: %i\n", deserialize_first_size);

  memcpy(&deserialize_first, buffer + deserialize_offset, deserialize_first_size);
  deserialize_offset += deserialize_first_size;
  printf("deserialize: The second thing from the buffer is %i\n", deserialize_first);

  memcpy(&deserialize_second_size, buffer + deserialize_offset, sizeof(int));
  deserialize_offset += sizeof(int);
  printf("deserialize: The third thing from the buffer is %i\n", deserialize_second_size);

  char *deserialize_second = malloc(deserialize_second_size);

  memcpy(deserialize_second, buffer + deserialize_offset, deserialize_second_size);
  deserialize_offset += deserialize_second_size;
  printf("deserialize: The fourth thing from the buffer is %s\n", deserialize_second);

  memcpy(&deserialize_third_size, buffer + deserialize_offset, sizeof(int));
  deserialize_offset += sizeof(int);
  printf("deserialize: The fifth thing from the buffer is %i\n", deserialize_third_size);

  memcpy(&deserialize_third, buffer + deserialize_offset, deserialize_third_size);

  printf("The sixth thing from the buffer is %i\n", deserialize_third);
}
