PROJECT(nvmath)

SET(MATH_SRCS
    nvmath.h
    Box.h Box.inl
    Color.h Color.inl
    Fitting.h Fitting.cpp
    Gamma.h Gamma.cpp
    Half.h Half.cpp
    Matrix.h Matrix.inl Matrix.cpp
    Plane.h Plane.inl Plane.cpp
    SphericalHarmonic.h SphericalHarmonic.cpp
    SimdVector.h SimdVector_SSE.h SimdVector_VE.h
    Vector.h Vector.inl)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(nvmath ${MATH_SRCS})

TARGET_LINK_LIBRARIES(nvmath ${LIBS} nvcore)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "${ROOT_FOLDER}/deps")

INSTALL(TARGETS nvmath
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
