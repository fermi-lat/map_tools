---
package: map_tools
summary: Livetime-cube container (Exposure, the storage class behind gtltcube and Likelihood's ExposureCube), a WCS sky-image helper (SkyImage), and two legacy tools (exposure_cube, gtdispcube).
layer: analysis
depends_on: [healpix, astro, tip, hoops, st_app, st_stream, facilities]
used_by: [Likelihood, SolarSystemTools, pyLikelihood]
last_reviewed_commit: 23a41db
---

# map_tools Architecture

**map_tools** is a small, mostly 2005–2010 package (original author
T. Burnett). Only one part of it is on the critical path of the
standard analysis: **`map_tools::Exposure`**, the HEALPix ×
cos(θ) [× φ] livetime-cube container. `Likelihood::LikeExposure`
(which `gtltcube` uses) derives from it. `Likelihood::ExposureCube` (which
every exposure, source-map and `gtexpcube2` calculation uses) reads the
`EXPOSURE` and `WEIGHTED_EXPOSURE` tables of a livetime cube into two
`map_tools::Exposure` objects. Most of the rest of the package is legacy
code: one helper with no users, parameter classes used only by tools that
are no longer built, and two built tools that are not part of the
documented tool set.

The Doxygen mainpage (`src/mainpage.h`, 2005) lists `count_map`,
`exposure_map`, `read_map` and `map_stats`, and names `HealpixArray`,
`CosineBinner` and `HealpixArrayIO` as map_tools classes. **All of this
is stale**: those four tools are not built, and the three classes moved to
the `healpix` package long ago. There is no `ExposureHyperCube` class.
`doc/release.notes` ends in 2014.

## Purpose and scope

**Does:**
- Define `Exposure`: a `healpix::HealpixArray<healpix::CosineBinner>`
  (one livetime histogram in cos θ, and optionally φ, per sky pixel)
  that can be filled from FT2 rows, with optional zenith-angle cuts and
  livetime-fraction weighting, and read and written through
  `healpix::HealpixArrayIO`. Filling can split the pixels across OpenMP
  threads (`setNumThreads`), and `fill_block` bins a block of FT2
  intervals into a cube and its weighted twin in one pass over the
  pixels (this is how `gtltcube` fills its cubes).
- Define the template `BasicExposure<S, C>`, which `SolarSystemTools`
  reuses with its own cell type.
- Define `SkyImage`: a float FITS image (2-D or layered 3-D) with an
  `astro::SkyProj` WCS. You can fill it point by point or from an
  `astro::SkyFunction`, and look up values by `SkyDir`.
- Build two executables: `exposure_cube` (FT2 → livetime cube) and
  `gtdispcube` (debug dump of one pixel of a livetime cube).

**Does not:**
- Compute exposure maps (Aeff × livetime) or count maps. `gtexpcube2`
  and `gtexpmap` (Likelihood) and `gtbin` (evtbin) do that. The old
  `exposure_map` and `count_map` sources are still here but are not built.
- Own the livetime-cube file format. The `CosineBinner` cell, its
  process-wide static binning, and `HealpixArrayIO` belong to `healpix`.
  `gtltcube` writes its file through `Likelihood::LikeExposure`, not
  through map_tools (see `src/healpix/doc/architecture/`).
- Use any IRFs. The `irfLoader` link is left over from the unbuilt
  `exposure_map` tool.

## Position in the system

| Direction | Package | Why |
|-----------|---------|-----|
| Depends on | `healpix` | `Exposure` *is* a `HealpixArray<CosineBinner>`. File I/O goes through `HealpixArrayIO`. |
| Depends on | `astro` | `SkyDir`, `PointingTransform` (φ binning), `SkyProj` and `SkyFunction` (`SkyImage`). CLHEP types (`Hep3Vector`, `HepRotation`) come in through `astro`'s headers. |
| Depends on | `tip` | FT2 rows in `Exposure::load`. Image I/O in `SkyImage`. |
| Depends on | `hoops` | `SkyImage(const hoops::IParGroup&)`. `Parameters` and `MapParameters` wrap or derive from `hoops::ParPromptGroup`. |
| Depends on | `st_app`, `st_stream` | Used only by the two executables. They are linked `PUBLIC` on the library, so every consumer inherits them. |
| Depends on | `facilities` | Only `gtdispcube` and the test use it (`Util::expandEnvVar`), and they get it transitively through `tip`. It is not linked directly. |
| Link only | `irfs` (`irfLoader`) | `target_link_libraries(map_tools PUBLIC ... irfLoader)`, but no compiled source includes `irfInterface` or `irfLoader`. Only the unbuilt `src/exposure_map/exposure_map.cxx` does. |
| Used by | `Likelihood` | `LikeExposure : map_tools::Exposure` (gtltcube fills it with `fill_block`, passing the weighted cube, and sets the thread count from its `nthreads` parameter). `ExposureCube` holds `map_tools::Exposure *m_exposure, *m_weightedExposure`, read with `Exposure(filename[, "WEIGHTED_EXPOSURE"])`, and reads livetimes through `data()[dir]`. |
| Used by | `SolarSystemTools` | `ExposureSun.h` uses only the global-namespace `BasicExposure<S, C>` template, instantiated with its own `HealpixArray<CosineBinner2D>`. It does not use `map_tools::Exposure` itself. |
| Used by | `pyLikelihood` | SWIG `%include`s `map_tools/Exposure.h` and `map_tools/SkyImage.h`, and adds `Exposure.data(SkyDir)` with `%extend`. It links map_tools through Likelihood. |
| Not built | `celestialSources/DarkMatter/src/makeMap.cxx` (uses `SkyImage`), `skymaps` (`LivetimeCube` is a 2008 copy of `Exposure`) | Neither is compiled by CMake. |

The tier in `docs/architecture/dependency-layers.md` (`analysis`) is
acceptable, but it comes from the stale `irfLoader` link. Going by what
the code actually uses, map_tools needs only tier 0 and 1 packages
(`healpix`, `astro`, `tip`, `hoops`, plus `st_app`/`st_stream` for the
tools). Removing the `irfLoader` link would let it sit in the `io` tier
and would remove the `map_tools → irfs` edge from the dependency graph.

## Key abstractions

```mermaid
classDiagram
    class BasicExposure~S,C~ {
      <<global namespace, abstract>>
      +fill(dirz, deltat)*
      +fill(dirz, dirzenith, deltat)*
      +operator()(dir, fun) double
      +integral(dir, fun) double
      +data() S&
      +total() / addtotal()
      +setData(S)
    }
    class Exposure {
      +Exposure(pixelsize, cosbinsize, zcut, weighted, zmaxcut)
      +Exposure(file, table="Exposure")
      +fill(dirz, deltat)
      +fill(dirz, zenith, deltat)
      +fill_zenith(dirz, dirx, zenith, deltat)
      +fill_block(rows, use_phi, weighted)
      +setNumThreads(n)
      +numThreads() int
      +load(tip::Table*, GTIvector, verbose)
      +write(file, table)
      +lost()
      -m_dir_cache : vector~pair~CosineBinner*, Simple3Vector~~
    }
    BasicExposure <|-- SkyExposure : typedef<SkyBinner, CosineBinner>
    SkyExposure <|-- Exposure
    Exposure <|-- LikeExposure : Likelihood
    BasicExposure <|-- ExposureSun : SolarSystemTools (via SkyExposure2D)

    class SkyImage {
      +SkyImage(IParGroup)
      +SkyImage(file, ext)
      +SkyImage(center, outfile, pixsize, fov, layers, proj, galactic)
      +addPoint(dir, delta, layer)
      +fill(SkyFunction, layer)
      +pixelValue(dir, layer)
      +operator()(dir)
    }
    SkyFunction <|-- SkyImage
    SkyFunction <|-- DiffuseFunction
    DiffuseFunction *-- SkyImage
```

| Type | Header | Role |
|------|--------|------|
| `BasicExposure<S, C>` | `map_tools/Exposure.h` | Template over a pixelization `S` (indexable by `SkyDir`) and a per-pixel angular binner `C`. Holds the sky array and a running `total()` of time added. **Declared in the global namespace**, as are the typedefs `SkyBinner` (= `healpix::HealpixArray<healpix::CosineBinner>`) and `SkyExposure`. |
| `map_tools::Exposure` | `map_tools/Exposure.h` | The livetime cube. NESTED, equatorial HEALPix. `nside` is the power of 2 for which `12·nside² ≥ 41252/pixelsize²`, capped at 512 (`pixelsize=1` gives nside 64). Fill routines add `deltat` to each pixel's `CosineBinner` at that pixel's cos θ (and φ) relative to the spacecraft axes. A private `m_dir_cache` holds (cell pointer, unit vector) pairs so that filling is a tight dot-product loop. The pixel loop can run on several OpenMP threads (see [Threading](#threading)); the nested struct `Exposure::Interval` is one FT2 interval for `fill_block`. |
| `SkyImage` | `map_tools/SkyImage.h` | Float FITS image with WCS, written to disk in the **destructor**. Layers carry energies (`getEnergies`) when built from parameters or from a counts map's EBOUNDS. |
| `DiffuseFunction` | `map_tools/DiffuseFunction.h` | `SkyFunction` over a 17-layer, 2^k energy-spaced diffuse cube held in a `SkyImage`, plus an analytic extragalactic power law. It is compiled into the library but **nothing uses it**. |
| `Parameters`, `MapParameters` | `map_tools/Parameters.h`, `MapParameters.h` | hoops-based parameter readers for the old tools. They are compiled into the library, but **only unbuilt tools use them**. |

## Data flow and control flow

### Filling a livetime cube (`Exposure::load`, used by `exposure_cube`)

```
Exposure(pixelsize, cosbinsize, zcut, weighted, zmaxcut)
  HealpixArray(Healpix(nside, NESTED, EQUATORIAL))      // one CosineBinner per pixel, sized from the static binning
  if 1/cosbinsize != CosineBinner::nbins():
      resize every cell; CosineBinner::setBinning(0, 1/cosbinsize)   // process-wide static
  create_cache()                                        // (cell*, pixel unit vector) for every pixel

load(scTable, gti, verbose)
  for each FT2 row:
      livetime == 0 → skip                              // "takes care of SAA"
      fraction = overlap of [START,STOP) with the first GTI it touches
      deltat   = LIVETIME × fraction  (× LIVETIME/(STOP−START) if weighted)
      fill_zenith(SCZ, SCX, ZENITH, deltat)
         for each pixel: if zcut < pixel·zenith < zmaxcut (or no cut):
              phi binning on  → CosineBinner::fill(cosθ, φ, deltat) in instrument frame (PointingTransform)
              phi binning off → CosineBinner::fill(pixel·SCZ, deltat)
```

`Likelihood::LikeExposure::load` (gtltcube) does **not** call
`Exposure::load`. It has its own FT2 loop with full GTI handling. It
collects the accepted rows into blocks of `Exposure::Interval` (2048 by
default) and passes each block to `fill_block` on itself, with the
second, weighted `map_tools::Exposure` as the `weighted` argument.
Changes to the `Filler` logic in `Exposure.cxx` (including
`Filler::fill_pixel`, which `fill_block` uses) therefore change gtltcube
output. Changes to `load`/`processEntry` affect only `exposure_cube`.

### Threading

Two paths share the same `Filler` arithmetic (`src/Exposure.cxx`):

```
fill / fill_zenith(row)                       fill_block(rows, use_phi, weighted)
  parallel_fill(m_dir_cache, Filler(row))       serial pre-pass: one Filler per row
    numThreads() <= 1: std::for_each              (same constructor arguments as
      (no OpenMP region)                           fill / fill_zenith)
    else: omp parallel for over pixels,         numThreads() <= 1: plain pixel loop
      one Filler copy per thread                else: omp parallel for over pixels
                                                for each pixel: every row in order,
                                                  Filler::fill_pixel into both cubes
```

- **Determinism:** each pixel's `CosineBinner` is touched by one thread,
  and receives the same additions in the same order as the serial loop.
  The cubes are **bit-identical for any thread count, block size and
  schedule** (checked against gold-standard cubes for 32 configurations,
  see Likelihood's `doc/proposals/ltcube-pixel-threading.md`). Only
  `total()` and `lost()`, which are sums over pixels, can differ in the
  last bits.
- **Thread count:** per object, `setNumThreads(n)`; every constructor
  sets 1. `0` (or less) means `omp_get_max_threads()` (`OMP_NUM_THREADS`,
  otherwise every core). Nothing calls `omp_set_num_threads`, so other
  OpenMP code in the process is not affected. In a build without OpenMP
  `numThreads()` is always 1.
- **One thread runs no OpenMP region.** Running the region on one thread
  made serial fills 7–10% slower, so `numThreads() <= 1` takes the
  original serial loop.
- **Loop schedule:** both loops use `schedule(runtime)`, set around each
  region from `MAP_TOOLS_EXPOSURE_SCHEDULE` (see
  [Environment variables](#environment-variables)) and restored after
  it. The runtime selection costs up to about 6% against a compiled-in
  `schedule(static)`; it exists to compare schedules across machines,
  and the plan is to compile the chosen one in.
- **Exceptions** thrown while filling (for example the `healpix`
  φ-index overflow, healpix-19) are caught inside the region and the
  first one is rethrown on the calling thread, so callers see the same
  exception as before instead of `std::terminate`.
- **Why `fill_block` is faster even on one thread:** the row geometry
  (astro/CLHEP rotations) is built once per row for both cubes instead
  of once per cube, and a pixel's bins stay in cache while the whole
  block is applied to it. On an 8-core laptop, `gtltcube` was 1.4–3.2×
  faster on one thread and 6.4–13.7× faster on 8 threads than the
  serial code before this change.

### Reading a livetime cube

`Exposure(file, table)` calls `HealpixArrayIO::read`. That **resets the
process-wide `CosineBinner` binning** (`NBRBINS`, `COSMIN`, `THETABIN`,
`PHIBINS`) from the file header, then replaces the pixel array.
`Likelihood::ExposureCube` then calls `exposure->data()[dir]` to get one
pixel's `CosineBinner`, and integrates Aeff over it.

### `exposure_cube`

Prompts for `infile` (FT2), `outfile`, `tstart`, `tstop` and `zmin`. It
sets `CosineBinner::setPhiBins(phibins)` (default **15**). It builds an
unweighted and a weighted `Exposure` (pixelsize 1°, cos-bin 0.025, so 40
bins), loads both from FT2 with a single GTI `[tstart, tstop]`, and writes
the `Exposure` and `WEIGHTED_EXPOSURE` tables with `HealpixArrayIO`.
Text-format (`.txt`) pointing input throws "not currently implemented".

### `gtdispcube`

It reads a livetime cube into an `Exposure`. The code that would build a
`SkyImage` and write `outfile` is inside `#if 0`. What the tool actually
does is write **`test_table.txt` in the current directory**: one line per
cos θ bin, giving the livetime at the image centre (`xref`, `yref`), with
one column per φ bin. `outfile`, `nxpix`, `nypix`, `pixscale`, `proj` and
`clobber` are read or prompted but have no effect.

## Public interfaces

### C++ library

The headers consumers actually use are `map_tools/Exposure.h`
(Likelihood, SolarSystemTools, pyLikelihood) and `map_tools/SkyImage.h`
(pyLikelihood SWIG only). `Exposure.h` pulls in
`healpix/HealpixArray.h` and `healpix/CosineBinner.h`.

Filling in blocks (what `Likelihood::LikeExposure::load` does):

```cpp
map_tools::Exposure cube(1., 0.025, -1, false, 1), wcube(1., 0.025, -1, true, 1);
cube.setNumThreads(8);                                   // 0 = OpenMP default
std::vector<map_tools::Exposure::Interval> rows;         // dirz, dirx, zenith, deltat, weighted_deltat
// ... for each accepted FT2 row: rows.push_back(...); when rows.size() reaches the block size:
cube.fill_block(rows, CosineBinner::nphibins() != 0, &wcube);   // use_phi: fill_zenith vs fill(dirz, zenith)
rows.clear();
```

`fill_block` has no equivalent of `fill(dirz, deltat)` (no zenith
direction). The weighted cube's own thread count is not used; the
calling cube's `numThreads()` covers both.

Typical read-only use (what `Likelihood::ExposureCube` does):

```cpp
map_tools::Exposure lt("ltcube.fits");                          // table "Exposure" (case-insensitive: EXPOSURE)
map_tools::Exposure wlt("ltcube.fits", "WEIGHTED_EXPOSURE");
const healpix::CosineBinner & cell = lt.data()[astro::SkyDir(ra, dec)];
double t = lt(astro::SkyDir(ra, dec), aeff_of_costheta);       // Σ_bins livetime × f(cosθ)
```

### Applications

| Tool | Built | Par file | Notes |
|------|-------|----------|-------|
| `exposure_cube` | yes | `pfiles/exposure_cube.par` | Not a `gt` tool, and not wrapped by `sane` or described in `fermitools-fhelp`. Superseded by `gtltcube` (Likelihood). |
| `gtdispcube` | yes | `pfiles/gtdispcube.par` | Debug dump only (see above). No fhelp page. |
| `test_map_tools` | yes (installed to `bin/`) | `pfiles/test_map_tools.par` | Test program. |
| `count_map`, `exposure_map`, `map_stats`, `read_map`, `cube_display` | **no** | par files for all five are still installed to `syspfiles` | `count_map.cxx` includes ROOT's `TSystem.h`. `exposure_map.cxx` needs `irfInterface` and `dataSubselector`. `cube_display.par` is an older copy of `gtdispcube.par` with different parameter names (`numxpix`/`numypix`). |

### Python

There is no module of its own. `pyLikelihood` exposes `Exposure`
(including the `fill*`, `write` and `load` methods) and `SkyImage`, so
Python users can create a `SkyImage` (it writes a file when it is garbage
collected). The `%extend` method `Exposure.data(SkyDir)` returns one
`CosineBinner`, but **it copies the whole `HealpixArray` on every call**
(`pyLikelihood/src/pyLikelihood.i`), which costs roughly
npix × nbins × 4 bytes per call.

### File formats

- **Exposure tables** (written by `HealpixArrayIO`): one row per HEALPix
  pixel with a `COSBINS` vector column. The header has
  `ORDERING=NESTED`, `COORDSYS`, `NSIDE`, `FIRSTPIX`/`LASTPIX`,
  `THETABIN`, `NBRBINS`, `COSMIN` and `PHIBINS`. `exposure_cube` writes
  **no TSTART/TSTOP, GTI or CTHETABOUNDS**, unlike `gtltcube`.
- **SkyImage**: a float image extension named `skyimage`, appended to
  `outfile`, with WCS keywords from `astro::SkyProj::setKeywords`.
- `Templates/CountMap` and `Templates/ExposureCube.txt` are not installed
  and nothing reads them.

### Environment variables

| Variable | Read by | Effect |
|----------|---------|--------|
| `MAP_TOOLS_EXPOSURE_SCHEDULE` | `loop_schedule` in `src/Exposure.cxx`, once per process | OpenMP schedule of the threaded pixel loops: `static` (default, also when unset or not understood), `dynamic,N` or `guided,N` (`N` optional). Cannot change the cubes. Meant for benchmarking; see [Threading](#threading). |
| `OMP_NUM_THREADS` | libgomp | Thread count only for an `Exposure` whose `setNumThreads` is 0 or less. The default of 1 ignores it. |

Plain `OMP_SCHEDULE` is not used: when it is unset libgomp defaults to
`dynamic,1`, which would make the default depend on the environment.
`facilities::Util::expandEnvVar` expands `$(VAR)` in `gtdispcube` file
names and in Likelihood's `ExposureCube`.
`PFILES` (see "Runtime environment" in ScienceTools'
`docs/architecture/README.md`) must find the tools' par files.

## Build and test

- **CMake target:** `map_tools` is a **STATIC** library, compiled from 5
  sources (about 1,200 lines). It links `OpenMP::OpenMP_CXX` `PUBLIC`
  for the threaded fill, so every consumer links libgomp. Upstream commit `83a6110` switched it to
  `SHARED` and `a3cf7cb` switched it back. Executables: `exposure_cube`,
  `gtdispcube`, `test_map_tools`. About 780 lines in
  `count_map/`, `exposure_map/`, `map_stats/` and `read_map/` are never
  compiled.
- **Install:** headers go to `include/map_tools`, `pfiles/` goes to
  `syspfiles` (including the par files of the unbuilt tools), and the
  three executables go to `bin/`. There is **no `data/` directory**, so
  `getDataPath("map_tools")` is never used.
- **`test_map_tools`:**
  - It is a plain `main()` that uses `hoops::ParPromptGroup`, so `PFILES`
    must find `test_map_tools.par`. It is **not registered with CTest**.
  - It checks that `xref` reads as 0. It fills a 10°-pixel, 10-bin
    `Exposure` with a uniform grid, writes it to `infile`
    (`testCube.fits`, in the current directory), reads it back, and writes
    it again to `outfile` (`testCube2.fits`). It then runs
    `TestCosineBinner`, which compares linear and sqrt cos θ binning
    integrals and **changes the static binning**.
  - **All numerical checks of `Exposure` are inside `#if 0`.** Apart
    from `TestCosineBinner`, the test only checks that nothing throws.
  - The threaded fill and `fill_block` are tested in Likelihood's
    `test_Likelihood`: `test_LikeExposure_threads` (thread counts and
    block sizes 1 and 7 against the serial cube) and
    `test_Exposure_fill_block` (row-by-row `fill`/`fill_zenith` against
    `fill_block` for several block sizes and thread counts, both cubes,
    bit for bit, and the `std::invalid_argument` for a mismatched
    weighted cube). The gold-standard comparison for `gtltcube` output
    is `Likelihood/src/test/ltcube_bench/` (`run_ltcube_matrix.py`,
    `compare_ltcubes.py`).
- **ST-unit-test:** the default run of ScienceTools'
  `recipe/tests/ST-unit-test` (`unitTestsWithoutRoot`) runs
  `test_map_tools`, then `check_test_map_tools_output`. That checks that
  both files exist and `ftdiff`s `testCube.fits` and `testCube2.fits`
  against `$CONDA_PREFIX/share/fermitools/data/map_tools/outref/`; with
  `--bit64` (not used by the CI invocation `-w -d -v`) it compares against
  `testCube_64bit.fits` and `testCube2_64bit.fits` instead. This package
  has no `outref` data of its own: the reference files ship in the
  runtime `fermitools-data` conda package, sourced from
  `outrefFiles/map_tools/` in the `fermitools-data` repository. See "Testing" in
  ScienceTools' `docs/architecture/README.md`.
- **Legacy:** `SConscript`, `map_toolsLib.py` and `doc/Doxyfile` belong to
  the SCons/CMT era. `azure-pipelines.yml` is old CI.
- **Branches:** `origin/f26-cpp17-compliance` has small C++17 fixes to
  `cube_display.cxx` and `test_main.cxx`. `origin/Issue17` makes the
  library type optional. `origin/addition_creator_fits` edits only the
  unused `Templates/`. None of them is merged.

## Invariants and gotchas

**Global state shared with healpix and Likelihood**
- `CosineBinner` binning (`nbins`, `cosmin`, sqrt or linear, `nphibins`)
  is **static and process-wide**. `Exposure(pixelsize, cosbinsize, …)`
  calls `CosineBinner::setBinning(0, 1/cosbinsize)` only when the bin
  count changes, and in that case it also resets sqrt weighting to the
  default `true`. `Exposure(file)` resets everything from the file
  header. Construct a second cube with different binning in the same
  process and the first one's cells will be interpreted with the wrong
  binning. Phi binning must be enabled with
  `CosineBinner::setPhiBins(n)` **before** an `Exposure` is constructed,
  because the cells are sized at construction.
- `BasicExposure`, `SkyBinner` and `SkyExposure` are in the **global
  namespace**. `SolarSystemTools` defines more global typedefs
  (`SkyBinner2D`, `SkyExposure2D`) next to them.

**Exposure construction and copying**
- `Exposure(file, table)` leaves `m_zcut`, `m_zmaxcut`, `m_lost` and
  `m_weighted` **uninitialized**, and it does not build `m_dir_cache`.
  A cube read from a file therefore cannot be filled: `fill()` adds to
  `total()` but touches no pixel. `lost()` is garbage. `total()` is 0,
  because the total time is not stored in the file (this is the "broken"
  total in `gtdispcube`'s `#if 0`).
- The implicitly generated copy constructor copies `m_dir_cache`, which
  holds **pointers into the source object's pixel array**. Filling a
  copy writes into the original, or into freed memory if the original
  has been destroyed. Copies made by `Likelihood` (`ExposureCube` copy
  constructor, `LikeExposure` copy constructor) are safe only as long as
  nobody fills them (from reading the code; not checked at runtime).
- `setData()` replaces the array without rebuilding the cache, with the
  same consequence.

**Threaded filling**
- The thread count is a member, copied with the object. A cube read
  from a file has an empty cache, so `fill_block` on it does nothing at
  all: no pixel, and `total()` and `lost()` don't change. Passing such a
  cube as
  `weighted` compares its uninitialized `m_zcut`/`m_zmaxcut` (from
  reading the code; not checked at runtime).
- `fill_block` throws `std::invalid_argument` unless the weighted cube
  has the same number of pixels and the same zenith cuts. It does not
  check the cos θ or φ binning, which is process-wide anyway.
- `fill_block` keeps one `Filler` per row of the block (about 176 bytes
  each), so the block size sets how much row data each thread streams
  per pixel. 2048 rows (360 KB) fits the L2 cache of current laptops;
  8192 rows was measurably slower on a 1.25 MB L2. The block size
  belongs to the caller (`LikeExposure::setBlockSize`).
- `MAP_TOOLS_EXPOSURE_SCHEDULE` is read once, on the first threaded
  fill; changing it later in the same process has no effect.

**Filling from FT2 (`Exposure::load`, used only by `exposure_cube`)**
- **Possible crash (from reading the code):**
  `irow % (nrows/20)` divides by zero when the FT2 table has fewer than
  20 rows and `verbose` is true (the default).
- An FT2 interval is credited only for its overlap with the **first** GTI
  it touches. If an interval spans a gap between two GTIs, the overlap
  with the second GTI is lost. `exposure_cube` passes a single GTI, so
  this does not affect it.
- Rows with `LIVETIME == 0` are skipped, which is how SAA passages are
  handled.
- Zenith cut: a pixel is used only if `zcut < cos(zenith angle) <
  zmaxcut`. Both bounds are exclusive, and `zcut = -1, zmaxcut = 1` means
  no cut. The time that is cut is accumulated in `lost()`.
- "Weighted" means the livetime is multiplied by the livetime fraction
  `LIVETIME/(STOP−START)`. This is the `WEIGHTED_EXPOSURE` used for the
  livetime-dependent efficiency correction in Likelihood.

**`exposure_cube` tool**
- It opens the FT2 file with `editTable` (**read-write**), so a
  read-only FT2 file fails, and it never deletes the table.
- `filter` is prompted and `avoid_saa` is declared, but neither is used.
  `tstart`/`tstop` have no defaults. `phibins` defaults to 15, unlike
  `gtltcube`'s default of 0.
- The output has no `TSTART`/`TSTOP` keywords. `Likelihood::ExposureCube`
  reads `TSTART` from the `EXPOSURE` header, so **exposure_cube output
  probably cannot be used as a gtltcube replacement** (from reading the
  code; not checked at runtime).

**SkyImage**
- Data is written only in the **destructor** (`m_save` is true for newly
  created images). An exception thrown during the write escapes a
  destructor, and the file is incomplete until the object is destroyed.
- `SkyImage(file, ext)` opens the image **read-write** (`editImageFlt`)
  but never writes it back. It requires a 3-D image, because reading
  `NAXIS3` throws for a 2-D file. `m_total` and the other statistics are
  left uninitialized.
- The "NaN" used to fill pixels outside the projection is built by type
  punning `unsigned long lnan[2] = {0xffffffff, 0x7fffffff}`. On LP64
  systems each `unsigned long` is 8 bytes, so the resulting double is a
  tiny denormal (about 2e-314), **not NaN** (from reading the code; not
  checked at runtime).
- Pixel lookup is not consistent. `addPoint` truncates the projected
  coordinate directly, while `pixel_index` and `getNeighbors` subtract
  0.5 first, which is a half-pixel offset between the two. The bounds
  check in `pixel_index` is `k > m_pixelCount+1`, which lets two
  out-of-range indices through. Negative projected coordinates are
  wrapped by adding `naxis`.
- In the `(center, outfile, …)` constructor with `fov > 90`, if `ptype`
  is not `CAR`, `AIT` or `ZEA`, the test `m_naxis1 == 0` reads an
  uninitialized member.
- Layer energies come from the counts map's `EBOUNDS` converted
  **keV → MeV** (×0.001), or are generated from `emin`/`emax` in MeV.
  `bincalc=CENTER` gives N layers at bin centres. Any other value gives
  N+1 layers at the bin edges.

**DiffuseFunction (unused)**
- `operator()` adds the extragalactic term only when interpolating
  between layers (`m_fract != 0`). `integral(dir, const Aeff&, a, b)` is
  declared but never defined, so calling it fails at link time.
  `layers()` is hard-coded to 17.

## Extension points

- **Livetime-cube changes** belong in `healpix` (cell layout and I/O) and
  `Likelihood` (`LikeExposure`, `ExposureCube`), not here. If you change
  `Exposure`'s fill logic, re-run the Likelihood and SolarSystemTools
  tests and compare `gtltcube` output against the gold standard
  (`Likelihood/src/test/ltcube_bench/README.md`), because `LikeExposure`
  inherits the `Filler`. Keep `Filler::operator()` and
  `Filler::fill_pixel` in step: the threaded fill uses the first and
  `fill_block` the second, and the cubes must stay bit-identical.
- **Fixing the copy constructor** (rebuilding `m_dir_cache` after copy or
  `setData`) is the main robustness change worth making if `Exposure` is
  ever filled after being copied.
- **Retiring dead code:** the unbuilt tool directories and their par
  files, `Parameters`/`MapParameters`/`DiffuseFunction`, `Templates/`, and
  the `irfLoader` link can be removed without affecting any built
  consumer. `SkyImage` must stay while `pyLikelihood` wraps it.
- **A new tool** would follow the `st_app` pattern used by
  `exposure_cube.cxx` (see `src/st_app/doc/architecture/`): add an
  `add_executable` and a par file.

## Further reading

- `src/healpix/doc/architecture/`: `CosineBinner`, `HealpixArray`,
  `HealpixArrayIO`, and the livetime-cube file format.
- `src/Likelihood/` (`LikeExposure`, `ExposureCube`,
  `makeExposureCube/`): how the analysis builds and reads livetime cubes.
- `src/st_app/doc/architecture/` and `src/ape/doc/architecture/parameter-files.md`:
  tool startup and par files.
- `src/mainpage.h` and `doc/release.notes`: history only, and stale.
