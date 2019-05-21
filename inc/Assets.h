#ifndef __ASSETS_H__
#define __ASSETS_H__

#include <map>
#include <string>
#include <dali-toolkit/dali-toolkit.h>
#include "obj-loader.h"

class Assets
{
    public:
        static void Init();
        static void LoadAllTextures();
        static void LoadAllObjs();
        static bool GetObj(std::string name, ObjLoader &buff);
        static bool GetTexture(std::string name, Dali::Texture &buff);

    private:
        static std::map<std::string, ObjLoader> mObjs;
        static std::map<std::string, Dali::PixelData> mImgs;
};

#endif