## ___Windows BMP images to ASCII strings___
--------------

Three ascii palettes are available in `<utils.h>` to choose the characters from. These are arrays of wide ascii characters ordered in increasing luminance:

```C
static const char palette_minimal[]  = { ... };
static const char palette_base[]     = { ... };
static const char palette_extended[] = { ... };
```

For the RGB to ascii conversion, a string of mappers are available in `<utils.h>`:

```C
// uses the arithmetic average of the red, green and blue values of pixels
static inline char arithmetic(const  rgbq* const  pixel, const  char* const  palette, const  unsigned plength)

// scales red, green and blue values of pixels with predetermined weights
static inline char weighted(...)

// uses the average of the minimum and maximum values amongst red, green and blue values of each pixel
static inline char minmax(...)

// scales red, green and blue values of pixels with predetermined weights (different from the weights used by weighted_mapper)
static inline char luminosity(...)
```

------

<div><img src="./images/readme/jennifer_lawrence.jpg"  width=45%> <img src="./images/readme/jennifer.jpg" width=45%></div>
<div><img src="./images/readme/vendetta-wallpaper.jpg"  width=45%> <img src="./images/readme/vendetta.jpg" width=45%></div>
<div><img src="./images/readme/ginger_woman.jpg"  width=45%> <img src="./images/readme/ginger.jpg" width=45%></div>
<div><img src="./images/readme/butterflies_2.jpg"  width=45%> <img src="./images/readme/butterflies.jpg" width=45%></div>

### ___Caveats___
-----------------

- Doesn't support any other image formats.
- Only supports bitmaps with bottom-up scanline ordering (majority of the bitmaps in contemporary use are of this type). Bitmaps with top-down scanline order will result in a runtime error.
- Owing to the liberal reliance on `Win32` API, will not compile on UNIX systems without substantial effort.
- Not particularly good at capturing specific details in images, especially if the images are large and those details are represented by granular differences in colour gradients (this specificity gets lost in the black and white transformation and downscaling)
- Best results with colour images are obtained when there's a stark contrast between the object of interest and the background (even with a penalizing mapper).
- Monospaced typefaces are critical to get decent renders, non-monospaced typefaces will probably make the patterns incoherent and indistinguishable!
- The distortion in the image dimension during ascii mapping comes from the inherent non-square shaped nature of most typefaces.
Even with monospaced typefaces, characters are taller than they are wide!. This unfortunately makes the ascii representations seem vertically stretched :(

___For a comprehensive explanation of the implementation, browse the source code, it is thoroughly annotated!.___
