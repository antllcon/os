add_library(Menu Menu.cpp ConcreteCommands.cpp)
target_include_directories(Menu PUBLIC .. BTree)