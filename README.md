## This is a collection of small (or not) projects.
<b>All of them will be linked statically (or at least i'll try) so you should be able to use them without downloading any DLLs.

\
Photo sorter:
- Sort pictures into monthly folders according to the date formatted onto their names
- TODO: use exif data instead

\
Obj render:
- Homemade renderer for .obj files, using libraries only for window handling and basic 2d drawing
- TODO: improve multi-object mode       (interface for choosing and moving models around)
- TODO: add further language support    (objects, groups, materials and such)
- TODO: add depth map and sh*t for better primitive intersection rendering
- TODO: change homemade perspective and rotation into Mat4 for view frustum
- Current controls:
    - ``zqsd`` to move camera
    - ``ijkl`` or mouse drag to orient camera
    - ``tfgh`` to move first object (for debug, error if no objects)
    - ``cvbn`` to orient first object (for debug, error if no objects)
