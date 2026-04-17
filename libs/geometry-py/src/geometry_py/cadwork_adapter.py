"""Conversion helpers from cadwork facet data into the geometry_py Brep model.

This module depends on the ``cadwork`` runtime package (installed alongside
cadwork 3D). Keeping the adapter separate from the core geometry classes
means ``geometry_py.Brep`` / ``Face`` / ``Loop`` remain usable in plain
Python environments where the ``cadwork`` module is unavailable.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from geometry_py.brep import Brep
from geometry_py.face import Face
from geometry_py.loop import Loop
from geometry_py.plane3d import Plane3D
from geometry_py.point3d import Point3D
from geometry_py.vector3d import Vector3D

if TYPE_CHECKING:
    from cadwork.facet_list import facet_list
    from cadwork.vertex_list import vertex_list


def _loop_from_vertex_list(verts: vertex_list) -> Loop:
    points: list[Point3D] = []
    for i in range(verts.count()):
        p = verts.at(i)
        points.append(Point3D(p.x, p.y, p.z))
    return Loop(points)


def brep_from_facet_list(facets: facet_list) -> Brep:
    """Build a :class:`Brep` from a cadwork ``facet_list``.

    Each facet becomes a :class:`Face` with:
      - an outer :class:`Loop` from ``facets.get_external_polygon(i)``
      - zero or more inner :class:`Loop` from ``facets.get_internal_polygons(i)``
      - a support :class:`Plane3D` built from the facet normal and the first
        outer-loop vertex (a real point on the plane, avoiding numerical
        issues from reconstructing via ``get_distance_to_origin``).

    The caller retains ownership of ``facets``.
    """
    brep = Brep()
    for i in range(facets.count()):
        outer = _loop_from_vertex_list(facets.get_external_polygon(i))

        holes = facets.get_internal_polygons(i)
        inner = [_loop_from_vertex_list(holes.at(h)) for h in range(holes.count())]

        n = facets.get_normal_vector(i)
        normal = Vector3D(n.x, n.y, n.z)

        point_on_plane = outer.vertex_at(0) if not outer.is_empty() else Point3D.origin()
        plane = Plane3D.from_point_and_normal(point_on_plane, normal)

        brep.add_face(Face(outer, plane, inner))
    return brep
