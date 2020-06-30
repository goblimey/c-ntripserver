INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}
                    ${PROJECT_SOURCE_DIR}/3rd_party ${PROJECT_SOURCE_DIR}/3rd_party/thrift
                    ${PROJECT_BINARY_DIR}/3rd_party/thrift
                    )

IF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/common/thrift)
  FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/common/thrift)
ENDIF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/common/thrift)

#IF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_js)
#  FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_js)
#ENDIF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_js)

#IF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_perl)
#  FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_perl)
#ENDIF (NOT IS_DIRECTORY ${PROJECT_BINARY_DIR}/thrift_perl)

SET (gpssthrift_sources
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_constants.cpp
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_constants.h
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_types.cpp
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_types.h
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.cpp
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.h
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.cpp
     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.h
#     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_constants.cpp
#     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_constants.h
#     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_types.cpp
#     ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_types.h
     ${PROJECT_BINARY_DIR}/common/thrift/RtnetData.cpp
     ${PROJECT_BINARY_DIR}/common/thrift/RtnetData.h
#     ${PROJECT_BINARY_DIR}/common/thrift/RtnetDataJs.cpp
#     ${PROJECT_BINARY_DIR}/common/thrift/RtnetDataJs.h
    )

ADD_LIBRARY(lib_gpssthrift ${gpssthrift_sources})
# create symbolic lib_codecs target for calling target gpsscodecs
##ADD_CUSTOM_TARGET( lib_gpssthrift DEPENDS lib_gpssthrift )
# change lib_target properties
SET_TARGET_PROPERTIES( lib_gpssthrift PROPERTIES
  # create *nix style library versions + symbolic links
  VERSION 1
  SOVERSION 1
  # allow creating static and shared libs without conflicts
  CLEAN_DIRECT_OUTPUT 1
  # avoid conflicts between library and binary target names
  OUTPUT_NAME gpssthrift )

INSTALL(TARGETS lib_gpssthrift DESTINATION lib PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE
)

#SET (lib_gpss_js ${PROJECT_BINARY_DIR}/thrift_js/RtnetDataJs.js
#                 ${PROJECT_BINARY_DIR}/thrift_js/rtnet_data_types.js
#                 ${PROJECT_BINARY_DIR}/thrift_js/rtnet_js_types.js
#    )

#INSTALL(FILES ${lib_gpss_js} DESTINATION www PERMISSIONS
#  OWNER_READ OWNER_WRITE OWNER_READ
#  GROUP_READ GROUP_READ
#  WORLD_READ WORLD_READ
#)

ADD_CUSTOM_COMMAND (
  OUTPUT ${PROJECT_BINARY_DIR}/common/thrift/rtnet_constants.cpp
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_constants.h
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.cpp
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.h
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.cpp
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.h
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_types.cpp
         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_types.h
         ${PROJECT_BINARY_DIR}/common/thrift/RtnetData.cpp
         ${PROJECT_BINARY_DIR}/common/thrift/RtnetData.h

  COMMAND cmake -E echo Generating rtnet Thrift code
  COMMAND cmake -E echo thrift -r --gen cpp --out ${PROJECT_BINARY_DIR}/common/thrift ${PROJECT_SOURCE_DIR}/thrift_template/rtnet.thrift
  COMMAND thrift --gen cpp -r --out ${PROJECT_BINARY_DIR}/common/thrift ${PROJECT_SOURCE_DIR}/thrift_template/rtnet.thrift
  # -r (recursive) apparently doesn't work for perl generator
#  COMMAND cmake -E echo thrift --gen perl --out ${PROJECT_BINARY_DIR}/common/thrift_perl ${PROJECT_SOURCE_DIR}/thrift_template/rtnet.thrift
#  COMMAND thrift --gen perl --out ${PROJECT_BINARY_DIR}/thrift_perl ${PROJECT_SOURCE_DIR}/thrift_template/rtnet.thrift
#  COMMAND cmake -E echo thrift --gen perl --out ${PROJECT_BINARY_DIR}/common/thrift_perl ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_data.thrift
#  COMMAND thrift --gen perl --out ${PROJECT_BINARY_DIR}/thrift_perl ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_data.thrift

  DEPENDS ${PROJECT_SOURCE_DIR}/thrift_template/rtnet.thrift
          ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_data.thrift
)
#ADD_CUSTOM_COMMAND (
#  OUTPUT ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_constants.cpp
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_constants.h
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_types.cpp
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_js_types.h
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.cpp
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_constants.h
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.cpp
#         ${PROJECT_BINARY_DIR}/common/thrift/rtnet_data_types.h
#         ${PROJECT_BINARY_DIR}/common/thrift/RtnetDataJs.cpp
#         ${PROJECT_BINARY_DIR}/common/thrift/RtnetDataJs.h
#         ${PROJECT_BINARY_DIR}/thrift_js/RtnetDataJs.js
#         ${PROJECT_BINARY_DIR}/thrift_js/rtnet_data_types.js
#         ${PROJECT_BINARY_DIR}/thrift_js/rtnet_js_types.js

#  COMMAND cmake -E echo Generating rtnet JavaScript Thrift code
#  COMMAND cmake -E echo thrift -r --gen cpp --out ${PROJECT_BINARY_DIR}/common/thrift ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_js.thrift
#  COMMAND thrift -r --gen cpp --out ${PROJECT_BINARY_DIR}/common/thrift ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_js.thrift
#  COMMAND cmake -E echo thrift -r --gen js:jquery --out ${PROJECT_BINARY_DIR}/thrift_js ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_js.thrift
#  COMMAND thrift -r --gen js:jquery --out ${PROJECT_BINARY_DIR}/thrift_js ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_js.thrift

#  DEPENDS ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_js.thrift
#          ${PROJECT_SOURCE_DIR}/thrift_template/rtnet_data.thrift
#)
