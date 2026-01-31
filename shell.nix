{
  mkShell,
  cmake,
  ninja,
  pkg-config,
  qt6,
  clang-tools,
  gdb,
  valgrind,
}:
mkShell {
  name = "dev-shell";

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    qt6.wrapQtAppsHook
    clang-tools
    gdb
    valgrind
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtdeclarative
    qt6.qttools
    qt6.qtsvg
  ];

  shellHook = ''
    echo "hola"
  '';
}
