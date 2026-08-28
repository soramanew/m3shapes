{
  lib,
  stdenv,
  cmake,
  ninja,
  qt6,
  debug ? false,
  withExamples ? false,
}: let
  version = "1.0.0";
in
  stdenv.mkDerivation {
    inherit version;
    pname = "m3shapes${lib.optionalString debug "-debug"}";

    src = lib.fileset.toSource {
      root = ./..;
      fileset = lib.fileset.unions [
        ./../CMakeLists.txt
        ./../src
        ./../examples
      ];
    };

    nativeBuildInputs = [cmake ninja];
    buildInputs = [qt6.qtbase qt6.qtdeclarative qt6.qtshadertools];

    dontWrapQtApps = true;

    cmakeBuildType =
      if debug
      then "Debug"
      else "RelWithDebInfo";

    cmakeFlags = [
      (lib.cmakeFeature "INSTALL_QMLDIR" qt6.qtbase.qtQmlPrefix)
      (lib.cmakeBool "M3SHAPES_BUILD_EXAMPLES" withExamples)
    ];

    dontStrip = debug;

    meta = {
      description = "A Qt6 library for Material 3 Expressive shapes";
      homepage = "https://github.com/soramanew/m3shapes";
      license = lib.licenses.asl20;
      platforms = lib.platforms.linux;
    };
  }
