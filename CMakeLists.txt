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
set(PICO_BOARD pico CACHE STRING "Board type")

# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE "Release")
#set(TINYUSB_DEBUG_LEVEL 0)

# DEBUG enables stdio output to the first CDC UART interface.
# (You can send text / data with printf() (or alike) to a terminal emulator
# that is running on the computer that the dJ is connected to.)
# To use it, in dirtyJTAGConfig.h, CDC_UART_INTF_COUNT should be > 0.
# Open up a terminal emulator to see the output.
# 0 - no debug
# 1 - debug
# Other levels are reserved for future use.
set(PDJ_DEBUG_LEVEL 0)
# Pass this to project files as well.
add_compile_definitions(PDJ_DEBUG_LEVEL=${PDJ_DEBUG_LEVEL})

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
#set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
#set(PICO_SDK_VERSION_REVISION 0) #hack to SDK is versioned as 2.1.1, and therefore currently requires a pioasm version of 2.1.1

project(dirtyJtag C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(dirtyJtag
		dirtyJtag.c
		usb_descriptors.c
		pio_jtag.c
        cdc_uart.c
		get_serial.c
		cmd.c
        led.c
        util.c
)

target_include_directories(dirtyJtag PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_compile_definitions(dirtyJtag PRIVATE SYS_CLK_MHZ=200)

pico_generate_pio_header(dirtyJtag ${CMAKE_CURRENT_LIST_DIR}/jtag.pio)

pico_set_program_name(dirtyJtag "dirtyJtag")
pico_set_program_version(dirtyJtag "0.1")

#pico_enable_stdio_uart(dirtyJtag 0)
#pico_enable_stdio_usb(dirtyJtag 0)

# Add the standard library to the build
target_link_libraries(dirtyJtag PRIVATE pico_stdlib)

# Add any user requested libraries
target_link_libraries(dirtyJtag PRIVATE
    hardware_pio
	hardware_dma
	pico_unique_id
	tinyusb_device
	tinyusb_board
	pico_multicore
)

# Allow debug messages to go through USB CDC UART interface.
if (${PDJ_DEBUG_LEVEL})
    pico_enable_stdio_usb(dirtyJtag 1)
endif()

pico_add_extra_outputs(dirtyJtag)

