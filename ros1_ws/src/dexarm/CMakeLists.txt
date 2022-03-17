cmake_minimum_required(VERSION 3.0.2)
project(dexarm)


## is used, also find other catkin packages
find_package(catkin REQUIRED)


catkin_package()

install(DIRECTORY URDF URDF/STL DESTINATION share/${PROJECT_NAME}/)

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

