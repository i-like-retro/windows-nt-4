#include <farversion.hpp>

#define PLUGIN_BUILD 9
#define PLUGIN_DESC L"Hello World Plugin for Far Manager"
#define PLUGIN_NAME L"HelloWorld"
#define PLUGIN_FILENAME L"HelloWorld.dll"
#define PLUGIN_AUTHOR FARCOMPANYNAME
#define PLUGIN_VERSION MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_REVISION,PLUGIN_BUILD,VS_RELEASE)