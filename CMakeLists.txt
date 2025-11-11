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

target_link_libraries(transport PUBLIC core_mqtt)

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# coreMQTT Demo (coremqtt_demo)

set(COREMQTT_DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/source/main.c)

set(COREMQTT_DEMO_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(coremqtt_demo ${COREMQTT_DEMO_SOURCES})

target_include_directories(coremqtt_demo PUBLIC ${COREMQTT_DEMO_INCLUDE_DIRS})

target_link_libraries(coremqtt_demo PUBLIC core_mqtt transport clock)

if(TARGET board_support_package)
  target_link_libraries(coremqtt_demo PUBLIC board_support_package)
endif()

# Get everything needed to establish MQTT connection for QuickConnect

file(DOWNLOAD https://quickconnect.freertos.aws.com/settings.json
     "${CMAKE_BINARY_DIR}/settings.json")

file(READ "${CMAKE_BINARY_DIR}/settings.json" QUICKCONNECT_SETTINGS_JSON)

string(JSON AWS_IOT_ENDPOINT GET "${QUICKCONNECT_SETTINGS_JSON}" iotEndpoint)

string(JSON AWS_API_ENDPOINT GET "${QUICKCONNECT_SETTINGS_JSON}" apiEndpoint)

message("AWS_IOT_ENDPOINT: ${AWS_IOT_ENDPOINT}")

message("AWS_API_ENDPOINT: ${AWS_API_ENDPOINT}")

string(RANDOM LENGTH 32 THING_NAME)

message("THING NAME: ${THING_NAME}")

execute_process(
  COMMAND openssl req -nodes -newkey rsa:2048 -keyout qc_private_key.key -out
          signing_request.csr -subj "/CN=${THING_NAME}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

file(READ "${CMAKE_BINARY_DIR}/signing_request.csr" SIGNING_REQUEST)
string(REPLACE "\n" "\\n" SIGNING_REQUEST "${SIGNING_REQUEST}")

execute_process(
  COMMAND
    curl -X POST -H "Content-Type: application/json" -d
    "{\"csr\": \"${SIGNING_REQUEST}\"}"
    "${AWS_API_ENDPOINT}certificate/${THING_NAME}"
  OUTPUT_VARIABLE CERT)

string(JSON CERT GET ${CERT} cert)
file(WRITE "${CMAKE_BINARY_DIR}/qc_cert.crt" "${CERT}")

file(DOWNLOAD https://www.amazontrust.com/repository/AmazonRootCA1.pem "${CMAKE_BINARY_DIR}/AmazonRootCA1.pem")

target_compile_definitions(coremqtt_demo PUBLIC CLIENT_CERT="${CMAKE_BINARY_DIR}/qc_cert.crt" PRIVATE_KEY="${CMAKE_BINARY_DIR}/qc_private_key.key" ROOT_CA="${CMAKE_BINARY_DIR}/AmazonRootCA1.pem" MQTT_BROKER_ENDPOINT="${AWS_IOT_ENDPOINT}" THING_NAME="${THING_NAME}")
