// Simple test file for preprocessor directive tracking

#define MAX_SIZE 100
#define MIN_SIZE 0

#ifdef DEBUG
  #define LOG_ENABLED 1
#else
  #define LOG_ENABLED 0
#endif

#ifndef RELEASE
  #define TEST_MODE 1
#elif TEST_MODE == 1
  #define VERBOSE 1
#else
  #define VERBOSE 0
#endif

#undef MAX_SIZE

#pragma once

#if MAX_SIZE > 50
  int x = 1;
#elif MAX_SIZE > 25
  int x = 2;
#else
  int x = 3;
#endif

int main() {
  return LOG_ENABLED + TEST_MODE + x;
}
