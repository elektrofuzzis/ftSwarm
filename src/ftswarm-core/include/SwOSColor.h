/*
 * SwOSColors.h
 *
 * some common color defintions
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <FastLED.h>

/*
struct CRGB {
    uint8_t R;
    uint8_t G;
    uint8_t B;

    CRGB() : R(0), G(0), B(0) {}
    CRGB(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b) {}
    
    CRGB Dim(uint8_t brightness) const {
        return CRGB(
            (R * brightness) / 255,
            (G * brightness) / 255,
            (B * brightness) / 255
        );
    }
};

namespace COLOR {
    const CRGB AliceBlue(240, 248, 255);
    const CRGB AntiqueWhite(250, 235, 215);
    const CRGB Aqua(0, 255, 255);
    const CRGB Aquamarine(127, 255, 212);
    const CRGB Azure(240, 255, 255);
    const CRGB Beige(245, 245, 220);
    const CRGB Bisque(255, 228, 196);
    const CRGB Black(0, 0, 0);
    const CRGB BlanchedAlmond(255, 235, 205);
    const CRGB Blue(0, 0, 255);
    const CRGB BlueViolet(138, 43, 226);
    const CRGB Brown(165, 42, 42);
    const CRGB BurlyWood(222, 184, 135);
    const CRGB CadetBlue(95, 158, 160);
    const CRGB Chartreuse(127, 255, 0);
    const CRGB Chocolate(210, 105, 30);
    const CRGB Coral(255, 127, 80);
    const CRGB CornflowerBlue(100, 149, 237);
    const CRGB Cornsilk(255, 248, 220);
    const CRGB Crimson(220, 20, 60);
    const CRGB Cyan(0, 255, 255);
    const CRGB DarkBlue(0, 0, 139);
    const CRGB DarkCyan(0, 139, 139);
    const CRGB DarkGoldenRod(184, 134, 11);
    const CRGB DarkGray(169, 169, 169);
    const CRGB DarkGreen(0, 100, 0);
    const CRGB DarkKhaki(189, 183, 107);
    const CRGB DarkMagenta(139, 0, 139);
    const CRGB DarkOliveGreen(85, 107, 47);
    const CRGB DarkOrange(255, 140, 0);
    const CRGB DarkOrchid(153, 50, 204);
    const CRGB DarkRed(139, 0, 0);
    const CRGB DarkSalmon(233, 150, 122);
    const CRGB DarkSeaGreen(143, 188, 143);
    const CRGB DarkSlateBlue(72, 61, 139);
    const CRGB DarkSlateGray(47, 79, 79);
    const CRGB DarkTurquoise(0, 206, 209);
    const CRGB DarkViolet(148, 0, 211);
    const CRGB DeepPink(255, 20, 147);
    const CRGB DeepSkyBlue(0, 191, 255);
    const CRGB DimGray(105, 105, 105);
    const CRGB DodgerBlue(30, 144, 255);
    const CRGB FireBrick(178, 34, 34);
    const CRGB FloralWhite(255, 250, 240);
    const CRGB ForestGreen(34, 139, 34);
    const CRGB Fuchsia(255, 0, 255);
    const CRGB Gainsboro(220, 220, 220);
    const CRGB GhostWhite(248, 248, 255);
    const CRGB Gold(255, 215, 0);
    const CRGB GoldenRod(218, 165, 32);
    const CRGB Gray(128, 128, 128);
    const CRGB Green(0, 128, 0);
    const CRGB GreenYellow(173, 255, 47);
    const CRGB HoneyDew(240, 255, 240);
    const CRGB HotPink(255, 105, 180);
    const CRGB IndianRed(205, 92, 92);
    const CRGB Indigo(75, 0, 130);
    const CRGB Ivory(255, 255, 240);
    const CRGB Khaki(240, 230, 140);
    const CRGB Lavender(230, 230, 250);
    const CRGB LavenderBlush(255, 240, 245);
    const CRGB LawnGreen(124, 252, 0);
    const CRGB LemonChiffon(255, 250, 205);
    const CRGB LightBlue(173, 216, 230);
    const CRGB LightCoral(240, 128, 128);
    const CRGB LightCyan(224, 255, 255);
    const CRGB LightGoldenRodYellow(250, 250, 210);
    const CRGB LightGray(211, 211, 211);
    const CRGB LightGreen(144, 238, 144);
    const CRGB LightPink(255, 182, 193);
    const CRGB LightSalmon(255, 160, 122);
    const CRGB LightSeaGreen(32, 178, 170);
    const CRGB LightSkyBlue(135, 206, 250);
    const CRGB LightSlateGray(119, 136, 153);
    const CRGB LightSteelBlue(176, 196, 222);
    const CRGB LightYellow(255, 255, 224);
    const CRGB Lime(0, 255, 0);
    const CRGB LimeGreen(50, 205, 50);
    const CRGB Linen(250, 240, 230);
    const CRGB Magenta(255, 0, 255);
    const CRGB Maroon(128, 0, 0);
    const CRGB MediumAquaMarine(102, 205, 170);
    const CRGB MediumBlue(0, 0, 205);
    const CRGB MediumOrchid(186, 85, 211);
    const CRGB MediumPurple(147, 112, 219);
    const CRGB MediumSeaGreen(60, 179, 113);
    const CRGB MediumSlateBlue(123, 104, 238);
    const CRGB MediumSpringGreen(0, 250, 154);
    const CRGB MediumTurquoise(72, 209, 204);
    const CRGB MediumVioletRed(199, 21, 133);
    const CRGB MidnightBlue(25, 25, 112);
    const CRGB MintCream(245, 255, 250);
    const CRGB MistyRose(255, 228, 225);
    const CRGB Moccasin(255, 228, 181);
    const CRGB NavajoWhite(255, 222, 173);
    const CRGB Navy(0, 0, 128);
    const CRGB OldLace(253, 245, 230);
    const CRGB Olive(128, 128, 0);
    const CRGB OliveDrab(107, 142, 35);
    const CRGB Orange(255, 165, 0);
    const CRGB OrangeRed(255, 69, 0);
    const CRGB Orchid(218, 112, 214);
    const CRGB PaleGoldenRod(238, 232, 170);
    const CRGB PaleGreen(152, 251, 152);
    const CRGB PaleTurquoise(175, 238, 238);
    const CRGB PaleVioletRed(219, 112, 147);
    const CRGB PapayaWhip(255, 239, 213);
    const CRGB PeachPuff(255, 218, 185);
    const CRGB Peru(205, 133, 63);
    const CRGB Pink(255, 192, 203);
    const CRGB Plum(221, 160, 221);
    const CRGB PowderBlue(176, 224, 230);
    const CRGB Purple(128, 0, 128);
    const CRGB RebeccaPurple(102, 51, 153);
    const CRGB Red(255, 0, 0);
    const CRGB RosyBrown(188, 143, 143);
    const CRGB RoyalBlue(65, 105, 225);
    const CRGB SaddleBrown(139, 69, 19);
    const CRGB Salmon(250, 128, 114);
    const CRGB SandyBrown(244, 164, 96);
    const CRGB SeaGreen(46, 139, 87);
    const CRGB SeaShell(255, 245, 238);
    const CRGB Sienna(160, 82, 45);
    const CRGB Silver(192, 192, 192);
    const CRGB SkyBlue(135, 206, 235);
    const CRGB SlateBlue(106, 90, 205);
    const CRGB SlateGray(112, 128, 144);
    const CRGB Snow(255, 250, 250);
    const CRGB SpringGreen(0, 255, 127);
    const CRGB SteelBlue(70, 130, 180);
    const CRGB Tan(210, 180, 140);
    const CRGB Teal(0, 128, 128);
    const CRGB Thistle(216, 191, 216);
    const CRGB Tomato(255, 99, 71);
    const CRGB Turquoise(64, 224, 208);
    const CRGB Violet(238, 130, 238);
    const CRGB Wheat(245, 222, 179);
    const CRGB White(255, 255, 255);
    const CRGB WhiteSmoke(245, 245, 245);
    const CRGB Yellow(255, 255, 0);
    const CRGB YellowGreen(154, 205, 50);
}

*/