set (HEADER_FILES
    BatchWizardExecutor.h
    WizardExecutor.h
)
set (SOURCE_FILES
    BatchWizardExecutor.cpp
    ModuleManager.cpp
    WizardExecutor.cpp
) 
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Opticks_SOURCE_DIR}/Wizard)
add_library(WizardExecutor SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(WizardExecutor
    ${Opticks_LIBRARIES}
    ${Opticks_WizardPrivate_LIBRARY} 
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS WizardExecutor RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS WizardExecutor RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS WizardExecutor LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS WizardExecutor LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
