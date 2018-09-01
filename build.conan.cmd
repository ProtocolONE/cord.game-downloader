set PackageName=GameDownloader/1.0.0@common/stable
conan install . -pr msvcprofile
conan create . %PackageName% -pr msvcprofile

conan install . -pr msvcprofiled
conan create . %PackageName% -pr msvcprofiled

@rem conan test CoreTest Core/1.0.0@common/stable -pr msvcprofile
conan upload %PackageName% --all -r=p1
