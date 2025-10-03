set(COMMON_SOURCES
    main.cpp
    ProcessPrinter/ProcessPrinter.cpp
)

if(WIN32)
    set(PLATFORM_SOURCES
        ScopedHandle/ScopedHandle.cpp
        ProcessScanner/ProcessScannerWin.cpp
    )

elseif(UNIX)
    set(PLATFORM_SOURCES
        ScopedResource/ScopedResource.cpp
        ProcessScanner/ProcessScannerLinux.cpp
    )
endif()

add_executable(ListProcess
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)

target_include_directories(ListProcess PRIVATE
    .
    ProcessPrinter
    ProcessScanner
)

if(WIN32)
    target_include_directories(ListProcess PRIVATE ScopedHandle)
elseif(UNIX)
    target_include_directories(ListProcess PRIVATE ScopedResource)
endif()


# add_executable(ListProccess 
  #  main.cpp 
   # ScopedHandle/ScopedHandle.cpp
    #ProcessScanner/ProcessScanner.cpp
    #ProcessPrinter/ProcessPrinter.cpp
#)

#target_include_directories(ListProccess PRIVATE
 #   .
  #  ScopedHandle
   # ProcessScanner
    #ProcessPrinter
#)

