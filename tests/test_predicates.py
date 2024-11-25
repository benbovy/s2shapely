import numpy as np

import spherely
import pytest


def test_intersects() -> None:
    # test array + scalar
    a = np.array(
        [
            spherely.linestring([(40, 8), (60, 8)]),
            spherely.linestring([(20, 0), (30, 0)]),
        ]
    )
    b = spherely.linestring([(50, 5), (50, 10)])

    actual = spherely.intersects(a, b)
    expected = np.array([True, False])
    np.testing.assert_array_equal(actual, expected)

    # two scalars
    a2 = spherely.point(50, 8)
    b2 = spherely.point(20, 5)
    assert not spherely.intersects(a2, b2)


def test_equals() -> None:
    # test array + scalar
    a = np.array(
        [
            spherely.linestring([(40, 8), (60, 8)]),
            spherely.linestring([(20, 0), (30, 0)]),
        ]
    )
    b = spherely.point(50, 8)

    actual = spherely.equals(a, b)
    expected = np.array([False, False])
    np.testing.assert_array_equal(actual, expected)

    # two scalars
    a2 = spherely.point(50, 8)
    b2 = spherely.point(50, 8)
    assert spherely.equals(a2, b2)


def test_contains():
    # test array + scalar
    a = np.array(
        [
            spherely.linestring([(40, 8), (60, 8)]),
            spherely.linestring([(20, 0), (30, 0)]),
        ]
    )
    b = spherely.point(40, 8)

    actual = spherely.contains(a, b)
    expected = np.array([True, False])
    np.testing.assert_array_equal(actual, expected)

    # two scalars
    a2 = spherely.linestring([(50, 8), (60, 8)])
    b2 = spherely.point(50, 8)
    assert spherely.contains(a2, b2)


def test_contains_polygon():
    # plain vs. hole polygon
    poly_plain = spherely.polygon(shell=[(0, 0), (4, 0), (4, 4), (0, 4)])

    poly_hole = spherely.polygon(
        shell=[(0, 0), (4, 0), (4, 4), (0, 4)],
        holes=[[(1, 1), (3, 1), (3, 3), (1, 3)]],
    )

    assert spherely.contains(poly_plain, spherely.point(2, 2))
    assert not spherely.contains(poly_hole, spherely.point(2, 2))


def test_within():
    # test array + scalar
    a = spherely.point(40, 8)
    b = np.array(
        [
            spherely.linestring([(40, 8), (60, 8)]),
            spherely.linestring([(20, 0), (30, 0)]),
        ]
    )

    actual = spherely.within(a, b)
    expected = np.array([True, False])
    np.testing.assert_array_equal(actual, expected)

    # two scalars
    a2 = spherely.point(50, 8)
    b2 = spherely.linestring([(50, 8), (60, 8)])
    assert spherely.within(a2, b2)


def test_within_polygon():
    # plain vs. hole polygon
    poly_plain = spherely.polygon(shell=[(0, 0), (4, 0), (4, 4), (0, 4)])

    poly_hole = spherely.polygon(
        shell=[(0, 0), (4, 0), (4, 4), (0, 4)],
        holes=[[(1, 1), (3, 1), (3, 3), (1, 3)]],
    )

    assert spherely.within(spherely.point(2, 2), poly_plain)
    assert not spherely.within(spherely.point(2, 2), poly_hole)


def test_disjoint():
    a = spherely.point(40, 9)
    b = np.array(
        [
            spherely.linestring([(40, 8), (60, 8)]),
            spherely.linestring([(20, 0), (30, 0)]),
        ]
    )

    actual = spherely.disjoint(a, b)
    expected = np.array([True, True])
    np.testing.assert_array_equal(actual, expected)

    # two scalars
    a2 = spherely.point(50, 9)
    b2 = spherely.linestring([(50, 8), (60, 8)])
    assert spherely.disjoint(a2, b2)


