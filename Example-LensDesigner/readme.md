# Usage notes

* We get a lot of failed solves with 'perfect' rays so we add a bit of noise in the ray transmissions from the point source
* When solving, steadily increase the resolution of the lens (after it's settled each time).
* Avoid 'ripples' in the lens. If a ripple appears then restart the solve (go back to low resolution mesh)
* Exporting SVG...
    * Edit the SVG file (check the `lens_output - Manual edits.svg`)
    * Change the `pt` to `mm` in the width and height sections
    * Remove the background
    * Note that there are 2 lines added that are +/-1000mm in x and y. Keep these to check scale when importing
* Importing SVG...
    * Currently we do this via Rhino
        * Import SVG
        * Ungroup (not Explode)
        * Remove any unwanted features (e.g. the scale lines)
        * Re-draw the outline of the lens, keeping the curved surface
        * Select all the curves and use `Join` command
        * Select the the closed curve and choose the `Surface from planar curves` tool (select Surface Tools and this will appear in position A4 on left hand side toolbar)
        * Export the surface as STEP
    * In Fusion 360
        * Import STEP
        * Revolve
        * Adjust the scale (e.g. cut it down to smaller diameter to safely fit onto build plate)
        * Add the wings for handling
        * Add the center hole for drainage
        * Test that you can convert it to an STL mesh and export to 3D printing
* 3D printing
    * Currently we use:
        * 10um layer height
        * Anti-alias 8
        * Image blur 4
        * 1.75s exposure time
