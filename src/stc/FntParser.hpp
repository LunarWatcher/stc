/** \file
 *
 *
 * Defines a parser for .fnt files. Note that .fnt here refers to a metadata format used alongside .png, and not the
 * binary image format.
 *
 * \deprecated  This module is no longer maintained, as I have switched to allegro, which has built-in .ttf parsing. That
 *              and I cannot find where the .fnt files came from, so I doubt it's a proper format.
 */
#pragma once

#include "StdFix.hpp"

#include <any>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

namespace stc {

namespace FntParser {

[[deprecated("FntParser is no longer maintained, and should not be used.")]]
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

[[deprecated("FntParser is no longer maintained, and should not be used.")]]
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

[[deprecated("FntParser is no longer maintained, and should not be used.")]]
inline std::vector<float> generateUVCoords(int atlasWidth, int atlasHeight, const FntCharInfo& chr) {
    auto x = (float) chr.x;
    auto y = (float) chr.y;
    auto width = (float) chr.width;
    auto height = (float) chr.height;

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

[[deprecated("FntParser is no longer maintained, and should not be used.")]]
inline std::map<std::string, std::any> parseLine(const std::string& line) {
    std::map<std::string, std::any> keys;

    size_t i = line.find(' ') + 1;
    do {
        // le sigh
        if (line[i] == ' ') {
            ++i;
            continue;
        }
        auto eq = line.find('=', i),
            sp = line.find(' ', i);
        if (sp == std::string::npos) sp = line.size();


        auto fragment = line.substr(i, sp - i);
        auto k = fragment.substr(0, eq - i);
        auto v = fragment.substr(eq - i + 1, sp - eq);

        std::any value;
        if (v.find('"') != std::string::npos) {
            if (v == "\"\"") value = std::string("");
            else value = v.substr(1, v.size() - 2);
        } else if (v.find(',') != std::string::npos) {
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
        keys[k] = value;
        i = sp + 1;
    } while (i < line.size());
    return keys;
}

[[deprecated("FntParser is no longer maintained, and should not be used.")]]
inline FntInfo loadAndParseFnt(const std::string& fileName) {
    if (!std::filesystem::exists(fileName)) {
        throw std::runtime_error("File doesn't exist");
    }

    std::filesystem::path p(fileName);
    std::ifstream f{p};

    if (!f.is_open()) {
        throw std::runtime_error("File not found");
    }

    FntInfo info;

    std::string buff;

    while (stc::StdFix::getline(f, buff)) {
        auto vars = parseLine(buff);

        if (buff.starts_with("info")) {
            info.faceName = std::any_cast<std::string>(vars.at("face"));
            info.size = std::any_cast<int>(vars.at("size"));
            info.bold = std::any_cast<int>(vars.at("bold"));
            info.italic = std::any_cast<int>(vars.at("italic"));
            info.unicode = std::any_cast<int>(vars.at("unicode"));
            info.smooth = std::any_cast<int>(vars.at("smooth"));
            info.antiAliasing = std::any_cast<int>(vars.at("aa"));
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
            info.base = std::any_cast<int>(vars.at("base"));
            info.scaleW = std::any_cast<int>(vars.at("scaleW"));
            info.scaleH = std::any_cast<int>(vars.at("scaleH"));

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

            chr.uvCoordinates = generateUVCoords(
                (int) info.scaleW,
                (int) info.scaleH,
                chr
            );

            info.characters[chr.id] = chr;
        }
            
    }
    
    return info;
}

}

}
