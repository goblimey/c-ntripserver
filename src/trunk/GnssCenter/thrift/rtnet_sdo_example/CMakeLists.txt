CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

PROJECT(rtnet_sdo_example)

SET( ${PROJECT_NAME}_MAJOR_VERSION 1 )
SET( ${PROJECT_NAME}_MINOR_VERSION 0 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )

ADD_DEFINITIONS(-DHAVE_NETINET_IN_H -DHAVE_INTTYPES_H)

INCLUDE_DIRECTORIES(.)

ADD_SUBDIRECTORY(common)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}
                    /usr/include/boost
                    /usr/local/include/thrift)

ADD_EXECUTABLE(rtnet_sdo_example
               rtnet_sdo_example.cpp
               Client.cpp Client.h
               RtnetDataHandler.cpp RtnetDataHandler.h
               RtnetDataRequest.cpp RtnetDataRequest.h
               RtnetDataListener.cpp RtnetDataListener.h
               ConnectionRequiredRunnable.cpp ConnectionRequiredRunnable.h
               ConnectionStatusMonitor.cpp ConnectionStatusMonitor.h
              )
TARGET_LINK_LIBRARIES(rtnet_sdo_example lib_gpssthrift thrift rt ssl z pthread)

INSTALL(TARGETS rtnet_sdo_example DESTINATION bin PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE
)

