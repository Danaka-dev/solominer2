#pragma once

// Copyright (c) 2021-2023 The XCoin developers
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

/**
 * Definitions and functions for interacting with console/terminal
 *
 * - ANSI escape codes (color...)
 */

#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H

//////////////////////////////////////////////////////////////////////////////
//! ANSI escape code

    //https://en.wikipedia.org/wiki/ANSI_escape_code

// C0 control code set, in ISO 6429 (ECMA-48)

#define C0_CSI              "\x1B["     // Control Sequence Introducer
#define C0_RESET            C0_CSI "0m"    // reset all attributes

//-- attributes
#define C0_NORMAL_          "0"
#define C0_BOLD_            "1"
#define C0_FAINT_           "2"
#define C0_ITALIC_          "3"     // not widely supported
#define C0_UNDERLINE_       "4"     // exist for Kitty, VTE, mintty, iTerm2 and Konsole
#define C0_BLINK_           "5"     // blinking less than 150/min
#define C0_FASTBLINK_       "6"     // blinking > 150/min, not widely supported

//-- colors (foreground)
#define C0_BLACK_           "30m"
#define C0_RED_             "31m"
#define C0_GREEN_           "32m"
#define C0_YELLOW_          "33m"
#define C0_BLUE_            "34m"
#define C0_MAGENTA_         "35m"
#define C0_CYAN_            "36m"
#define C0_WHITE_           "37m"

//-- colors (background)
#define C0_BLACK_BG_        "40m"
#define C0_RED_BG_          "41m"
#define C0_GREEN_BG_        "42m"
#define C0_YELLOW_BG_       "43m"
#define C0_BLUE_BG_         "44m"
#define C0_MAGENTA_BG_      "45m"
#define C0_CYAN_BG_         "46m"
#define C0_WHITE_BG_        "47m"

//-- wrapper (foreground)
#define CCOLOR_BLACK        C0_CSI C0_NORMAL_ C0_BLACK_
#define CCOLOR_RED          C0_CSI C0_NORMAL_ C0_RED_
#define CCOLOR_GREEN        C0_CSI C0_NORMAL_ C0_GREEN_
#define CCOLOR_YELLOW       C0_CSI C0_NORMAL_ C0_YELLOW_
#define CCOLOR_BLUE         C0_CSI C0_NORMAL_ C0_BLUE_
#define CCOLOR_MAGENTA      C0_CSI C0_NORMAL_ C0_MAGENTA_
#define CCOLOR_CYAN         C0_CSI C0_NORMAL_ C0_CYAN_
#define CCOLOR_WHITE        C0_CSI C0_NORMAL_ C0_WHITE_

#define CCOLOR_BRIGHT_BLACK        C0_CSI C0_BOLD_ C0_BLACK_
#define CCOLOR_BRIGHT_RED          C0_CSI C0_BOLD_ C0_RED_
#define CCOLOR_BRIGHT_GREEN        C0_CSI C0_BOLD_ C0_GREEN_
#define CCOLOR_BRIGHT_YELLOW       C0_CSI C0_BOLD_ C0_YELLOW_
#define CCOLOR_BRIGHT_BLUE         C0_CSI C0_BOLD_ C0_BLUE_
#define CCOLOR_BRIGHT_MAGENTA      C0_CSI C0_BOLD_ C0_MAGENTA_
#define CCOLOR_BRIGHT_CYAN         C0_CSI C0_BOLD_ C0_CYAN_
#define CCOLOR_BRIGHT_WHITE        C0_CSI C0_BOLD_ C0_WHITE_

#if defined(MAC_OSX)
 #define CCOLOR_GRAY        C0_CSI C0_NORMAL_ C0_WHITE_
#else
 #define CCOLOR_GRAY        C0_CSI C0_BOLD_ C0_BLACK_
#endif

//-- wrapper (background)
#define CCOLOR_BLACK_BG        C0_CSI C0_NORMAL_ C0_BLACK_BG_
#define CCOLOR_RED_BG          C0_CSI C0_NORMAL_ C0_RED_BG_
#define CCOLOR_GREEN_BG        C0_CSI C0_NORMAL_ C0_GREEN_BG_
#define CCOLOR_YELLOW_BG       C0_CSI C0_NORMAL_ C0_YELLOW_BG_
#define CCOLOR_BLUE_BG         C0_CSI C0_NORMAL_ C0_BLUE_BG_
#define CCOLOR_MAGENTA_BG      C0_CSI C0_NORMAL_ C0_MAGENTA_BG_
#define CCOLOR_CYAN_BG         C0_CSI C0_NORMAL_ C0_CYAN_BG_
#define CCOLOR_WHITE_BG        C0_CSI C0_NORMAL_ C0_WHITE_BG_

#define CCOLOR_BRIGHT_BLACK_BG        C0_CSI C0_BOLD_ C0_BLACK_BG_
#define CCOLOR_BRIGHT_RED_BG          C0_CSI C0_BOLD_ C0_RED_BG_
#define CCOLOR_BRIGHT_GREEN_BG        C0_CSI C0_BOLD_ C0_GREEN_BG_
#define CCOLOR_BRIGHT_YELLOW_BG       C0_CSI C0_BOLD_ C0_YELLOW_BG_
#define CCOLOR_BRIGHT_BLUE_BG         C0_CSI C0_BOLD_ C0_BLUE_BG_
#define CCOLOR_BRIGHT_MAGENTA_BG      C0_CSI C0_BOLD_ C0_MAGENTA_BG_
#define CCOLOR_BRIGHT_CYAN_BG         C0_CSI C0_BOLD_ C0_CYAN_BG_
#define CCOLOR_BRIGHT_WHITE_BG        C0_CSI C0_BOLD_ C0_WHITE_BG_

#if defined(MAC_OSX)
#define CCOLOR_GRAY_BG        C0_CSI C0_NORMAL_ C0_WHITE_BG
#else
#define CCOLOR_GRAY_BG        C0_CSI C0_BOLD_ C0_BLACK_BG_
#endif

//-- term
#define CCOLOR_NORMAL       C0_RESET

#endif //COMMON_CONSOLE_H