### mosaic

Simple image blocking code to run blockwise image averaging (or median filtering) to create an abstract representation of an image.

See http://www.jenniferlashbrook.com/Portfolio.html for examples of using paint swatches to create physical mosaics.

### Recreating swatch mosaics
This could be seen as the first step to algorithmically creating swatch paintings. Of course, creating a swatch image this is not as simple as finding the average of a block. The mapping of RGB values to physcial swatches is not straightforward. Differences in albedo, eye response, monitor calibration, lighting, etc. precludes a simple 1:1 mapping. Compiling a database of swatches and canonical RGB values will have to suffice as a first order approximation.