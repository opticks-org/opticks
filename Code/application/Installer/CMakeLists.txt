set(HEADER_FILES
    Aeb.h
    AebId.h
    AebIo.h
    AebPlatform.h
    AebRequirement.h
    AebVersion.h
    ExtensionListModel.h
    InstallerServices.h
    InstallerServicesImp.h
    ReferenceCountDatabase.h
    Transaction.h
    TransactionLog.h
    ZipResource.h
)
set (MOC_HEADERS
    ExtensionListDelegate.h
    ExtensionListDialog.h
    ExtensionListItem.h
    InstallWizard.h
    InstallWizardInfoPage.h
    InstallWizardLicensePage.h
)
set(SOURCE_FILES
    Aeb.cpp
    AebId.cpp
    AebIo.cpp
    AebPlatform.cpp
    AebRequirement.cpp
    AebVersion.cpp
    ExtensionListDelegate.cpp
    ExtensionListDialog.cpp
    ExtensionListItem.cpp
    InstallerServicesImp.cpp
    InstallWizard.cpp
    InstallWizardInfoPage.cpp
    InstallWizardLicensePage.cpp
    ReferenceCountDatabase.cpp
    Transaction.cpp
    TransactionLog.cpp
)
set(ALL_SOURCES )
if(ONE_OPTICKS_LIB)
    set_property(DIRECTORY PROPERTY ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES})
    set_property(DIRECTORY PROPERTY MOC_HEADERS ${MOC_HEADERS})
else()
    qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
    source_group(moc FILES ${MOC_OUTFILES})
    add_library(Installer STATIC ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
endif()

