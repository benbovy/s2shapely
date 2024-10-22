.. _api:

API reference
=============

.. currentmodule:: spherely

.. _api_properties:

Geography properties
--------------------

.. autosummary::
   :toctree: _api_generated/

   GeographyType
   is_geography
   get_dimensions
   get_type_id

.. _api_creation:

Geography creation
------------------

.. autosummary::
   :toctree: _api_generated/

   point
   linestring
   multipoint
   multilinestring
   polygon
   geography_collection
   is_prepared
   prepare
   destroy_prepared

.. _api_accessors:

Geography accessors
-------------------

.. autosummary::
   :toctree: _api_generated/

   centroid
   boundary
   convex_hull

.. _api_predicates:

Predicates
----------

.. autosummary::
   :toctree: _api_generated/

   equals
   intersects
   contains
   within
   disjoint


Input/Output
------------

.. autosummary::
   :toctree: _api_generated/

   from_wkt
   to_wkt
