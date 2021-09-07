COCOS_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
COCOS_ROOT=$COCOS_ROOT/..

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac
echo ${machine}

if [ ${machine} = "Mac" ]; then
     NUM_OF_CORES=`getconf _NPROCESSORS_ONLN`
     echo "Compiling MacOSX ... "
     cd  $COCOS_ROOT/templates/mac
     mkdir -p build-mac/proj
     touch build-mac/proj/cfg.cmake
     echo "set(CC_USE_VULKAN OFF)" >> build-mac/proj/cfg.cmake
     echo "set(CC_USE_GLES2 OFF)" >> build-mac/proj/cfg.cmake
     echo "set(CC_USE_METAL ON)" >> build-mac/proj/cfg.cmake
     echo "set(USE_WEBSOCKET_SERVER OFF)" >> build-mac/proj/cfg.cmake
     mkdir build-mac/assets

     RES_DIR=$COCOS_ROOT/templates/mac/build-mac
     cd build-mac
     cmake ../ -GXcode -DCC_USE_GLES3=ON -DCMAKE_OSX_ARCHITECTURES=x86_64 -DRES_DIR=$RES_DIR -DCOCOS_X_PATH=$COCOS_ROOT
else
     cd $COCOS_ROOT/templates/windows
     mkdir -p build-win32/proj
     touch build-win32/proj/cfg.cmake
     echo "set(CC_USE_GLES3 ON)" >> build-win32/proj/cfg.cmake
     echo "set(CC_USE_VULKAN ON)" >> build-win32/proj/cfg.cmake
     echo "set(CC_USE_GLES2 ON)" >> build-win32/proj/cfg.cmake
     echo "set(USE_WEBSOCKET_SERVER ON)" >> build-win32/proj/cfg.cmake
     mkdir build-win32/assets
     cd build-win32
     RES_DIR=${GITHUB_WORKSPACE//\\//}/templates/windows/build-win32
     cmake ../ -G"Visual Studio 16 2019" -DRES_DIR=$RES_DIR -DCOCOS_X_PATH=$COCOS_ROOT -Awin32
fi