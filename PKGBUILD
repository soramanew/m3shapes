# m3shapes

# Maintainer: Soramane <soramane32 at gmail dot com>

_pkgname='m3shapes'
pkgname="$_pkgname-git"
pkgver=r6.252af33
pkgrel=1
pkgdesc='A QT plugin port of the androidx shapes library'
arch=('x86_64')
url='https://github.com/soramanew/m3shapes'
depends=('qt6-declarative')
makedepends=('git' 'cmake' 'ninja')
provides=($_pkgname)
conflicts=($_pkgname)
source=("$pkgname::git+$url.git")
sha256sums=('SKIP')

pkgver() {
    cd "${srcdir}/${pkgname}"
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
}

build() {
    cd "${srcdir}/${pkgname}"

    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/
    cmake --build build
}

package() {
    cd "${srcdir}/${pkgname}"

    DESTDIR="$pkgdir" cmake --install build
    install -Dm644 LICENSE "$pkgdir"/usr/share/licenses/$_pkgname/LICENSE
}
