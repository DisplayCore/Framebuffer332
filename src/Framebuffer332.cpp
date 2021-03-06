#include <Framebuffer332.h>

Framebuffer332::Framebuffer332(int w, int h, uint8_t *b) {
    _width = w;
    _height = h;
    _buf = b;
}

void Framebuffer332::initializeDevice() {
    fillScreen(0);
}

static inline uint8_t color565to332(color_t c) {
    color_t t = ((c & 0b11000) >> 3) | ((c & 0b11100000000) >> 6) | ((c & 0b1110000000000000) >> 8);
    return t & 0xFF;
}

static inline color_t color332to565(uint8_t c) {
    color_t clr = (((color_t)c & 0b11) << 3) | (((color_t)c & 0b11100) << 6) | (((color_t)c & 0b11100000) << 8);
    clr |= ((clr & 0b0010000000000000) ? 0b0001100000000000 : 0);
    clr |= ((clr & 0b0000000100000000) ? 0b0000000011100000 : 0);
    clr |= ((clr & 0b0000000000011000) ? 0b0000000000000111 : 0);
    return clr;
}

void Framebuffer332::setPixel(int x, int y, color_t color) {

    if (x < 0 || x >= _width || y < 0 || y >= _height) {
        return;
    }
    
    _buf[x + y * _width] = color565to332(color);
}

void Framebuffer332::fillScreen(color_t color) {
    for (int x = 0; x < _width * _height; x++) {
        _buf[x] = color565to332(color);
    }
}

color_t Framebuffer332::colorAt(int x, int y) {
    return color332to565(_buf[x + (y * getWidth())]);
}

void Framebuffer332::draw(Cariad *dev, int x, int y) {
    if (_filter != NULL) {
        uint32_t p = 0;
        color_t line[getWidth()];
        for (int py = 0; py < getHeight(); py++) {
            for (int px = 0; px < getWidth(); px++) {
                line[px] = _filter->process(color332to565(_buf[p]));
                p++;
            }
            dev->openWindow(x, y + py, getWidth(), 1);
            dev->windowData(line, getWidth());
            dev->closeWindow();
        }
    } else {
        dev->openWindow(x, y, getWidth(), getHeight());
        uint32_t len = getWidth() * (uint32_t)getHeight();
        for (uint32_t p = 0; p < len; p++) {
            dev->windowData(color332to565(_buf[p]));
        }
        dev->closeWindow();
    }
}

void Framebuffer332::draw(Cariad *dev, int x, int y, color_t t) {
    uint32_t p = 0;
    color_t line[getWidth()];

    for (int py = 0; py < getHeight(); py++) {
        bool haveTrans = false;
        for (int px = 0; px < getWidth(); px++) {
            color_t col = color332to565(_buf[p]);
            if (col == t) {
                haveTrans = true;
                line[px] = col;
            } else {
                if (_filter != NULL) {
                    col = _filter->process(col);
                }
                line[px] = col;
            }
            p++;
        }
        if (!haveTrans) {
            dev->openWindow(x, y + py, getWidth(), 1);
            dev->windowData(line, getWidth());
            dev->closeWindow();
        } else {
            for (int px = 0; px < getWidth(); px++) {
                if (color332to565(_buf[py * getWidth() + px]) != t) {
                    dev->setPixel(x + px, y + py, line[px]);
                }
            }
        }
    }
}

void Framebuffer332::drawTransformed(Cariad *dev, int x, int y, int transform) {
    uint32_t p = 0;
    for (int py = 0; py < getHeight(); py++) {
        for (int px = 0; px < getWidth(); px++) {
            switch (transform) {
                default:
                    dev->setPixel(x + px, y + py, color332to565(_buf[p]));
                    break;
                case MirrorH:
                    dev->setPixel(getWidth() - (x + px) - 1, y + py, color332to565(_buf[p]));
                    break;
                case MirrorV:
                    dev->setPixel(x + px, getHeight() - (y + py) - 1, color332to565(_buf[p]));
                    break;
                case Rotate180:
                    dev->setPixel(getWidth() - (x + px) - 1, getHeight() - (y + py) - 1, color332to565(_buf[p]));
                    break;
            }
            p++;
        }
    }
}

void Framebuffer332::drawTransformed(Cariad *dev, int x, int y, int transform, color_t t) {
    uint32_t p = 0;
    for (int py = 0; py < getHeight(); py++) {
        for (int px = 0; px < getWidth(); px++) {
            if (color332to565(_buf[p]) != t) {
                switch (transform) {
                    default:
                        dev->setPixel(x + px, y + py, color332to565(_buf[p]));
                        break;
                    case MirrorH:
                        dev->setPixel(getWidth() - (x + px) - 1, y + py, color332to565(_buf[p]));
                        break;
                    case MirrorV:
                        dev->setPixel(x + px, getHeight() - (y + py) - 1, color332to565(_buf[p]));
                        break;
                    case Rotate180:
                        dev->setPixel(getWidth() - (x + px) - 1, getHeight() - (y + py) - 1, color332to565(_buf[p]));
                        break;
                }
            }
            p++;
        }
    }
}
