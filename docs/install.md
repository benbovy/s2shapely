(install)=

# Installation

## Built distributions

The easiest way to install Spherely is via its binary packages available for
Linux, MacOS, and Windows platforms on [conda-forge](https://conda-forge.org/)
and [PyPI](https://pypi.org/project/spherely/).

### Installation of Python binary wheels (PyPI)

Install the last released binary wheel, e.g., using [pip](https://pip.pypa.io/):

``` sh
$ pip install spherely
```

### Installation of Conda packages (conda-forge)

Install the last released conda-forge package using
[conda](https://docs.conda.io/projects/conda/en/stable/):

``` sh
$ conda install spherely --channel conda-forge
```

## Installation from source

Compiling and installing Spherely from source may be useful for development
purpose and/or for building it against a specific version of S2Geography and/or
S2Geometry.

### Requirements

- Python
- Numpy
- [s2geography](https://github.com/paleolimbot/s2geography) v0.2.0 or higher
- [s2geometry](https://github.com/google/s2geometry) v0.11.1 or higher

Additional build dependencies:

- C++ compiler supporting C++17 standard
- CMake
- [scikit-build-core](https://github.com/scikit-build/scikit-build-core)

### Cloning the source repository

Spherely's source code can be downloaded by cloning its [source
repository](https://github.com/benbovy/spherely):

```sh
$ git clone https://github.com/benbovy/spherely
$ cd spherely
```

### Setting up a development environment using pixi

Spherely provides everything needed to manage its dependencies and run common
tasks via [pixi](https://pixi.sh).

If you have `pixi` installed, you can install a complete development environment
for your platform simply by executing the following command from Spherely's
project root directory:

```sh
$ pixi install --environment all
```

Running the command below from Spherely's root directory will install all
required tools and dependencies (if not installed yet) in a local environment,
build and install Spherely (if needed) and run the tests:

```sh
$ pixi run tests
```

All available tasks are detailed in the ``pyproject.toml`` file or listed via
the following command:

```sh
$ pixi task list
```

### Setting up a development environment using conda

If you don't have `pixi` installed, you can follow the steps below to manually
setup a conda environment for developing Spherely.

After cloning Spherely's source repository, create a conda environment
with the required (and development) dependencies using the
`ci/environment-dev.yml` file:

```sh
$ conda env create -f ci/environment-dev.yml
$ conda activate spherely-dev
```

Build and install Spherely:

```sh
$ python -m pip install . -v --no-build-isolation
```

Note that you can specify a build directory in order to avoid rebuilding the
whole library from scratch each time after editing the code:

```sh
$ python -m pip install . -v --no-build-isolation --config-settings build-dir=build/skbuild
```

Run the tests:

```sh
$ pytest . -v
```

Spherely also uses [pre-commit](https://pre-commit.com/) for code
auto-formatting and linting at every commit. After installing it, you can enable
pre-commit hooks with the following command:

```sh
$ pre-commit install
```

(Note: you can skip the pre-commit checks with `git commit --no-verify`)

### Using the latest S2Geography version

If you want to compile Spherely against the latest version of S2Geography, use:

 ```sh
 $ git clone https://github.com/paleolimbot/s2geography
 $ cmake \
 $     -S s2geography \
 $     -B s2geography/build \
 $     -DCMAKE_CXX_STANDARD=17 \
 $     -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX
 $ cmake --build s2geography/build
 $ cmake --install s2geography/build
 ```
