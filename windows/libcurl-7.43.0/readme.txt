Used msys2 to compile libcurl with slighly modified PKGBUILD to
avoid strtok_r which seems to be introduced in recent mingw-w64:

  curl_disallow_strtok_r=yes ../${_realname}-${pkgver}/configure \
    --prefix=${MINGW_PREFIX} \
    --build=${MINGW_CHOST} \
    --host=${MINGW_CHOST} \
    --target=${MINGW_CHOST} \
    --without-random \
    --enable-static \
    --enable-shared \
    --enable-sspi \
    --enable-ipv6 \
    "${_variant_config[@]}" \
    "${extra_config[@]}"
  make

All static dependency libraries were grabbed from msys2 repos except for
librtmp.a which I kept from the previous version (from the libcurl website).
Note that libintl is part of gettext.

