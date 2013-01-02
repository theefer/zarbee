# Zarbee

Zarbee is an engine for creating 3D scenes.

Scenes are described in XML as a hierarchy of primitive 3D objects,
onto which modifiers (linear and non-linear) and textures can be
applied.  Dynamic parameters, computed with custom functions, can be
used to animate any part of the world, including the lights and the
camera.

## Building

To compile, simply run the `Makefile` in the `src` directory:

    $ cd src
    $ make

To run a scene, pass it to the `zarbee` executable:

    $ cd src
    $ ./zarbee ../scenes/scene1.xml

To run the full demo:

    $ ./run-demo.sh

## Credits

This project was built for a Computer Graphics course in Linköping in 2005 by:

* Sébastien Cevey
* Josefin Gustavson
* Marco Schmalz
* Sandro Stucki

The *Comic Bakery* track is by Instant Remedy.
