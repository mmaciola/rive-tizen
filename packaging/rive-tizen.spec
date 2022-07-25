Name:       rive-tizen
Summary:    Rive Animation skia Runtime Engine
Version:    0.2.0
Release:    1
Group:      Graphics System/Rendering Engine
License:    MIT
URL:        https://github.com/rive-app/rive-tizen
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig
BuildRequires:  gn
BuildRequires:  meson
BuildRequires:  ninja
Requires(post): /sbin/ldconfig

%description
Rive Animation SKIA Runtime Engine

%package devel
Summary:    Rive Animation SKIA Runtime Engine
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Rive Animation SKIA Runtime Engine (devel)

%prep
%setup -q

%build
export DESTDIR=%{buildroot}

./make_skia.sh cpu tizen

meson setup \
      --prefix /usr \
      --libdir %{_libdir} \
      builddir 2>&1
ninja \
      -C builddir \
      -j %(echo "`/usr/bin/getconf _NPROCESSORS_ONLN`")

%install

export DESTDIR=%{buildroot}

ninja -C builddir install
cp -rf %{_builddir}/%{name}-%{version}/submodule/skia/out/Shared/libskia.so %{buildroot}/usr/lib

%files
%defattr(-,root,root,-)
%{_libdir}/librive-tizen.so.*
%{_libdir}/libskia.so
%manifest packaging/rive-tizen.manifest

%files devel
%defattr(-,root,root,-)
%{_includedir}/*.hpp
# Rive-cpp related
%{_includedir}/rive/*.hpp
%{_includedir}/rive/animation/*.hpp
%{_includedir}/rive/assets/*.hpp
%{_includedir}/rive/bones/*.hpp
%{_includedir}/rive/constraints/*.hpp
%{_includedir}/rive/core/*.hpp
%{_includedir}/rive/core/field_types/*.hpp
%{_includedir}/rive/generated/animation/*.hpp
%{_includedir}/rive/generated/assets/*.hpp
%{_includedir}/rive/generated/bones/*.hpp
%{_includedir}/rive/generated/constraints/*.hpp
%{_includedir}/rive/generated/shapes/paint/*.hpp
%{_includedir}/rive/generated/shapes/*.hpp
%{_includedir}/rive/generated/*.hpp
%{_includedir}/rive/importers/*.hpp
%{_includedir}/rive/math/*.hpp
%{_includedir}/rive/shapes/*.hpp
%{_includedir}/rive/shapes/paint/*.hpp
# Skia related
%{_includedir}/skia/*.h
%{_includedir}/include/core/*.h
%{_includedir}/include/config/*.h
%{_includedir}/include/private/*.h
%{_includedir}/include/gpu/*.h
%{_includedir}/include/gpu/gl/*.h
%{_libdir}/librive-tizen.so
%{_libdir}/libskia.so
%{_libdir}/pkgconfig/rive-tizen.pc