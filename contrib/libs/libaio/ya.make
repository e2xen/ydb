# Generated by devtools/yamaker from nixpkgs 22.05.

LIBRARY()

WITHOUT_LICENSE_TEXTS()

VERSION(0.3.113)

LICENSE(Service-Dll-Harness)

NO_RUNTIME()

IF (EXPORT_CMAKE)
    OPENSOURCE_EXPORT_REPLACEMENT(
        CMAKE
        AIO
        CMAKE_TARGET
        AIO::aio
    )
ELSEIF (USE_DYNAMIC_AIO)
    PEERDIR(
        contrib/libs/libaio/dynamic
    )
ELSE()
    PEERDIR(
        contrib/libs/libaio/static
    )
ENDIF()

END()

RECURSE(
    dynamic
    static
)
