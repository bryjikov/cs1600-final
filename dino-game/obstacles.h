/*
   obstacles.h
   Types for managing obstacles.
*/

// Represents an obstacle that moves across the screen.
typedef struct {
  uint8_t x;
  uint8_t y;
} obstacle_t;

// Represents the direction that obstacles are currently moving.
typedef enum {
  LEFT,
  RIGHT
} direction_t;
