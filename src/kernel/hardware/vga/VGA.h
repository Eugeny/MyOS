#ifndef HARDWARE_VGA_VGA_H
#define HARDWARE_VGA_VGA_H


class VGA {
public:
    static void moveCursor(int x, int y);
    static void enableHighResolution();
    static int width, height;
};

#endif 