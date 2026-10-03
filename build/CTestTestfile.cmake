# CMake generated Testfile for 
# Source directory: /app
# Build directory: /app/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(public_tests "/app/build/public_tests")
set_tests_properties(public_tests PROPERTIES  _BACKTRACE_TRIPLES "/app/CMakeLists.txt;26;add_test;/app/CMakeLists.txt;0;")
add_test(student_tests "/app/build/student_tests")
set_tests_properties(student_tests PROPERTIES  _BACKTRACE_TRIPLES "/app/CMakeLists.txt;33;add_test;/app/CMakeLists.txt;0;")
