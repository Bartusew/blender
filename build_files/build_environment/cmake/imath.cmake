# SPDX-FileCopyrightText: 2022 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

set(IMATH_EXTRA_ARGS
  -DBUILD_SHARED_LIBS=ON
  -DBUILD_TESTING=OFF
  -DIMATH_LIB_SUFFIX=${OPENEXR_VERSION_BUILD_POSTFIX}
)

ExternalProject_Add(external_imath
  URL file://${PACKAGE_DIR}/${IMATH_FILE}
  DOWNLOAD_DIR ${DOWNLOAD_DIR}
  URL_HASH ${IMATH_HASH_TYPE}=${IMATH_HASH}
  PREFIX ${BUILD_DIR}/imath
  CMAKE_GENERATOR ${PLATFORM_ALT_GENERATOR}

  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${LIBDIR}/imath
    ${DEFAULT_CMAKE_FLAGS}
    ${IMATH_EXTRA_ARGS}

  PATCH_COMMAND ${PATCH_CMD} -p 1 -d
    ${BUILD_DIR}/imath/src/external_imath <
    ${PATCH_DIR}/imath_358.diff

  INSTALL_DIR ${LIBDIR}/imath
)

if(WIN32)
  ExternalProject_Add_Step(external_imath after_install
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${LIBDIR}/imath/lib
      ${HARVEST_TARGET}/imath/lib
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${LIBDIR}/imath/include
      ${HARVEST_TARGET}/imath/include
    COMMAND ${CMAKE_COMMAND} -E copy
      ${LIBDIR}/imath/bin/imath${OPENEXR_VERSION_POSTFIX}.dll
      ${HARVEST_TARGET}/imath/bin/imath${OPENEXR_VERSION_POSTFIX}.dll

    DEPENDEES install
  )
else()
  harvest(external_imath imath/include imath/include "*.h")
  harvest_rpath_lib(external_imath imath/lib imath/lib "*${SHAREDLIBEXT}*")
endif()
