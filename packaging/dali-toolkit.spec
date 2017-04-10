Name:       dali-toolkit
Summary:    The OpenGLES Canvas Core Library Toolkit
Version:    1.2.34
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-2-Clause and MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-toolkit.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(dali-core)
BuildRequires: gettext

# dali-toolkit only need to know the interfaces(APIs) of dali-adaptor(the devel package).
# It doesn't need to know which adaptor will be used by applications.
# Applications or dali-addon will decide which one they will use.
BuildRequires:  dali-adaptor-devel

#need libtzplatform-config for directory if tizen version is 3.x

%if 0%{?tizen_version_major} >= 3
BuildRequires:  pkgconfig(libtzplatform-config)
%endif

#############################
# profile setup
#############################

%if "%{tizen_version_major}" == "2" && 0%{?tizen_profile_name:1}
%define profile %{tizen_profile_name}
%endif

# tv
# %define dali_style_folder 1920x1080

# common
%define dali_style_folder 720x1280
# dali_style to be provided by build system as with dali_toolkit_profile or by passing --define 'dali_style 470x800' to the rpm build command

%if "%{?dali_style}"
  %define dali_style_folder %{dali_style}
%endif

%description
The OpenGLES Canvas Core Library Toolkit - a set of controls that provide
user interface functionality.

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%package profile_tv
Summary:    style files for Tizen TV (1920x1080)
Requires:   %{name} = %{version}-%{release}
%description profile_tv
dali-toolkit style files for Tizen TV (1920x1080)
%endif

##############################
# devel
##############################
%package devel
Summary:    Application development package for the OpenGLES Canvas toolkit
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}

%description devel
Application development package for the OpenGLES Canvas toolkit - headers and package config

##############################
# Preparation
##############################
%prep
%setup -q

#Use TZ_PATH when tizen version is 3.x

%if "%{tizen_version_major}" == "2"
%define dali_data_rw_dir            /opt/usr/share/dali/
%define dali_data_ro_dir            /usr/share/dali/
%else
%define dali_data_rw_dir            %TZ_SYS_SHARE/dali/
%define dali_data_ro_dir            %TZ_SYS_RO_SHARE/dali/
%endif

%define dali_toolkit_image_files    %{dali_data_ro_dir}/toolkit/images/
%define dali_toolkit_sound_files    %{dali_data_ro_dir}/toolkit/sounds/
%define dali_toolkit_style_files    %{dali_data_ro_dir}/toolkit/styles/
%define dev_include_path %{_includedir}

# PO
{
cd %{_builddir}/dali-toolkit-%{version}/dali-toolkit/po
for language in *.po
do
  language=${language%.po}
  msgfmt -o ${language}.mo ${language}.po
done
} &> /dev/null

##############################
# Build
##############################
%build
PREFIX="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections "
LDFLAGS+=" -Wl,--rpath=$PREFIX/lib -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%if 0%{?enable_coverage}
CXXFLAGS+=" --coverage "
LDFLAGS+=" --coverage "
%endif

libtoolize --force
cd %{_builddir}/dali-toolkit-%{version}/build/tizen
autoreconf --install
DALI_DATA_RW_DIR="%{dali_data_rw_dir}" ; export DALI_DATA_RW_DIR
DALI_DATA_RO_DIR="%{dali_data_ro_dir}" ; export DALI_DATA_RO_DIR

%configure --enable-profile=TIZEN \
           --with-style=%{dali_style_folder} \
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-i18n=yes

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}
pushd build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"


# PO
{
cd %{_builddir}/dali-toolkit-%{version}/dali-toolkit/po
for language in *.mo
do
  language=${language%.mo}
  mkdir -p %{buildroot}/%{_datadir}/locale/${language}/LC_MESSAGES/
  cp ${language}.mo %{buildroot}/%{_datadir}/locale/${language}/LC_MESSAGES/dali-toolkit.mo
done
} &> /dev/null
popd

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "mobile" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
mkdir -p %{buildroot}%{dali_toolkit_style_files}/1920x1080
# 720x1280/images/*.png files are exactly same with 1920x1080/images/*.png
# if not, copy them as same as dali-toolkit/styles/1920x1080/*.json
# cp dali-toolkit/styles/1920x1080/* %{buildroot}%{dali_toolkit_style_files}/1920x1080
cp dali-toolkit/styles/1920x1080/*.json %{buildroot}%{dali_toolkit_style_files}/1920x1080

# Do not let style package files be overwritten by the main package
#pushd %{buildroot}%{dali_toolkit_style_files}/1920x1080
#for FILE in *.json; do rm -f ../"${FILE}"; done
#popd

%endif

##############################
# Post Install
##############################
%post
/sbin/ldconfig
exit 0

##############################
# Post Uninstall
##############################
%postun
/sbin/ldconfig
exit 0

##############################
# Files in Binary Packages
##############################
%files
%if 0%{?enable_dali_smack_rules}
%manifest dali-toolkit.manifest-smack
%else
%manifest dali-toolkit.manifest
%endif
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so*
%{dali_toolkit_image_files}/*
%{dali_toolkit_sound_files}/*
%{dali_toolkit_style_files}/*
%license LICENSE
%{_datadir}/locale/*/LC_MESSAGES/*

%files devel
%defattr(-,root,root,-)
%{dev_include_path}/%{name}/*
%{_libdir}/pkgconfig/*.pc

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "mobile" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_tv
pushd %{dali_toolkit_style_files}/1920x1080
for FILE in *.json; do mv 1920x1080/"${FILE}" ../"${FILE}"; done
popd

%preun profile_tv
case "$1" in
  0)
    # This is an un-installation.
    pushd %{dali_toolkit_style_files}
	for FILE in *.json; do mv 1920x1080/"${FILE}"; done
	popd
  ;;
  1)
    # This is an upgrade.
    # Do nothing.
    :
  ;;
esac
%files profile_tv
%{dali_toolkit_style_files}/1920x1080/*
%endif
