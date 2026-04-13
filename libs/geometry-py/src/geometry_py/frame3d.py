from __future__ import annotations

import math

from geometry_py.point3d import Point3D
from geometry_py.vector3d import Vector3D


class Frame3D:
    """A right-handed 3D coordinate frame with origin and three orthonormal axes.

    Supports coordinate transformations between world and local spaces via
    both matrix-based and dot-product-based methods.
    """

    __slots__ = ("_origin", "_axis_x", "_axis_y", "_axis_z")

    def __init__(
        self,
        origin: Point3D | None = None,
        axis_x: Vector3D | None = None,
        axis_y: Vector3D | None = None,
        axis_z: Vector3D | None = None,
    ) -> None:
        self._origin = origin if origin is not None else Point3D.origin()

        if axis_x is None and axis_y is None and axis_z is None:
            # World / identity frame
            self._axis_x = Vector3D.unit_x()
            self._axis_y = Vector3D.unit_y()
            self._axis_z = Vector3D.unit_z()
        elif axis_x is not None and axis_y is not None and axis_z is not None:
            # 3-axis constructor: normalize all
            self._axis_x = axis_x.normalized()
            self._axis_y = axis_y.normalized()
            self._axis_z = axis_z.normalized()
        elif axis_x is not None and axis_y is not None and axis_z is None:
            # 2-axis constructor: derive Z = normalize(X x Y)
            self._axis_x = axis_x.normalized()
            self._axis_y = axis_y.normalized()
            self._axis_z = axis_x.cross(axis_y).normalized()
        else:
            raise ValueError("Provide either no axes, X+Y, or X+Y+Z")

    # ------------------------------------------------------------------
    # Accessors
    # ------------------------------------------------------------------

    @property
    def origin(self) -> Point3D:
        return self._origin

    @property
    def axis_x(self) -> Vector3D:
        return self._axis_x

    @property
    def axis_y(self) -> Vector3D:
        return self._axis_y

    @property
    def axis_z(self) -> Vector3D:
        return self._axis_z

    def set_origin(self, origin: Point3D) -> None:
        self._origin = origin

    def set_axes(self, axis_x: Vector3D, axis_y: Vector3D, axis_z: Vector3D) -> None:
        self._axis_x = axis_x.normalized()
        self._axis_y = axis_y.normalized()
        self._axis_z = axis_z.normalized()

    # ------------------------------------------------------------------
    # Static factory
    # ------------------------------------------------------------------

    @staticmethod
    def world_frame() -> Frame3D:
        return Frame3D()

    # ------------------------------------------------------------------
    # Matrix-based transformations
    # ------------------------------------------------------------------

    def get_transformation_matrix(self) -> list[float]:
        """Return the 4x4 local-to-world transformation matrix (row-major, 16 elements)."""
        ax, ay, az, o = self._axis_x, self._axis_y, self._axis_z, self._origin
        return [
            ax.x, ay.x, az.x, o.x,
            ax.y, ay.y, az.y, o.y,
            ax.z, ay.z, az.z, o.z,
            0.0,  0.0,  0.0,  1.0,
        ]

    def get_inverse_transformation_matrix(self) -> list[float]:
        """Return the 4x4 world-to-local transformation matrix (row-major, 16 elements)."""
        ax, ay, az = self._axis_x, self._axis_y, self._axis_z
        origin_vec = Vector3D(self._origin.x, self._origin.y, self._origin.z)
        return [
            ax.x, ax.y, ax.z, -ax.dot(origin_vec),
            ay.x, ay.y, ay.z, -ay.dot(origin_vec),
            az.x, az.y, az.z, -az.dot(origin_vec),
            0.0,  0.0,  0.0,  1.0,
        ]

    def world_to_local_matrix(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        if isinstance(obj, Point3D):
            m = self.get_inverse_transformation_matrix()
            lx = m[0] * obj.x + m[1] * obj.y + m[2] * obj.z + m[3]
            ly = m[4] * obj.x + m[5] * obj.y + m[6] * obj.z + m[7]
            lz = m[8] * obj.x + m[9] * obj.y + m[10] * obj.z + m[11]
            return Point3D(lx, ly, lz)
        # Vector: rotation only (no translation)
        ax, ay, az = self._axis_x, self._axis_y, self._axis_z
        lx = ax.x * obj.x + ax.y * obj.y + ax.z * obj.z
        ly = ay.x * obj.x + ay.y * obj.y + ay.z * obj.z
        lz = az.x * obj.x + az.y * obj.y + az.z * obj.z
        return Vector3D(lx, ly, lz)

    def local_to_world_matrix(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        if isinstance(obj, Point3D):
            m = self.get_transformation_matrix()
            wx = m[0] * obj.x + m[1] * obj.y + m[2] * obj.z + m[3]
            wy = m[4] * obj.x + m[5] * obj.y + m[6] * obj.z + m[7]
            wz = m[8] * obj.x + m[9] * obj.y + m[10] * obj.z + m[11]
            return Point3D(wx, wy, wz)
        # Vector: rotation only
        ax, ay, az = self._axis_x, self._axis_y, self._axis_z
        wx = ax.x * obj.x + ay.x * obj.y + az.x * obj.z
        wy = ax.y * obj.x + ay.y * obj.y + az.y * obj.z
        wz = ax.z * obj.x + ay.z * obj.y + az.z * obj.z
        return Vector3D(wx, wy, wz)

    # ------------------------------------------------------------------
    # Dot-product-based transformations
    # ------------------------------------------------------------------

    def world_to_local_dot_product(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        if isinstance(obj, Point3D):
            disp = obj - self._origin  # Vector3D
            return Point3D(disp.dot(self._axis_x), disp.dot(self._axis_y), disp.dot(self._axis_z))
        return Vector3D(obj.dot(self._axis_x), obj.dot(self._axis_y), obj.dot(self._axis_z))

    def local_to_world_dot_product(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        ax, ay, az = self._axis_x, self._axis_y, self._axis_z
        if isinstance(obj, Point3D):
            o = self._origin
            return Point3D(
                o.x + obj.x * ax.x + obj.y * ay.x + obj.z * az.x,
                o.y + obj.x * ax.y + obj.y * ay.y + obj.z * az.y,
                o.z + obj.x * ax.z + obj.y * ay.z + obj.z * az.z,
            )
        return Vector3D(
            obj.x * ax.x + obj.y * ay.x + obj.z * az.x,
            obj.x * ax.y + obj.y * ay.y + obj.z * az.y,
            obj.x * ax.z + obj.y * ay.z + obj.z * az.z,
        )

    # ------------------------------------------------------------------
    # Convenience methods (use dot-product internally)
    # ------------------------------------------------------------------

    def world_to_local(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        return self.world_to_local_dot_product(obj)

    def local_to_world(self, obj: Point3D | Vector3D) -> Point3D | Vector3D:
        return self.local_to_world_dot_product(obj)

    # ------------------------------------------------------------------
    # Frame operations
    # ------------------------------------------------------------------

    def is_orthonormal(self, tolerance: float = 1e-10) -> bool:
        if abs(self._axis_x.length_squared() - 1.0) > tolerance:
            return False
        if abs(self._axis_y.length_squared() - 1.0) > tolerance:
            return False
        if abs(self._axis_z.length_squared() - 1.0) > tolerance:
            return False
        if abs(self._axis_x.dot(self._axis_y)) > tolerance:
            return False
        if abs(self._axis_y.dot(self._axis_z)) > tolerance:
            return False
        if abs(self._axis_z.dot(self._axis_x)) > tolerance:
            return False
        return True

    # ------------------------------------------------------------------
    # String representation
    # ------------------------------------------------------------------

    def __repr__(self) -> str:
        return (
            f"Frame3D{{\n"
            f"  Origin: {self._origin}\n"
            f"  X-Axis: {self._axis_x}\n"
            f"  Y-Axis: {self._axis_y}\n"
            f"  Z-Axis: {self._axis_z}\n"
            f"}}"
        )

    def __str__(self) -> str:
        return self.__repr__()
