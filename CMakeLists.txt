cmake_minimum_required(VERSION 3.0)
project(mftest1)

set (CMAKE_CXX_STANDARD 11)

add_executable(mftest1
    main.cpp
    MFInfo.cpp
    MFInfo.h
)

TARGET_LINK_LIBRARIES (mftest1
  mf
  mfplat
  mfuuid
  Mfreadwrite
  strmiids
  comsuppw
)