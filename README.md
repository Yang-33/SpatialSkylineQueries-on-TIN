# SpatialSkylineQueries-on-TIN
A spatial skyline query is a query to find a set of data points that are not spatially dominated by other data points, given a set of data points ![P](https://render.githubusercontent.com/render/math?math=%5Clarge+%5Cdisplaystyle+P) and query points ![Q](https://render.githubusercontent.com/render/math?math=%5Clarge+%5Cdisplaystyle+Q) in a multidimensional space.
The query enumerates the skyline points based on distance in a multidimensional space.
However, existing spatial skyline queries can lead to large errors with actual travel distances in geospaces because the query is based on the Euclidean distances.

Therefore, we propose **a spatial skyline query on TINs** (triangulated irregular networks) that are frequently used to represent the surface of an object as a terrain.
We define a spatial skyline query based on more accurate travel distances considering TIN instead of the Euclidean distance.
We also propose an efficient solution method using indexes to find nearest neighbor points in a TIN space and a method to reduce unnecessary candidates of the skyline points and TINs.

## Requirement
- Docker (>= 19.03.13)
- Linux (>= 18.04.5)

## Try on Docker
```bash
$ xhost +local:root
$ docker image build -t ssqontin .
$ docker run -a STDOUT -a STDERR --mount type=bind,source=${PWD},target=/workspaces/SSQonTIN --net host -e DISPLAY=:0 -v /tmp/.X11-unix:/tmp/.X11-unix --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --entrypoint /bin/bash -it ssqontin
$ cd workspaces/SSQonTIN # in docker container
$ ./scripts/build-type-exec.sh release ./data/q3-0001000.off '-Psize=100 -Qsize=10' # in docker container
$ xhost -local:root # in your local environment
```

You don't have to type the above commands if you use [vscode dev container](https://code.visualstudio.com/docs/remote/containers).

  - **for wsl2 user** : please modify |DISPLAY| to execute the docker run command (you need Windows X Server).
``` diff
- -e DISPLAY=':0'
+ -e DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0.0
```

## Build, Run and Visualize
``` bash
$ ./scripts/build-type-exec.sh [release/debug] [TINfilepath] [option]
```

Example :
```bash
$ ./scripts/build-type-exec.sh release ./data/q3-0001000.off '-Psize=100 -Qsize=10'
```

### Option
See [`source/skyline/main.cc`](https://github.com/Yang-33/SpatialSkylineQueries-on-TIN/blob/main/source/skyline/main.cc)
- You can only run faster solutions by adding the `-onlyfast` option.

## Measure time
### Naive vs Proposal
``` bash
$ ./scripts/measure-multi-rand-{P,Q,MBRQ,tin}.sh [number of trials]
```
After running the script, see `log/summary/{ds,time}-{Psize,Qsize,MBRQ,tin}-randrange.txt` as a summary.

### Measure time in detail
``` bash
$ git checkout -b detail origin/detail
$ ./scripts/measure-multi-rand-{P,Q,MBRQ,tin}-detail.sh [number of trials]
```
After running the script, see `log/summary/{ds,time}-{Psize,Qsize,MBRQ,tin}Detail-[method name]-randrange.txt` as a summary.

## (Optional) Create TIN files
There are several TIN files under the `./data` directory.

If you want to use other TINs to solve the problem, you can use this script `./scripts/create-tin.sh` to crate TINs.
- You can also modify the number of vertices and the random seed by changing `./source/tin/create_fractal_points.cc`.
