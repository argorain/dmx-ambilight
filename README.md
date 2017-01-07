# Ambilight

Ambient light control based on computer screen color.
It controls DMX enabled light via FTDI-DMX interface.
My setup is PC -> DMXPipe -> SOUNDSATION PAR64B-ENTRY

## Requiremnts
- Linux
- libftdi-dev
- xlib
- cmake 3.0 or newer

## Building
1. Create directory build within project folder
```    
    mkdir build
```
2. Go there
```    
    cd build
```
3. Cmake it
```    
    cmake ..
```
4. Build it
```    
    make
```
5. Use it!
```    
    ./ambilight
```

Good luck!
