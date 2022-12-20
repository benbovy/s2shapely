#include "geography.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <s2/s2latlng.h>
#include <s2geography.h>

#include "pybind11.hpp"

namespace py = pybind11;
namespace s2geog = s2geography;
using namespace spherely;

py::detail::type_info *PyObjectGeography::geography_tinfo = nullptr;

/*
** Geography factories
*/

class PointFactory {
public:
    static std::unique_ptr<Point> FromLatLonDegrees(double lat_degrees,
                                                    double lon_degrees) {
        auto latlng = S2LatLng::FromDegrees(lat_degrees, lon_degrees);
        S2GeographyPtr s2geog_ptr =
            std::make_unique<s2geog::PointGeography>(S2Point(latlng));
        std::unique_ptr<Point> point_ptr =
            std::make_unique<Point>(std::move(s2geog_ptr));

        return point_ptr;
    }
};

class LineStringFactory {
public:
    using LatLonCoords = std::vector<std::pair<double, double>>;

    static std::unique_ptr<LineString> FromLatLonCoords(LatLonCoords coords) {
        std::vector<S2LatLng> latlng_pts;
        for (auto &latlng : coords) {
            latlng_pts.push_back(
                S2LatLng::FromDegrees(latlng.first, latlng.second));
        }

        auto polyline = std::make_unique<S2Polyline>(latlng_pts);
        S2GeographyPtr s2geog_ptr =
            std::make_unique<s2geog::PolylineGeography>(std::move(polyline));

        return std::make_unique<LineString>(std::move(s2geog_ptr));
    }
};

/*
** Temporary testing Numpy-vectorized API (TODO: remove)
*/

py::array_t<int> num_shapes(const py::array_t<PyObjectGeography> geographies) {
    py::buffer_info buf = geographies.request();

    auto result = py::array_t<int>(buf.size);
    py::buffer_info result_buf = result.request();
    int *rptr = static_cast<int *>(result_buf.ptr);

    for (size_t i = 0; i < buf.size; i++) {
        auto geog_ptr = (*geographies.data(i)).as_geog_ptr();
        rptr[i] = geog_ptr->num_shapes();
    }

    return result;
}

py::array_t<PyObjectGeography> create(py::array_t<double> xs,
                                      py::array_t<double> ys) {
    py::buffer_info xbuf = xs.request(), ybuf = ys.request();
    if (xbuf.ndim != 1 || ybuf.ndim != 1) {
        throw std::runtime_error("Number of dimensions must be one");
    }
    if (xbuf.size != ybuf.size) {
        throw std::runtime_error("Input shapes must match");
    }

    auto result = py::array_t<PyObjectGeography>(xbuf.size);
    py::buffer_info rbuf = result.request();

    double *xptr = static_cast<double *>(xbuf.ptr);
    double *yptr = static_cast<double *>(ybuf.ptr);
    py::object *rptr = static_cast<py::object *>(rbuf.ptr);

    size_t size = static_cast<size_t>(xbuf.shape[0]);

    for (size_t i = 0; i < xbuf.shape[0]; i++) {
        auto point_ptr = PointFactory::FromLatLonDegrees(xptr[i], yptr[i]);
        // rptr[i] = PyObjectGeography::as_py_object(std::move(point_ptr));
        rptr[i] = py::cast(std::move(point_ptr));
    }

    return result;
}

/*
** Geography properties
*/

std::int8_t get_type_id(PyObjectGeography obj) {
    return static_cast<std::int8_t>(obj.as_geog_ptr()->geog_type());
}

int get_dimensions(PyObjectGeography obj) {
    return obj.as_geog_ptr()->dimension();
}

/*
** Geography utils
*/

bool is_geography(PyObjectGeography obj) { return obj.is_geog_ptr(); }

/*
** Geography creation
*/

bool is_prepared(PyObjectGeography obj) {
    return obj.as_geog_ptr()->has_index();
}

PyObjectGeography prepare(PyObjectGeography obj) {
    // triggers index creation if not yet built
    obj.as_geog_ptr()->geog_index();
    return obj;
}

PyObjectGeography destroy_prepared(PyObjectGeography obj) {
    obj.as_geog_ptr()->reset_index();
    return obj;
}

