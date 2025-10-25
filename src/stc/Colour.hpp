/** \file */
#pragma once

#include <cstdint>
#include <ostream>
#include "Environment.hpp"

/**
 * Colour module. 
 *
 * Interfaces with `std::ostream`s to print colour and some other ANSI codes. 
 *
 * Example use:
 * ```cpp
 * // 4 bit
 * std::cout << stc::colour::fg<stc::colour::FourBitColour::RED> << "Whatever" << stc::colour::reset << std::endl;
 * // 8 bit
 * std::cout << stc::colour::fg<240> << "Whatever" << stc::colour::reset << std::endl;
 * // 24 bit
 * std::cout << stc::colour::fg<240, 0, 240> << "Whatever" << stc::colour::reset << std::endl;
 * ```
 * `::fg` and `::bg` have the exact same interface, but change foreground and background colours respectively. In
 * addition to colour, `stc::colour::use` exists, which takes an stc::colour::Typography. 
 *
 * Like typography, 8 bit colours are hard-coded in stc::colour::FourBitColour.
 *
 * ## General usability note
 *
 * Though very outside the scope of this module, do be aware of the usability of the thing you make when you involve
 * colour. Unless you go full TUI and control the background, the user can and will have themes that may not work with
 * the colours you use. Particularly if you hard-code black or white, you run the risk of picking a colour that
 * corresponds to the user's background colour, making your CLI app unusable.
 *
 * None of the colour types guarantee that your CLI app will be legible on all themes, but you do minimise the risk
 * significantly by sticking to just a few colours. If you do anything major with colour, you'll probably want to offer
 * the user a way to change the colours at an app level. Again, out of scope for the module, but this is not discussed
 * nearly often enough, so I might as well mention it.
 */
namespace stc::colour {

enum class FourBitColour {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,

    BRIGHT_BLACK = 90,
    BRIGHT_RED = 91,
    BRIGHT_GREEN = 92,
    BRIGHT_YELLOW = 93,
    BRIGHT_BLUE = 94,
    BRIGHT_MAGENTA = 95,
    BRIGHT_CYAN = 96,
    BRIGHT_WHITE = 97,
};

/**
 * Represents typographic elements.
 * This set is kept to a minimal mostly universally supported set of codes. This cuts out many technically supported
 * codes due to feature support, and this interface aims to make it more clear what is and isn't supported.
 *
 * Note that RESET is not included even though it is supported, because it has a standalone function for naming and
 * brevity purposes.
 *
 * \see https://en.wikipedia.org/wiki/ANSI_escape_code#Select_Graphic_Rendition_parameters
 */
enum class Typography {
    BOLD = 1,
    FAINT = 2,
ITALIC = 3,
    UNDERLINE = 4,
    SLOW_BLINK = 5,

    /**
     * Resets BOLD and FAINT
     */
    RESET_INTENSITY = 22,
    NO_ITALIC = 23,
    NO_UNDERLINE = 24,
    NO_BLINKING = 25
};  

namespace _detail {

const int FOREGROUND = 38;
const int BACKGROUND = 48;

// forced: iword defaults to 0
const int MODE_AUTO = 0;
const int MODE_FORCE = 1;


inline int getStreamConfigIdx() {
    static int idx = std::ios::xalloc();
    return idx;
}

template <typename CharT>
static bool shouldPrintColour(std::basic_ostream<CharT>& ss) {
    auto iword = ss.iword(getStreamConfigIdx());
    if (iword == MODE_FORCE) {
        return true;
    }

    return isCppStreamTTY(ss);
}

template <int Mode>
struct Colouriser {
    /**
     * \see stc::colour::FourBitColour
     * \see https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
     */
    template <FourBitColour Colour, typename CharT>
    static constexpr std::basic_ostream<CharT>& fourBit(std::basic_ostream<CharT>& stream) {
        if (shouldPrintColour(stream)) {
            stream << "\033["
                << (Mode == _detail::FOREGROUND ? static_cast<int>(Colour) : (static_cast<int>(Colour) + 10)) 
                << "m";
        }
        return stream;
    }

    /**
     * Note that unlike four bit colours, 8 bit colours use a full uint8_t (from 0 to 255), so the values themselves are
     * not named in an enum. 
     *
     * \see https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
     */
    template <uint8_t code, typename CharT>
    static constexpr std::basic_ostream<CharT>& eightBit(std::basic_ostream<CharT>& stream) {
        if (shouldPrintColour(stream)) {
            stream << "\033[" << Mode << ";5;" << code << "m";
        }
        return stream;
    }

