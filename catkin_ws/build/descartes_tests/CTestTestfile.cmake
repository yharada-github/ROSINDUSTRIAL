# CMake generated Testfile for 
# Source directory: /home/ubuntu/catkin_ws/src/descartes/descartes_tests
# Build directory: /home/ubuntu/catkin_ws/build/descartes_tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(_ctest_descartes_tests_gtest_descartes_tests_trajectory_utest "/home/ubuntu/catkin_ws/build/descartes_tests/catkin_generated/env_cached.sh" "/usr/bin/python3" "/opt/ros/noetic/share/catkin/cmake/test/run_tests.py" "/home/ubuntu/catkin_ws/build/descartes_tests/test_results/descartes_tests/gtest-descartes_tests_trajectory_utest.xml" "--return-code" "/home/ubuntu/catkin_ws/devel/.private/descartes_tests/lib/descartes_tests/descartes_tests_trajectory_utest --gtest_output=xml:/home/ubuntu/catkin_ws/build/descartes_tests/test_results/descartes_tests/gtest-descartes_tests_trajectory_utest.xml")
set_tests_properties(_ctest_descartes_tests_gtest_descartes_tests_trajectory_utest PROPERTIES  _BACKTRACE_TRIPLES "/opt/ros/noetic/share/catkin/cmake/test/tests.cmake;160;add_test;/opt/ros/noetic/share/catkin/cmake/test/gtest.cmake;98;catkin_run_tests_target;/opt/ros/noetic/share/catkin/cmake/test/gtest.cmake;37;_catkin_add_google_test;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;53;catkin_add_gtest;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;0;")
add_test(_ctest_descartes_tests_gtest_descartes_tests_planner_utest "/home/ubuntu/catkin_ws/build/descartes_tests/catkin_generated/env_cached.sh" "/usr/bin/python3" "/opt/ros/noetic/share/catkin/cmake/test/run_tests.py" "/home/ubuntu/catkin_ws/build/descartes_tests/test_results/descartes_tests/gtest-descartes_tests_planner_utest.xml" "--return-code" "/home/ubuntu/catkin_ws/devel/.private/descartes_tests/lib/descartes_tests/descartes_tests_planner_utest --gtest_output=xml:/home/ubuntu/catkin_ws/build/descartes_tests/test_results/descartes_tests/gtest-descartes_tests_planner_utest.xml")
set_tests_properties(_ctest_descartes_tests_gtest_descartes_tests_planner_utest PROPERTIES  _BACKTRACE_TRIPLES "/opt/ros/noetic/share/catkin/cmake/test/tests.cmake;160;add_test;/opt/ros/noetic/share/catkin/cmake/test/gtest.cmake;98;catkin_run_tests_target;/opt/ros/noetic/share/catkin/cmake/test/gtest.cmake;37;_catkin_add_google_test;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;64;catkin_add_gtest;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;0;")
add_test(_ctest_descartes_tests_rostest_test_moveit_launch_utest.launch "/home/ubuntu/catkin_ws/build/descartes_tests/catkin_generated/env_cached.sh" "/usr/bin/python3" "/opt/ros/noetic/share/catkin/cmake/test/run_tests.py" "/home/ubuntu/catkin_ws/build/descartes_tests/test_results/descartes_tests/rostest-test_moveit_launch_utest.xml" "--return-code" "/usr/bin/python3 /opt/ros/noetic/share/rostest/cmake/../../../bin/rostest --pkgdir=/home/ubuntu/catkin_ws/src/descartes/descartes_tests --package=descartes_tests --results-filename test_moveit_launch_utest.xml --results-base-dir \"/home/ubuntu/catkin_ws/build/descartes_tests/test_results\" /home/ubuntu/catkin_ws/src/descartes/descartes_tests/test/moveit/launch/utest.launch ")
set_tests_properties(_ctest_descartes_tests_rostest_test_moveit_launch_utest.launch PROPERTIES  _BACKTRACE_TRIPLES "/opt/ros/noetic/share/catkin/cmake/test/tests.cmake;160;add_test;/opt/ros/noetic/share/rostest/cmake/rostest-extras.cmake;52;catkin_run_tests_target;/opt/ros/noetic/share/rostest/cmake/rostest-extras.cmake;80;add_rostest;/opt/ros/noetic/share/rostest/cmake/rostest-extras.cmake;100;_add_rostest_google_test;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;76;add_rostest_gtest;/home/ubuntu/catkin_ws/src/descartes/descartes_tests/CMakeLists.txt;0;")
subdirs("gtest")
