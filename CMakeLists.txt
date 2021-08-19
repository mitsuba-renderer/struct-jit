cmake_minimum_required(VERSION 3.13...3.18)

project(enoki
  DESCRIPTION "struct-jit"
  LANGUAGES CXX
)

add_library(
  struct-jit
  SHARED
  struct-jit.cpp
  struct-jit.h
)
