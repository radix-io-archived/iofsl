BMIDIR=$(HOME)/ioforward/install

CC = gcc
CXX = g++
LD = g++
CPPFLAGS = 
CFLAGS = -fno-inline  -g -Wall -I$(BMIDIR)/include
CXXFLAGS = -fno-inline -g -Wall 
ADDLIBS = -lbmi -lpthread
CXXCPP = g++ -E 
LDFLAGS = -L$(BMIDIR)/lib

SUBDIRS = src/iofwd src/frontend src/backend sandbox



include Makefile.buildsys

