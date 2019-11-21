set(src
  tinycbor/cborparser.c
  tinycbor/cborvalidation.c
  extra/cbor_extra.c
  )

add_library(3rdparty_tinycbor ${src})
target_include_directories(3rdparty_tinycbor PUBLIC tinycbor extra)
