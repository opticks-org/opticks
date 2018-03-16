set(HEADER_FILES
    ConnectionParameters.h
    Feature.h
    FeatureClassProperties.h
    FormatStringProcessor.h
)
add_custom_target(ArcProxyLib ALL SOURCES ${HEADER_FILES})
