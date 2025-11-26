add_library(Menu Menu.cpp ConcreteCommands.cpp)
target_include_directories(Menu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})