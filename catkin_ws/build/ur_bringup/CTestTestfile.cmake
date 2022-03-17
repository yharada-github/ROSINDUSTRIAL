# CMake generated Testfile for 
# Source directory: /home/ubuntu/catkin_ws/src/universal_robot/ur_bringup
# Build directory: /home/ubuntu/catkin_ws/build/ur_bringup
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(_ctest_ur_bringup_roslaunch-check_tests_roslaunch_test.xml "/home/ubuntu/catkin_ws/build/ur_bringup/catkin_generated/env_cached.sh" "/usr/bin/python3" "/opt/ros/noetic/share/catkin/cmake/test/run_tests.py" "/home/ubuntu/catkin_ws/build/ur_bringup/test_results/ur_bringup/roslaunch-check_tests_roslaunch_test.xml.xml" "--return-code" "/usr/bin/cmake -E make_directory /home/ubuntu/catkin_ws/build/ur_bringup/test_results/ur_bringup" "/opt/ros/noetic/share/roslaunch/cmake/../scripts/roslaunch-check -o \"/home/ubuntu/catkin_ws/build/ur_bringup/test_results/ur_bringup/roslaunch-check_tests_roslaunch_test.xml.xml\" \"/home/ubuntu/catkin_ws/src/universal_robot/ur_bringup/tests/roslaunch_test.xml\" ")
set_tests_properties(_ctest_ur_bringup_roslaunch-check_tests_roslaunch_test.xml PROPERTIES  _BACKTRACE_TRIPLES "/opt/ros/noetic/share/catkin/cmake/test/tests.cmake;160;add_test;/opt/ros/noetic/share/roslaunch/cmake/roslaunch-extras.cmake;66;catkin_run_tests_target;/home/ubuntu/catkin_ws/src/universal_robot/ur_bringup/CMakeLists.txt;11;roslaunch_add_file_check;/home/ubuntu/catkin_ws/src/universal_robot/ur_bringup/CMakeLists.txt;0;")
subdirs("gtest")
