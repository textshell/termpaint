#include <stdlib.h>

#include <array>
#include <map>
#include <set>

#include <termpaint.h>
#include "testhelper.h"

#include "../third-party/catch.hpp"

#define C(name) TERMPAINT_CAPABILITY_ ## name

static const std::array<const char*, 9> allSeq = {
        "\033[>c",
        "\033[>1c",
        "\033[>0;1c",
        "\033[=c",
        "\033[5n",
        "\033[6n",
        "\033[?6n",
        "\033[1x",
        "\033]4;255;?\007",
    };

static const std::vector<int> allCaps = {
    C(CSI_POSTFIX_MOD), C(TITLE_RESTORE), C(MAY_TRY_CURSOR_SHAPE_BAR), C(CURSOR_SHAPE_OSC50),
    C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED), C(88_COLOR),
    C(CLEARED_COLORING), C(7BIT_ST),
};

static std::vector<int> allCapsBut(std::initializer_list<int> excluded) {
    std::vector<int> ret = allCaps;
    for (int cap : excluded) {
        ret.erase(std::remove(ret.begin(), ret.end(), cap), ret.end());
    }
    return ret;
}

struct SeqResult {
    std::string reply;
    std::string junk = std::string();
};

enum PatchStatus { WithoutGlitchPatching, NeedsGlitchPatching };

struct TestCase {
    std::string name;
    std::map<std::string, SeqResult> seq;
    std::string auto_detect_result_text;
    std::vector<int> caps;
    PatchStatus glitchPatching;
};

#define STRINGIFY2(x) #x
#define STRINGIFY1(x) STRINGIFY2(x)
#define LINEINFO " (on line " STRINGIFY1(__LINE__) ")"

