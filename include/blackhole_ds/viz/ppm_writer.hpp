// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/ppm_writer.hpp
// A dependency-free RGB image buffer and PPM (Portable Pixmap) writer.
//
// PPM is the simplest real raster format: a tiny text header followed by
// raw RGB bytes (P6) or ASCII triples (P3). No external library needed,
// and every image viewer and ImageMagick/ffmpeg reads it, so it is the
// right first raster target before pulling in a PNG dependency.
//
// Truth tier: infrastructure (no physics). The pixels it carries may be any
// tier; the writer just serializes bytes.

#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

namespace blackhole_ds::viz {

struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

class Image {
public:
    Image(int width, int height)
        : width_(width > 0 ? width : 1), height_(height > 0 ? height : 1),
          pixels_(static_cast<std::size_t>(width_) * height_) {}

    [[nodiscard]] int width() const noexcept {
        return width_;
    }
    [[nodiscard]] int height() const noexcept {
        return height_;
    }

    void set(int x, int y, Rgb c) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            pixels_[index(x, y)] = c;
        }
    }

    [[nodiscard]] Rgb at(int x, int y) const {
        return pixels_[index(x, y)];
    }

    // Binary P6 PPM: compact, the default. `out` should be a binary stream.
    void write_ppm(std::ostream& out) const {
        out << "P6\n" << width_ << ' ' << height_ << "\n255\n";
        for (const Rgb& p : pixels_) {
            const char rgb[3] = {static_cast<char>(p.r), static_cast<char>(p.g),
                                 static_cast<char>(p.b)};
            out.write(rgb, 3);
        }
    }

    // ASCII P3 PPM: larger but human-readable / diff-able, handy for small
    // committed reference images.
    void write_ppm_ascii(std::ostream& out) const {
        out << "P3\n" << width_ << ' ' << height_ << "\n255\n";
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                const Rgb& p = pixels_[index(x, y)];
                out << static_cast<int>(p.r) << ' ' << static_cast<int>(p.g)
                    << ' ' << static_cast<int>(p.b)
                    << (x + 1 < width_ ? "  " : "");
            }
            out << '\n';
        }
    }

private:
    [[nodiscard]] std::size_t index(int x, int y) const {
        return static_cast<std::size_t>(y) * width_ + x;
    }

    int width_;
    int height_;
    std::vector<Rgb> pixels_;
};

} // namespace blackhole_ds::viz
