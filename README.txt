MINI PROJECT: STUDY CORNER
===========================

A full Visual Studio SOLUTION, laid out exactly like your teacher's
"3D_Cube" / "Lighting" projects:

    StudyCorner\                     <- solution folder
        StudyCorner.sln              <- double-click this to open in Visual Studio
        StudyCorner\                 <- project folder
            StudyCorner.vcxproj
            main.cpp
            shader.h
            camera.h
            pointLight.h
            directionalLight.h
            vertexShader.vs
            fragmentShader.fs
            vertexShaderForPhongShading.vs
            fragmentShaderForPhongShading.fs

WHAT'S IN THE SCENE NOW
-------------------------
- Floor, ceiling, two side walls, and a back wall
- A window in the back wall, directly in front of the desk. The opening is
  left empty (no flat "glass" cube -- that just looked like a painted
  square) so you can see a small outdoor scene through it: an overcast sky,
  a low dark tree line, and a handful of rain streaks that continuously
  fall and loop
- A desk with a lamp (POINT LIGHT), a monitor (its screen "glows" using
  the same constant-color trick as the lamp bulb), and a chair pulled up
  in front of it, facing the desk
- A bookshelf, now filled with books across all 3 shelf compartments
  (different colors and slightly different heights per book), placed along
  the SAME back wall as the window (just to its left), so it sits directly
  ahead of the desk instead of tucked away near the room's entrance
- A continuously spinning ceiling fan with 3 different-colored blades
  (the required moving/animated object)

Lighting: the desk lamp is a POINT LIGHT (bulb-style, shines equally in all
directions with distance attenuation) combined with a DIRECTIONAL LIGHT
("sunlight" through the window) -- two different light types, per the
lecture's illumination model slides.

The rain is built the exact same way as the fan blades: plain cubes moved
every frame with translate/rotate/scale. Nothing new is introduced (no
textures, no transparency/blending) -- it's just more of the same trick,
so it should still be easy to explain in the report.

IMPORTANT -- WHERE TO PUT THIS FOLDER
---------------------------------------
Just like your teacher's projects, this .vcxproj points to a shared
GLAD/GLFW/GLM install at "C:\opengl" (Include/Lib folders) and a glad.c
file reached by going up 6 folders from the project file:

    ..\..\..\..\..\..\opengl\glad.c

That only resolves correctly if this project sits at the SAME FOLDER DEPTH
as your other class projects (3D_Cube, Lighting, etc.) -- which, for the
default Visual Studio project location, is:

    C:\Users\<you>\source\repos\<SolutionFolder>\<ProjectFolder>\

So: unzip "StudyCorner" so it sits directly inside the SAME parent folder
where your 3D_Cube / Lighting solution folders already live (e.g. drop it
straight into your "source\repos" folder, right next to them). If your
3D_Cube/Lighting projects open and build fine from that folder, StudyCorner
will too, with no changes needed.

IF IT STILL CAN'T FIND glad.c / GLFW / GLM
---------------------------------------------
Your machine's setup is slightly different from the assumption above. The
safest fix: open your existing, WORKING "Lighting" project in Visual
Studio, and add every file from this folder to it (Add -> Existing Item),
overwriting main.cpp and fragmentShaderForPhongShading.fs, and removing
sphere.h, basic_camera.h, and the Gouraud shader files if they're still in
that project (overwrite that project's pointLight.h with this one too, if
it has an older version). Since "Lighting" already compiles on your
machine, this sidesteps the path question entirely.

HOW TO BUILD
-------------
1. Open StudyCorner.sln in Visual Studio.
2. Make sure the configuration dropdown says "Debug" and "x64" (this is the
   only configuration with the library paths filled in, same as your other
   class projects).
3. Press F5 (or Ctrl+F5 to run without debugging).

CONTROLS (fully keyboard -- no mouse needed)
-----------------------------------------------
W / A / S / D     - move the camera through the scene
Arrow keys        - look around (turn/tilt the camera)
1                 - toggle the desk lamp (point light) on/off
2                 - toggle the sunlight (directional light) on/off
F                 - pause/resume the spinning fan
ESC               - quit

WHAT TO SCREENSHOT FOR THE REPORT LATER
------------------------------------------
- Full scene, both lights on
- Lamp OFF (key 1) to show the point light's contribution alone
- Sun OFF (key 2) to show the directional light's contribution alone
- A couple of frames of the fan mid-spin, showing blade rotation
- A shot through the window showing the rain/sky scenery and the bookshelf
  standing next to it along the back wall
