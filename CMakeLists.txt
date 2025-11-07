cmake_minimum_required(VERSION 3.15)

project(coreMQTT-Demo C)

# ------------------------------------------------------------------------------
# Includes
# ------------------------------------------------------------------------------

include(FetchContent)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# Target Support Repo - Gives needed CMake targets for the libraries used in
# this demo.

if(DEFINED TARGET_SUPPORT_REPO)

  # Pull requested hash if defined, otherwise pull main branch.

  if(NOT DEFINED TARGET_SUPPORT_REPO_TAG OR TARGET_SUPPORT_REPO_TAG STREQUAL "")
    set(TARGET_SUPPORT_REPO_TAG "main")
  endif()

  FetchContent_Declare(
    target_support_package
    GIT_REPOSITORY ${TARGET_SUPPORT_REPO}
    GIT_TAG ${TARGET_SUPPORT_REPO_TAG}
    GIT_PROGRESS TRUE)

  FetchContent_MakeAvailable(target_support_package)

endif()

# coreMQTT (gives core_mqtt target)

FetchContent_Declare(
  core_mqtt
  GIT_REPOSITORY https://github.com/jasonpcarroll/coreMQTT.git
  GIT_TAG main
  GIT_PROGRESS TRUE)

FetchContent_MakeAvailable(core_mqtt)

# FreeRTOS-Kernel (gives freertos_kernel target) #

FetchContent_Declare(
  freertos_kernel
  GIT_REPOSITORY https://github.com/jasonpcarroll/FreeRTOS-Kernel.git
  GIT_TAG main
  GIT_PROGRESS TRUE)

FetchContent_MakeAvailable(freertos_kernel)

target_link_libraries(logging_freertos PUBLIC freertos_kernel)
target_link_libraries(core_mqtt PUBLIC logging_freertos)

target_link_libraries(tls_transport PUBLIC core_mqtt)
target_link_libraries(plaintext_transport PUBLIC core_mqtt)


# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# coreMQTT Demo (coremqtt_demo)

set(COREMQTT_DEMO_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/source/main.c)

set(COREMQTT_DEMO_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(coremqtt_demo ${COREMQTT_DEMO_SOURCES})

target_include_directories(coremqtt_demo PUBLIC ${COREMQTT_DEMO_INCLUDE_DIRS})

target_link_libraries(coremqtt_demo PUBLIC core_mqtt)

if(TARGET board_support_package)
  target_link_libraries(coremqtt_demo PUBLIC board_support_package)
endif()
