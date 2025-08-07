# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD waveshare_rp2350_plus_4mb CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
set(PICO_EXTRAS_PATH ${USERHOME}/src/pico/pico-extras)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(numbers_pwm C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(numbers_pwm
    numbers_pwm.cpp
    audio_player.cpp
    number_to_speech.cpp
)

pico_set_program_name(numbers_pwm "numbers_pwm")
pico_set_program_version(numbers_pwm "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(numbers_pwm 0)
pico_enable_stdio_usb(numbers_pwm 1)

# Add preprocessor definitions
target_compile_definitions(numbers_pwm PRIVATE 
    # We are hardcoding rate and format for this fixed-function program
    AUDIO_SAMPLE_RATE=22050
    AUDIO_BUFFER_FORMAT=AUDIO_BUFFER_FORMAT_PCM_S16
    # This is the bottom left pin on the Seeed XIAO 2350 board
    AUDIO_PWM_PIN=0
    # ~23ms @ 22050Hz
    AUDIO_BUFFER_SAMPLE_LENGTH=512
    AUDIO_BUFFER_COUNT=5
)

# Add the standard library to the build
target_link_libraries(numbers_pwm
    pico_audio
    pico_audio_pwm
    hardware_adc
    hardware_gpio
    pico_stdlib
)

# Add the standard include files to the build
target_include_directories(numbers_pwm PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(numbers_pwm)
