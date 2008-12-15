CC = gcc
CXX = g++
LD = g++
CPPFLAGS = 
CFLAGS = -fno-inline  -g -Wall
CXXFLAGS = -fno-inline -g -Wall
ADDLIBS = 
CXXCPP = g++ -E 

SUBDIRS = src


include Makefile.buildsys

