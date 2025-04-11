cmake_minimum_required(VERSION 3.25)
project(tf_hwsys C)
set(CMAKE_C_STANDARD 17)
# Add the libraries

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_C_FLAGS_DEBUG -g)


add_library(i2c_module STATIC
    lib/i2c_lib/i2c_module.c
    lib/i2c_lib/i2c_module.h
)

add_library(events_module STATIC
    lib/events_lib/events_handlers.c
    lib/events_lib/events_handlers.h
)

add_library(gpio_module STATIC
    lib/gpio_lib/gpio_module.c
    lib/gpio_lib/gpio_module.h
)

add_library(utils_module STATIC
    lib/utils_lib/utils_module.c
    lib/utils_lib/utils_module.h
)

add_library(ubus_module STATIC
    lib/ubus_lib/ubus_module.c
    lib/ubus_lib/ubus_module.h
)

# Add the include directories
target_include_directories(i2c_module    PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(events_module  PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(gpio_module   PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(utils_module  PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(ubus_module  PUBLIC ${PROJECT_SOURCE_DIR}/lib)

find_path(uci_include_dir NAMES uci.h)
find_path(ubus_include_dir NAMES libubus.h)
find_path(ubox_include_dir NAMES libubox/usock.h)

include_directories(${ubox_include_dir})
include_directories(${ubus_include_dir})
include_directories(${uci_include_dir})


# Create the project_2 executable
add_executable(tf_hwsysd
        hwsysd.c
        hwsysd.h
        global_include.h
        config.c
        config.h
)

target_link_libraries(tf_hwsysd
        i2c_module
        events_module
        gpio_module
        utils_module
        ubus_module
        rt
        gpiod
        ubox
        ubus
        uci)
INSTALL(TARGETS tf_hwsysd
        RUNTIME DESTINATION /usr/bin/
)

