#!/bin/bash
./scripts/build-release.sh \
&& ./out/release/create_fractal_points > fractal_points.off \
&& ./out/release/tin_reconstruction_from_points fractal_points.off 1 \
&& ./out/release/tin_validator reconstructed_mesh.off \
&& meshlab reconstructed_mesh.off || echo "It can't create right mesh files because the CGAL library can't calculate surface distances with fractal_points.off.\n Please try again after modifying the settings in 'source/tin/create_fractal_points.cc'"