    /**
     * Takes three template arguments corresponding to the red, green, and blue respectively, and works identically to
     * standard RGB colours. 
     *
     * Note that while the underlying ANSI guarantees the colour you provide is used, there's no guarantee it'll render
     * visibly everywhere. Remember that some people use light mode or theme variants that may not work with the colour
     * you've picked. You need to be extra aware of theming when using this function.
     *
     * \see https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit
     */
    template <uint8_t r, uint8_t g, uint8_t b, typename CharT>
    static constexpr std::basic_ostream<CharT>& truecolour(std::basic_ostream<CharT>& stream) {
        if (shouldPrintColour(stream)) {
            stream << "\033[" << Mode << ";2;" << r << ";" << g << ";" << b << "m";
        }
        return stream;
    }
};

}

using FgColour = _detail::Colouriser<_detail::FOREGROUND>;
using BgColour = _detail::Colouriser<_detail::BACKGROUND>;

template <Typography feature, typename CharT>
static constexpr std::basic_ostream<CharT>& use(std::basic_ostream<CharT>& stream) {
    if (shouldPrintColour(stream)) {
        stream << "\033[" << static_cast<int>(feature) << "m";
    }
    return stream;
}

/**
 * Can be used to force colouring to happen. By default, colouring is only applied if the stream is a TTY. If it isn't,
 * colours are disabled. This means if stdout/stderr is redirected, or if you write to a stringstream, no colours are
 * written. You can override this here, by using 
 * ```cpp
 * ss << stc::colour::force
 *     << // commands
 * ```
 *
 * If you force it, but then want to set it back into default mode, supply `stc::colour::force<false>` instead.
 */
template <bool val = true, typename CharT>
static constexpr std::basic_ostream<CharT>& force(std::basic_ostream<CharT>& stream) {
    stream.iword(_detail::getStreamConfigIdx()) = val ? _detail::MODE_FORCE : _detail::MODE_AUTO; 

    return stream;
}


template <typename CharT>
static constexpr std::basic_ostream<CharT>& reset(std::basic_ostream<CharT>& stream) {
    if (_detail::shouldPrintColour(stream)) {
        stream << "\033[0m";
    }
    return stream;
}

/**
 * Foreground colour for 4 bit colours.
 *
 * \copydoc _detail::Colouriser::fourBit
 */
template <FourBitColour Colour, typename CharT>
static constexpr std::basic_ostream<CharT>& fg(std::basic_ostream<CharT>& stream) {
    return FgColour::fourBit<Colour, CharT>(stream);
}

/**
 * Foreground colour for 8 bit colours.
 *
 * \copydoc _detail::Colouriser::eightBit
 */
template <uint8_t code, typename CharT>
static constexpr std::basic_ostream<CharT>& fg(std::basic_ostream<CharT>& stream) {
    return FgColour::eightBit<code, CharT>(stream);
}

/**
 * Foreground colour for 24 bit colours.
 *
 * \copydoc _detail::Colouriser::truecolour
 */
template <uint8_t r, uint8_t g, uint8_t b, typename CharT>
static constexpr std::basic_ostream<CharT>& fg(std::basic_ostream<CharT>& stream) {
    return FgColour::truecolour<r, g, b, CharT>(stream);
}

/**
 * Foreground colour for 4 bit colours.
 *
 * \copydoc _detail::Colouriser::fourBit
 */
template <FourBitColour Colour, typename CharT>
static constexpr std::basic_ostream<CharT>& bg(std::basic_ostream<CharT>& stream) {
    return BgColour::fourBit<Colour, CharT>(stream);
}

/**
 * Foreground colour for 8 bit colours.
 *
 * \copydoc _detail::Colouriser::eightBit
 */
template <uint8_t code, typename CharT>
static constexpr std::basic_ostream<CharT>& bg(std::basic_ostream<CharT>& stream) {
    return BgColour::eightBit<code, CharT>(stream);
}

/**
 * Foreground colour for 24 bit colours.
 *
 * \copydoc _detail::Colouriser::truecolour
 */
template <uint8_t r, uint8_t g, uint8_t b, typename CharT>
static constexpr std::basic_ostream<CharT>& bg(std::basic_ostream<CharT>& stream) {
    return BgColour::truecolour<r, g, b, CharT>(stream);
}


}