def test_touches():
    a = spherely.polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)])
    b = np.array(
        [
            spherely.polygon([(1.0, 1.0), (1.0, 2.0), (2.0, 2.0), (2.0, 1.0)]),
            spherely.polygon([(0.5, 0.5), (0.5, 1.5), (1.5, 1.5), (1.5, 0.5)]),
        ]
    )

    actual = spherely.touches(a, b)
    expected = np.array([True, False])
    np.testing.assert_array_equal(actual, expected)

    a_p = spherely.point(1.0, 1.0)
    b_p = spherely.point(1.0, 1.0)
    # Points do not have a boundary, so they cannot touch per definition
    # This is consistent with PostGIS for example
    # (cmp. https://postgis.net/docs/ST_Touches.html)
    assert not spherely.touches(a_p, b_p)

    b_line = spherely.linestring([(1.0, 1.0), (1.0, 2.0)])
    assert spherely.touches(a_p, b_line)


@pytest.fixture
def parent_poly():
    return spherely.polygon(
        [
            (-118.0, 60.0),
            (-118.0, 40.0),
            (-118.0, 23.0),
            (34.0, 23.0),
            (34.0, 40.0),
            (34.0, 60.0),
        ]
    )


@pytest.fixture
def geographies_covers_contains():
    return np.array(
        [
            # Basic point covers tests, outside, on boundary and interior
            spherely.point(-120.0, 70.0),
            spherely.point(-118.0, 41.0),
            spherely.point(-116.0, 37.0),
            # Basic polyline tests, crossing, on boundary and interior
            spherely.linestring([(-120.0, 70.0), (-116.0, 37.0)]),
            spherely.linestring([(-118.0, 41.0), (-118.0, 23.0)]),
            spherely.linestring([(-117.0, 39.0), (-115.0, 37.0)]),
            # Basic polygon test, crossing, shared boundary and interior
            spherely.polygon(
                [(-120.0, 41.0), (-120.0, 35.0), (-115.0, 35.0), (-115.0, 41.0)]
            ),
            # TODO: This case is currently not fully correct. Supplying a
            # polygon `a` and `b` and checking whether `a` covers `b`,
            # where `a` and `b` share co-linear edges only works
            # when the edges between them are identical.
            # An example of this breaking would be a polygon `a`,
            # consisting of the edges AB, BC and CA and a polygon `b`,
            # consisting of the edges AB*, B*C* and C*A, where B* and C* reside
            # somewhere on the edge AB and BC, respectively.
            # In this case, s2geometry tries to resolve the co-linearity by
            # symbolic perturbation, where B* and C* are moved by
            # an infinitesimal amount. However, the resulting geometry may then not
            # be covered anymore, even if it would be in reality. Therefor,
            # these tests assume identical shared edges between polygons `a` and `b`,
            # which does work as intended.
            spherely.polygon(
                [(-118.0, 40.0), (-118.0, 23.0), (34.0, 23.0), (34.0, 40.0)]
            ),
            spherely.polygon(
                [(-117.0, 40.0), (-117.0, 35.0), (-115.0, 35.0), (-115.0, 40.0)]
            ),
        ]
    )


@pytest.fixture
def geographies_covers_with_labels(geographies_covers_contains):
    return (
        geographies_covers_contains,
        np.array([False, True, True, False, True, True, False, True, True]),
    )


@pytest.fixture
def geographies_contains_with_labels(geographies_covers_contains):
    return (
        geographies_covers_contains,
        np.array([False, False, True, False, False, True, False, True, True]),
    )


@pytest.mark.skip(
    reason="Testing whether a polygon contains a points on its boundary \
                currently returns true, although it should be false"
)
def test_contains_edge_cases(parent_poly, geographies_contains_with_labels):
    polys_to_check, expected_labels = geographies_contains_with_labels

    actual = spherely.contains(parent_poly, polys_to_check)
    np.testing.assert_array_equal(actual, expected_labels)


def test_covers(parent_poly, geographies_covers_with_labels):
    polys_to_check, expected_labels = geographies_covers_with_labels

    actual = spherely.covers(parent_poly, polys_to_check)
    np.testing.assert_array_equal(actual, expected_labels)


def test_covered_by(parent_poly, geographies_covers_with_labels):
    polys_to_check, expected_labels = geographies_covers_with_labels

    actual = spherely.covered_by(polys_to_check, parent_poly)
    np.testing.assert_array_equal(actual, expected_labels)