static const std::initializer_list<TestCase> tests = {
    // ---------------
    {
        "xterm 264" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;264;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>0;264;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS}R" }},
            { "\033[1x",          { "\033[3;1;1;128;128;1;0x" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\007" }},
        },
        "Type: xterm(264) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(TITLE_RESTORE),
          C(EXTENDED_CHARSET),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "xterm 280" LINEINFO,
        {
            { "\033[>c",          { "\033[>41;280;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>41;280;0c" }},
            { "\033[=c",          { "\033P!|0\033\\" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS};1R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\007" }},
        },
        "Type: xterm(280) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(TITLE_RESTORE),
          C(EXTENDED_CHARSET),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "xterm 336" LINEINFO,
        {
            { "\033[>c",          { "\033[>41;336;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>41;336;0c" }},
            { "\033[=c",          { "\033P!|00000000\033\\" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS};1R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\007" }},
        },
        "Type: xterm(336) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(TITLE_RESTORE), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "xterm 354" LINEINFO,
        {
            { "\033[>c",          { "\033[>41;354;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>41;354;0c" }},
            { "\033[=c",          { "\033P!|00000000\033\\" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS};1R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\007" }},
        },
        "Type: xterm(354) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(TITLE_RESTORE), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "DA3 new id promise (safe-CPR)" LINEINFO, // promise that newly allocated DA3 ids will be seen as fully featured
        {
            { "\033[>c",          { "\033[>61;234;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>61;234;0c", }},
            { "\033[=c",          { "\033P!|FFFFFFFF\033\\", }},
            { "\033[5n",          { "\033[0n", }},
            { "\033[6n",          { "\033[{POS}R", }},
            { "\033[?6n",         { "\033[?{POS}R", }},
            { "\033[1x",          { "", }},
            { "\033]4;255;?\007", { "", }},
        },
        "Type: unknown full featured(0) safe-CPR seq:>=",
        allCapsBut({C(CURSOR_SHAPE_OSC50), C(88_COLOR)}), // should have all compliant capabilites.
            // excluding CURSOR_SHAPE_OSC50 because it's konsole specific and non standard
            // excluding 88_COLOR because it reduces 256 color palette to 88 color.
        WithoutGlitchPatching
    },
    // ---------------
    {
        "DA3 new id promise (no safe-CPR)" LINEINFO, // promise that newly allocated DA3 ids will be seen as fully featured
        {
            { "\033[>c",          { "\033[>61;234;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>61;234;0c", }},
            { "\033[=c",          { "\033P!|FFFFFFFF\033\\", }},
            { "\033[5n",          { "\033[0n", }},
            { "\033[6n",          { "\033[{POS}R", }},
            { "\033[?6n",         { "", }},
            { "\033[1x",          { "", }},
            { "\033]4;255;?\007", { "", }},
        },
        "Type: unknown full featured(0)  seq:>=",
        allCapsBut({C(CURSOR_SHAPE_OSC50), C(88_COLOR)}), // should have all compliant capabilites. See above for details
        WithoutGlitchPatching
    },
    // ---------------
    {
        "DA3 new id promise (safe-CPR, CSI>1c)" LINEINFO, // promise that newly allocated DA3 ids will be seen as fully featured
        {
            { "\033[>c",          { "\033[>61;234;0c" }},
            { "\033[>1c",         { "\033[>61;234;0c" }},
            { "\033[>0;1c",       { "\033[>61;234;0c", }},
            { "\033[=c",          { "\033P!|FFFFFFFF\033\\", }},
            { "\033[5n",          { "\033[0n", }},
            { "\033[6n",          { "\033[{POS}R", }},
            { "\033[?6n",         { "\033[?{POS}R", }},
            { "\033[1x",          { "", }},
            { "\033]4;255;?\007", { "", }},
        },
        "Type: unknown full featured(0) safe-CPR seq:>=",
        allCapsBut({C(CURSOR_SHAPE_OSC50), C(88_COLOR)}), // should have all compliant capabilites.
            // excluding CURSOR_SHAPE_OSC50 because it's konsole specific and non standard
            // excluding 88_COLOR because it reduces 256 color palette to 88 color.
        WithoutGlitchPatching
    },
    // ---------------
    {
        "DA3 new id promise (no safe-CPR, CSI>1c)" LINEINFO, // promise that newly allocated DA3 ids will be seen as fully featured
        {
            { "\033[>c",          { "\033[>61;234;0c" }},
            { "\033[>1c",         { "\033[>61;234;0c" }},
            { "\033[>0;1c",       { "\033[>61;234;0c", }},
            { "\033[=c",          { "\033P!|FFFFFFFF\033\\", }},
            { "\033[5n",          { "\033[0n", }},
            { "\033[6n",          { "\033[{POS}R", }},
            { "\033[?6n",         { "", }},
            { "\033[1x",          { "", }},
            { "\033]4;255;?\007", { "", }},
        },
        "Type: unknown full featured(0)  seq:>=",
        allCapsBut({C(CURSOR_SHAPE_OSC50), C(88_COLOR)}), // should have all compliant capabilites. See above for details
        WithoutGlitchPatching
    },
    {
        "DA3 new id promise (no safe-CPR, CSI 1x)" LINEINFO, // promise that newly allocated DA3 ids will be seen as fully featured
        {
            { "\033[>c",          { "\033[>61;234;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>61;234;0c", }},
            { "\033[=c",          { "\033P!|FFFFFFFF\033\\", }},
            { "\033[5n",          { "\033[0n", }},
            { "\033[6n",          { "\033[{POS}R", }},
            { "\033[?6n",         { "", }},
            { "\033[1x",          { "\033[3;1;1;128;128;1;0x", }},
            { "\033]4;255;?\007", { "", }},
        },
        "Type: unknown full featured(0)  seq:>=",
        allCapsBut({C(CURSOR_SHAPE_OSC50), C(88_COLOR)}), // should have all compliant capabilites. See above for details
        WithoutGlitchPatching
    },
    // ---------------
    {
        "kitty 0.13.3" LINEINFO,
        {
            { "\033[>c",          { "\033[>1;4000;13c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>1;4000;13c" }},
            { "\033[=c",          { "", "c" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\033\\" }},
        },
        "Type: base(0) safe-CPR seq:>",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        NeedsGlitchPatching
    },
    // ---------------
    {
        "st 0.8.2" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: incompatible with input handling(0)  seq:",
        { C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "no cursor position but terminal status" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: toodumb(0)  seq:",
        { C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "cursor position and terminal status" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "only ESC[>c" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;115;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "" }},
            { "\033[6n",          { "" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: toodumb(0)  seq:",
        { C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "cursor position, terminal status and ESC[>c" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;115;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "alacritty 0.2.9" LINEINFO,
        {
            { "\033[>c",          { "\033[?6c" }},
            { "\033[>1c",         { "\033[?6c" }},
            { "\033[>0;1c",       { "\033[?6c" }},
            { "\033[=c",          { "\033[?6c" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: toodumb(0)  seq:",
        { C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "alacritty 0.4.0" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "eterm 0.9.6" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "konsole 14.12.3" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;115;0c" }},
            { "\033[>1c",         { "\033[>0;115;0c" }},
            { "\033[>0;1c",       { "\033[>0;115;0c\033[>0;115;0c" }},
            { "\033[=c",          { "", "c" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "\033[3;1;1;112;112;1;0x" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: konsole(0)  seq:>",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR), C(CURSOR_SHAPE_OSC50),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED),
          C(CLEARED_COLORING) },
        NeedsGlitchPatching
    },
    // ---------------
    {
        "mlterm 3.8.9" LINEINFO,
        {
            { "\033[>c",          { "\033[>24;279;0c" }},
            { "\033[>1c",         { "\033[>24;279;0c" }},
            { "\033[>0;1c",       { "\033[>24;279;0c" }},
            { "\033[=c",          { "\033P!|000000\033\\" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS};1R" }},
            { "\033[1x",          { "\033[3;1;1;112;112;1;0x" }},
            { "\033]4;255;?\007", { "\033]4;255;rgb:eeee/eeee/eeee\033\\" }},
        },
        "Type: base(0) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "pangoterm with libvterm 0.1.3" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;100;0c" }},
            { "\033[>1c",         { "\033[>0;100;0c" }},
            { "\033[>0;1c",       { "\033[>0;100;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "pterm/putty 0.73" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;100;0c" }},
            { "\033[>1c",         { "\033[>0;100;0c" }},
            { "\033[>0;1c",       { "\033[>0;100;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "screen 3.9.15" LINEINFO,
        {
            { "\033[>c",          { "\033[>83;30915;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>83;30915;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "\033[3;1;1;112;112;1;0x" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: screen(30915)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET),
          C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "terminology 1.6.0" LINEINFO,
        {
            { "\033[>c",          { "\033[>61;337;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>61;337;0c" }},
            { "\033[=c",          { "\033P!|7E7E5459\033\\" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS};1R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: terminology(0) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "terminus 1.0.104 with xtermjs" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;276;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>0;276;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "\033[?{POS}R" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: xterm(276) safe-CPR seq:>=",
        { C(CSI_POSTFIX_MOD), C(TITLE_RESTORE),
          C(EXTENDED_CHARSET),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "tmux 0.9" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: incompatible with input handling(0)  seq:",
        { C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "tmux 1.3" LINEINFO,
        {
            { "\033[>c",          { "" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "tmux 1.7" LINEINFO,
        {
            { "\033[>c",          { "\033[>0;95;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>0;95;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: base(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "tmux 2.0" LINEINFO,
        {
            { "\033[>c",          { "\033[>84;0;0c" }},
            { "\033[>1c",         { "" }},
            { "\033[>0;1c",       { "\033[>84;0;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: tmux(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(TRUECOLOR_MAYBE_SUPPORTED), C(TRUECOLOR_SUPPORTED),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "rxvt-unicode 9.09" LINEINFO,
        {
            { "\033[>c",          { "\033[>85;95;0c" }},
            { "\033[>1c",         { "\033[>85;95;0c" }},
            { "\033[>0;1c",       { "\033[>85;95;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "\033[3;1;1;128;128;1;0x" }},
            { "\033]4;255;?\007", { "\033]4;rgb:eeee/eeee/eeee\007" }},
        },
        "Type: urxvt(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
    // ---------------
    {
        "rxvt-unicode 9.09 with 88color compile time option" LINEINFO,
        {
            { "\033[>c",          { "\033[>85;95;0c" }},
            { "\033[>1c",         { "\033[>85;95;0c" }},
            { "\033[>0;1c",       { "\033[>85;95;0c" }},
            { "\033[=c",          { "" }},
            { "\033[5n",          { "\033[0n" }},
            { "\033[6n",          { "\033[{POS}R" }},
            { "\033[?6n",         { "" }},
            { "\033[1x",          { "\033[3;1;1;128;128;1;0x" }},
            { "\033]4;255;?\007", { "" }},
        },
        "Type: urxvt(0)  seq:>=",
        { C(CSI_POSTFIX_MOD), C(MAY_TRY_CURSOR_SHAPE_BAR),
          C(EXTENDED_CHARSET), C(88_COLOR),
          C(CLEARED_COLORING), C(7BIT_ST) },
        WithoutGlitchPatching
    },
};

static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return str;
    }
    std::string ret = str;
    ret.replace(start_pos, from.length(), to);
    return ret;
}


static void event_callback(void *ctx, termpaint_event *event) {
    bool& events_leaked = *static_cast<bool*>(ctx);
    if (event->type != TERMPAINT_EV_AUTO_DETECT_FINISHED) {
        events_leaked = true;
    }
}

#define CHECK_UPDATE_OK(...) CHECK( __VA_ARGS__ ); \
    if (!(__VA_ARGS__)) ok = false

TEST_CASE("finger printing") {
    const TestCase& testcase = GENERATE(values(tests));
    CAPTURE(testcase.name);

    REQUIRE(testcase.seq.size() == allSeq.size()); // test case sanity check
    bool ok = true;
    for (const auto& seq : allSeq) {
        CHECK_UPDATE_OK(testcase.seq.find(seq) != testcase.seq.end()); // test case sanity check
    }

    auto theTest = [&] (int initialX, int initialY, int width, int height) {

        struct Integration : public termpaint_integration {
            std::string sent;
            std::string log;
        } integration;

        bool events_leaked = false;

        termpaint_integration_init(&integration,
                                   [] (termpaint_integration*) {}, // free
                                   [] (termpaint_integration *integration_generic, const char *data, int length) {
                                        auto& integration = *static_cast<Integration*>(integration_generic);
                                        integration.sent.append(data, length);
                                   },
                                   [] (termpaint_integration*) {} // flush
                                );

        termpaint_integration_set_logging_func(&integration, [] (termpaint_integration *integration_generic, const char *data, int length) {
                    auto& integration = *static_cast<Integration*>(integration_generic);
                    integration.log.append(data, length);
                });

        terminal_uptr term;
        term.reset(termpaint_terminal_new(&integration));
        termpaint_terminal_set_event_cb(term, event_callback, &events_leaked);
        termpaint_terminal_auto_detect(term);

        PatchStatus glitchPatchingWasNeeded = PatchStatus::WithoutGlitchPatching;
        std::set<std::tuple<int, int>> glitched;
        int cursorX = initialX;
        int cursorY = initialY;
        bool pendingWrap = false;

        auto wrapIfNeeded = [&] {
            if (pendingWrap) {
                cursorX = 0;
                if (cursorY + 1 >= height) {
                    std::set<std::tuple<int, int>> glitchedNew;
                    for (auto pos : glitched) {
                        glitchedNew.insert(std::make_tuple(std::get<0>(pos), std::get<1>(pos) - 1));
                    }
                    glitched = glitchedNew;
                } else {
                    cursorY += 1;
                }
            }
        };
        auto advance = [&] {
            if (cursorX + 1 < width) {
                cursorX += 1;
            } else {
                pendingWrap = true;
            }
        };

        for (size_t i = 1; i <= integration.sent.size(); i++) {
            const std::string part = integration.sent.substr(0, i);
            if (part == " ") {
                glitched.erase(std::make_tuple(cursorX, cursorY));
                advance();

                integration.sent = integration.sent.substr(1);
                i = 0;
            } else if (part == "\010") {
                cursorX = std::max(cursorX - 1, 0);

                integration.sent = integration.sent.substr(1);
                i = 0;
            } else {
                const auto it = testcase.seq.find(part);
                if (it != testcase.seq.end()) {
                    const auto result = it->second;
                    if (result.reply.size()) {
                        std::string replyAdjusted = result.reply;
                        std::string cursorPosition = std::to_string(cursorY + 1) + ";" + std::to_string(cursorX + 1);
                        replyAdjusted = replace(replyAdjusted, "{POS}", cursorPosition);
                        termpaint_terminal_add_input_data(term, replyAdjusted.data(), replyAdjusted.size());
                    }
                    if (result.junk.size()) {
                        glitchPatchingWasNeeded = PatchStatus::NeedsGlitchPatching;
                        for (size_t j = 0; j < result.junk.size(); j++) {
                            wrapIfNeeded();
                            glitched.insert(std::make_tuple(cursorX, cursorY));
                            advance();
                        }
                    }
                    integration.sent = integration.sent.substr(i);
                    i = 0;
                }
            }
        }

        INFO(integration.log);

        REQUIRE(integration.sent.empty());

        CHECK_UPDATE_OK(termpaint_terminal_auto_detect_state(term) != termpaint_auto_detect_running);
        CHECK_UPDATE_OK(!events_leaked);
        CHECK_UPDATE_OK(glitched == std::set<std::tuple<int, int>>{});

        char auto_detect_result_text[1000];
        termpaint_terminal_auto_detect_result_text(term, auto_detect_result_text, sizeof (auto_detect_result_text));
        CHECK_UPDATE_OK(auto_detect_result_text == testcase.auto_detect_result_text);

        std::vector<int> detected_caps;
        for (int cap : allCaps) {
            if (termpaint_terminal_capable(term, cap)) {
                detected_caps.push_back(cap);
            }
        }

        CHECK_UPDATE_OK(detected_caps == testcase.caps);

        CHECK_UPDATE_OK(glitchPatchingWasNeeded == testcase.glitchPatching);

        term.reset();
        termpaint_integration_deinit(&integration);
    };
    theTest(0, 0, 40, 4);

    if (ok && testcase.glitchPatching == NeedsGlitchPatching) {
        SECTION("Varying terminal positions") {
            struct TermSetup {
                int initialX, initialY, width, height;
            };

            const auto setup = GENERATE(
                        TermSetup{38, 0, 40, 4},
//                        TermSetup{39, 0, 40, 4}, // needs additional workaround for glitches at end of line
                        TermSetup{0, 3, 40, 4}
//                        TermSetup{39, 3, 40, 4}
                        );

            CAPTURE(setup.initialX, setup.initialY, setup.width, setup.height);
            theTest(setup.initialX, setup.initialY, setup.width, setup.height);
        }
    }
}
