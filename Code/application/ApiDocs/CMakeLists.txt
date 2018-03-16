set(DOCUMENTATION_FILES
    aeb.html
    aeb.txt
    aebl.html
    contextmenus.txt
    coordinate_systems.txt
    dependencies.txt
    deploymentfiles.txt
    howto_buildenvironment.txt
    howto_compileplugins.txt
    howto_customdraganddrop.txt
    howto_importer.txt
    howto_sessionsave.txt
    howto_writeinstaller.txt
    iceformat.txt
    mainpage.txt
    overviews.txt
    plugins.txt
    plugintutorial_building.txt
    plugintutorial_overview.txt
    plugintutorial1.txt
    plugintutorial2.txt
    plugintutorial3.txt
    plugintutorial4.txt
    plugintutorial5.txt
    settings_macros.txt
    stringutilities_types.txt
    suppressiblemsgdlg.txt
    typeconverter_types.txt
    upgrade_sdk.txt
    usingimporterresource.txt
    wavelengthfileformat.txt
)
set(DOXYGEN_CONFIG
    "${Opticks_BINARY_DIR}/ApiDocs/application.dox"	
    trailer.html
)
file(TO_NATIVE_PATH "${Opticks_BINARY_DIR}/DoxygenOutput" DOXYGEN_OUTPUT_DIR) 
set(GENERATE_CHM NO)
if(MsHelpCompiler_FOUND)
    set(GENERATE_CHM YES)
endif()
set(CHM_NAME "OpticksSDK-${Opticks_VERSION}.chm")
configure_file("${Opticks_SOURCE_DIR}/cmake/Templates/application.dox" .)
if(DOXYGEN_FOUND AND DOXYGEN_DOT_PATH)
    add_custom_command(OUTPUT ${Opticks_BINARY_DIR}/DoxygenOutput/html/index.html COMMAND "${DOXYGEN_EXECUTABLE}" "${Opticks_BINARY_DIR}/ApiDocs/application.dox" VERBATIM)
    add_custom_target(ApiDocs ALL SOURCES ${Opticks_BINARY_DIR}/DoxygenOutput/html/index.html ${DOCUMENTATION_FILES} ${DOXYGEN_CONFIG})
    source_group(Documentation FILES ${DOCUMENTATION_FILES})
    source_group("Doxygen Configuration" FILES ${DOXYGEN_CONFIG})
    set_target_properties(ApiDocs PROPERTIES EXCLUDE_FROM_ALL 1)
endif()
