﻿CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

ADD_EXECUTABLE(dpf_tests 
    "dpf_tests.cpp"
)

TARGET_INCLUDE_DIRECTORIES(dpf_tests PRIVATE "${CMAKE_SOURCE_DIR}")
TARGET_INCLUDE_DIRECTORIES(dpf_tests PRIVATE "${CMAKE_SOURCE_DIR}/include")

TARGET_LINK_LIBRARIES(dpf_tests PRIVATE dpf)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS	"/std:c++20 /EHsc")
ENDIF()

ADD_TEST(NAME patch			 COMMAND dpf_tests 0 ${CMAKE_SOURCE_DIR})
ADD_TEST(NAME check_checksum COMMAND dpf_tests 1 ${CMAKE_SOURCE_DIR})
ADD_TEST(NAME get_files		 COMMAND dpf_tests 2 ${CMAKE_SOURCE_DIR})
ADD_TEST(NAME is_dpf_file	 COMMAND dpf_tests 3 ${CMAKE_SOURCE_DIR})
