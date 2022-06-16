#pragma once

#include "FS.hpp"

#include <any>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

namespace stc {

namespace FntParser {

typedef struct {
    int id;
    int x,
        y,
        width,
        height,
        xOffset,
        yOffset,
        xAdvance,
        page,
        channel;

    std::vector<float> uvCoordinates;
} FntCharInfo;

typedef struct {
    std::string faceName;
    int size; // In what unit?
    size_t scaleW, scaleH;

    bool bold,
        italic,
        unicode,
        smooth,
        antiAliasing;

    size_t padLeft, padTop, padRight, padBottom;
    size_t spaceLeft, spaceTop;

    int lineHeight, base;

    // Loading the files is left as an exercise to the reader.
    std::vector<std::string> pages;

    std::map<int, FntCharInfo> characters;

} FntInfo;

inline std::vector<float> generateUVCoords(int atlasWidth, int atlasHeight, const FntCharInfo& chr) {
    float x = chr.x;
    float y = chr.y;
    float width = chr.width;
    float height = chr.height;

    float reX = ((float) x) / atlasWidth;
    float reY = ((float) y) / atlasHeight;
    float newX = ((float) x + width) / atlasWidth;
    float newY = ((float) y + height) / atlasHeight;
    // reX += 1.0 / DIMENSIONS;
    // reY += 1.0 / DIMENSIONS;
    // newX -= 1.0 / DIMENSIONS;
    // newY -= 1.0 / DIMENSIONS;

    return {
        reX, reY, // 0
        reX, newY, // 1
        newX, reY, // 3
        newX, reY, // 3
        reX, newY, // 1
        newX, newY, // 2
    };
}

inline std::map<std::string, std::any> parseLine(const std::string& line) {

    std::map<std::string, std::any> keys;

    size_t i = line.find(' ');
    do {
        auto eq = line.find('=', i),
             sp = line.find(' ', i);

        auto fragment = line.substr(i, i - sp);
        std::cout << fragment << std::endl;
        auto k = line.substr(i, i - eq);
        std::cout << k << std::endl;
        auto v = line.substr(eq, sp - eq);
        std::cout << v << std::endl;
        std::any value;
        if (v.find('"')) {
            if (v == "\"\"") value = std::string("");
            else value = v.substr(1, v.size() - 2);
        } else if (v.find(',')) {
            std::vector<int> tmp;

            std::string buff;
            std::stringstream ss(v);
            while (std::getline(ss, buff, ',')) {
                tmp.push_back(std::stoi(buff));
            }
            
            value = tmp;
        } else {
            value = std::stoi(v);
        }
        keys[k] = v;
    } while (i < line.size());
    return keys;
}

inline FntInfo loadAndParseFnt(const std::string& fileName) {
    if (!fs::exists(fileName)) {
        throw std::runtime_error("File doesn't exist");
    }

    fs::path p(fileName);
    std::ifstream f(p);

    FntInfo info;

    std::string buff;
    while (std::getline(f, buff)) {
        auto vars = parseLine(buff);

        if (buff.starts_with("info")) {
            info.faceName = std::any_cast<std::string>(vars.at("face"));
            info.size = std::any_cast<int>(vars.at("size"));
            info.bold = std::any_cast<bool>(vars.at("bold"));
            info.italic = std::any_cast<bool>(vars.at("italic"));
            info.unicode = std::any_cast<bool>(vars.at("unicode"));
            info.smooth = std::any_cast<bool>(vars.at("smooth"));
            info.antiAliasing = std::any_cast<bool>(vars.at("aa"));
            auto padding = std::any_cast<std::vector<int>>(vars.at("padding"));

            info.padTop = padding.at(0);
            info.padRight = padding.at(1);
            info.padBottom = padding.at(2);
            info.padLeft = padding.at(3);


            auto spacing = std::any_cast<std::vector<int>>(vars.at("spacing"));
            info.spaceLeft = spacing.at(0);
            info.spaceTop = spacing.at(1);
        } else if (buff.starts_with("common")) {
            info.lineHeight = std::any_cast<int>(vars.at("lineHeight"));
            info.base = std::any_cast<int>("base");
            info.scaleW = std::any_cast<int>("scaleW");
            info.scaleH = std::any_cast<int>("scaleH");

        } else if (buff.starts_with("page")) {
            info.pages.push_back(std::any_cast<std::string>(vars.at("file")));
        } else if (buff.starts_with("chars")) {
            // ignore
        } else if (buff.starts_with("char")) {
            FntCharInfo chr;
            chr.id = std::any_cast<int>(vars.at("id"));
            chr.x = std::any_cast<int>(vars.at("x"));
            chr.y = std::any_cast<int>(vars.at("y"));
            chr.width = std::any_cast<int>(vars.at("width"));
            chr.height = std::any_cast<int>(vars.at("height"));
            chr.xOffset = std::any_cast<int>(vars.at("xoffset"));
            chr.yOffset = std::any_cast<int>(vars.at("yoffset"));
            chr.xAdvance = std::any_cast<int>(vars.at("xadvance"));
            chr.page = std::any_cast<int>(vars.at("page"));

            chr.uvCoordinates = generateUVCoords(info.scaleW, info.scaleH, chr);

            info.characters[chr.id] = chr;
        }
            
    }
    
    return info;
}

}

}