void init_geography(py::module &m) {
    // Geography types

    auto pygeography_types =
        py::enum_<GeographyType>(m, "GeographyType", R"pbdoc(
        The enumeration of Geography types
    )pbdoc");

    pygeography_types.value("NONE", GeographyType::None);
    pygeography_types.value("POINT", GeographyType::Point);
    pygeography_types.value("LINESTRING", GeographyType::LineString);

    // Geography classes

    auto pygeography = py::class_<Geography>(m, "Geography", R"pbdoc(
        Base class for all geography types.

        Cannot be instanciated directly.

    )pbdoc");

    pygeography.def_property_readonly("dimensions", &Geography::dimension,
                                      R"pbdoc(
        Returns the inherent dimensionality of a geometry.

        The inherent dimension is 0 for points, 1 for linestrings and 2 for
        polygons. For geometrycollections it is the max of the containing elements.
        Empty collections and None values return -1.

    )pbdoc");

    pygeography.def_property_readonly("nshape", &Geography::num_shapes, R"pbdoc(
        Returns the number of elements in the collection, or 1 for simple geography
        objects.

    )pbdoc");

    pygeography.def("__repr__", [](const Geography &geog) {
        s2geog::WKTWriter writer;
        return writer.write_feature(geog.geog());
    });

    auto pypoint = py::class_<Point, Geography>(m, "Point", R"pbdoc(
        A geography type that represents a single coordinate with lat,lon or x,y,z values.

        A point is a zero-dimensional feature and has zero length and zero area.

        Parameters
        ----------
        lat : float
            latitude coordinate, in degrees
        lon : float
            longitude coordinate, in degrees

    )pbdoc");

    pypoint.def(py::init(&PointFactory::FromLatLonDegrees), py::arg("lat"),
                py::arg("lon"));

    auto pylinestring =
        py::class_<LineString, Geography>(m, "LineString", R"pbdoc(
        A geography type composed of one or more arc segments.

        A LineString is a one-dimensional feature and has a non-zero length but
        zero area. A LineString is not closed.

        Parameters
        ----------
        coordinates : list of tuple
            A sequence of (lat, lon) coordinates for each vertex.

    )pbdoc");

    pylinestring.def(py::init(&LineStringFactory::FromLatLonCoords),
                     py::arg("coordinates"));

    // Temp test

    m.def("nshape", &num_shapes);
    m.def("create", &create);

    // Geography properties

    m.def("get_type_id", py::vectorize(&get_type_id), py::arg("geography"),
          R"pbdoc(
        Returns the type ID of a geography.

        - None (missing) is -1
        - POINT is 0
        - LINESTRING is 1

        Parameters
        ----------
        geography : :py:class:`Geography` or array_like
            Geography object(s).

    )pbdoc");

    m.def("get_dimensions", py::vectorize(&get_dimensions),
          py::arg("geography"), R"pbdoc(
        Returns the inherent dimensionality of a geography.

        The inherent dimension is 0 for points, 1 for linestrings and 2 for
        polygons. For geometrycollections it is the max of the containing elements.
        Empty collections and None values return -1.

        Parameters
        ----------
        geography : :py:class:`Geography` or array_like
            Geography object(s)

    )pbdoc");

    // Geography utils

    m.def("is_geography", py::vectorize(&is_geography), py::arg("obj"),
          R"pbdoc(
        Returns True if the object is a :py:class:`Geography`, False otherwise.

        Parameters
        ----------
        obj : any or array_like
            Any object.

    )pbdoc");

    // Geography creation

    m.def("is_prepared", py::vectorize(&is_prepared), py::arg("geography"),
          R"pbdoc(
        Returns True if the geography object is "prepared", False otherwise.

        A prepared geography is a normal geography with added information such as
        an index on the line segments. This improves the performance of many operations.

        Note that it is not necessary to check if a geography is already prepared
        before preparing it. It is more efficient to call prepare directly
        because it will skip geographies that are already prepared.

        Parameters
        ----------
        geography : :py:class:`Geography` or array_like
            Geography object(s)

        See Also
        --------
        prepare
        destroy_prepared

    )pbdoc");

    m.def("prepare", py::vectorize(&prepare), py::arg("geography"),
          R"pbdoc(
        Prepare a geography, improving performance of other operations.

        A prepared geography is a normal geography with added information such as
        an index on the line segments. This improves the performance of the
        following operations.

        This function does not recompute previously prepared geographies; it is
        efficient to call this function on an array that partially contains
        prepared geographies.

        Parameters
        ----------
        geography : :py:class:`Geography` or array_like
            Geography object(s)

        See Also
        --------
        is_prepared
        destroy_prepared

    )pbdoc");

    m.def("destroy_prepared", py::vectorize(&destroy_prepared),
          py::arg("geography"),
          R"pbdoc(
        Destroy the prepared part of a geography, freeing up memory.

        Note that the prepared geography will always be cleaned up if the
        geography itself is dereferenced. This function needs only be called in
        very specific circumstances, such as freeing up memory without losing
        the geographies, or benchmarking.

        Parameters
        ----------
        geography : :py:class:`Geography` or array_like
            Geography object(s)

        See Also
        --------
        is_prepared
        prepare

    )pbdoc");
}
