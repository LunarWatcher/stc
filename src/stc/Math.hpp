/** \file
 *
 * This file contains mathematical extension functions. It relies on many built-in math functions; with the exception of
 * a few functions explicitly designed to be fast or deal with type floatiness in built-in functions, these are
 * extensions and not replacements.
 *
 * Many functions in this library reference an arbitrary vector type (VectorType2D, VectorType3D, ...). 
 * This library does not provide a vector implementation; it's assumed you bring your own. 
 * Non-vector variants may be added in the future if there's interest in them, but as my use primarily covers use-cases
 * where a vector is involved, I don't plan to implement it in the foreseeable future. 
 *
 * If you don't have one, and don't feel like doing it, I can strongly recommend glm: https://github.com/g-truc/glm
 * However, the requirements for the vectors are minimised. The main requirement is that you have an x/y/z property
 * (depending on the dimensionality), and offer an operator==. This is a bare minimum 2D vector that passes these
 * requirements:
 * ```cpp
 * struct Vec2 {
 *     int64_t x, y;
 *     bool operator(const Vec2& other) const { return other.x == x && other.y == y; }
 * };
 * ```
 *
 * Vectors can be of any numeric type, both integers and floats. If you use a float, the usual floating point math
 * caveats apply.
 * Behaviour is often undefined for unsigned vectors; though they're technically legal, subtraction is performed that
 * may be unsafe with certain combinations of unsigned vectors. It's strongly recommended you use a signed type.
 */

#pragma once

#include <cmath>
#include <concepts>

namespace stc::math {

/**
 * Describes an arbitrary 2D vector type that has the properties x and y.
 */
template <typename T, typename IT>
concept VectorType2D = requires (const T& val) {
    { val.x } -> std::convertible_to<IT>;
    { val.y } -> std::convertible_to<IT>;
};

template <typename VT, typename IT>
concept VectorType3D = requires (const VT& val) {
    { val.x } -> std::convertible_to<IT>;
    { val.y } -> std::convertible_to<IT>;
    { val.z } -> std::convertible_to<IT>;
};

/**
 * Utility function for pow(x, 2) that does raw multiplication. Maybe a little bit faster, according to statements I
 * have never fact-checked.
 * Compatible with anything that provides an operator*
 */
template <typename T>
inline T square(const T& val){
    return val * val;
}

/**
 * Contains 2D geometric utility functions.
 *
 * For 3D, when it eventually is implemented, see the g3d namespace.
 */
namespace g2d {

/**
 * Primarily an internal helper function. Checks if three provided points are counter-clockwise.
 */
template <typename IT, VectorType2D<IT> VT>
inline bool isCounterClockwise(const VT& a, const VT& b, const VT& c) {
    return (c.y - a.y) * (b.x - a.x) > (b.y - a.y) * (c.x - a.x);
}

/**
 * Returns whether or not a point is on the left of an edge
 *
 * \returns > 0 if on the left, 0 if on the edge, < 0 if on the right. This is based on the direction of the vector, so
 *          be careful when using in other functions
 */
template <std::signed_integral IT, VectorType2D<IT> VT>
inline IT isPointOnLeftOfEdge(
    const VT& point,
    const VT& lineStart,
    const VT& lineEnd
) {
    // TODO: is this just the dot product? Can it be replaced with 
    //      0 <= dot(AB,AM) <= dot(AB,AB) && 0 <= dot(BC,BM) <= dot(BC,BC)
    // for any real-world benefit?
    return (lineEnd.x - lineStart.x) * (point.y - lineStart.y)
        - (point.x - lineStart.x) * (lineEnd.y - lineStart.y);
}

/**
 * Checks if two arbitrary lines intersect.
 * Note that this method does not consider the start and end points as an intersection. This leads to two fun consequences:
 *
 * 1. If line1 == line2, this returns false. 
 * 2. If line1.start == line2.start (or equivalent for arbitrary permutations of start and end), this method returns false.
 *
 * For inclusive intersects, use stc::math::g2d::lineIntersectsLineInclusive
 *
 * \see https://web.archive.org/web/20170517111501/http://jeffe.cs.illinois.edu/teaching/373/notes/x06-sweepline.pdf
 * \see https://web.archive.org/web/20250916142823/https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
 */
template <typename IT, VectorType2D<IT> VT>
inline bool lineIntersectsLineExclusive(
    const VT& l1Start,
    const VT& l1End,
    const VT& l2Start,
    const VT& l2End
) {
    if (
        l1Start == l1End
        || l1End == l2Start
        || l2Start == l1Start
    ) {
        return false;
    }
    // Degenerate case: either bou
    return (isCounterClockwise<IT, VT>(l1Start, l2Start, l2End) != isCounterClockwise<IT, VT>(l1End, l2Start, l2End)
        && isCounterClockwise<IT, VT>(l1Start, l1End, l2Start) != isCounterClockwise<IT, VT>(l1Start, l1End, l2End)
    );
}

/**
 * Equivalent to stc::math::g2d::lineIntersects, but also counts points on the line.
 *
 * \see https://web.archive.org/web/20170517111501/http://jeffe.cs.illinois.edu/teaching/373/notes/x06-sweepline.pdf
 * \see https://web.archive.org/web/20250916142823/https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
 */
template <typename IT, VectorType2D<IT> VT>
    requires std::equality_comparable_with<VT, VT>
inline bool lineIntersectsLineInclusive(
    const VT& l1Start,
    const VT& l1End,
    const VT& l2Start,
    const VT& l2End
) {
    return (isCounterClockwise<IT, VT>(l1Start, l2Start, l2End) != isCounterClockwise<IT, VT>(l1End, l2Start, l2End)
        && isCounterClockwise<IT, VT>(l1Start, l1End, l2Start) != isCounterClockwise<IT, VT>(l1Start, l1End, l2End)
    )
        || isPointOnLeftOfEdge<IT, VT>(l1Start, l2Start, l2End) == 0
        || isPointOnLeftOfEdge<IT, VT>(l1End, l2Start, l2End) == 0;
}

/**
 * Checks if a line intersects a rectangle.
 *
 * Note that the input arguments are in the form
 * ```
 *   (B) #------# (D)
 *       |      |
 *   (A) #------# (C)
 * ```
 * 
 * Meaning the segments AB, AC, BD, CD are checked. The orientation is irrelevant as long as you provide the arguments
 * in an order that meets these constraints.
 *
 * This function is exclusive, meaning tangential lines (including lines on the borders) is not considered an
 * intersection. For tangential lines to be included, use stc::math::g2d::lineIntersectsRectangleInclusive. 
 * 
 * Nether of the functions check if the line is contained within the rectangle. Use stc::math::g2d::rectangleContains to
 * check point inclusion.
 */
template <typename IT, VectorType2D<IT> VT>
inline bool lineIntersectsRectangleExclusive(
    const VT& lineStart,
    const VT& lineEnd,

    const VT& rectCornerA,
    const VT& rectCornerB,
    const VT& rectCornerC,
    const VT& rectCornerD
) {
    return lineIntersectsLineExclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerA, rectCornerB
    ) || lineIntersectsLineExclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerA, rectCornerC
    ) || lineIntersectsLineExclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerB, rectCornerD
    ) || lineIntersectsLineExclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerC, rectCornerD
    );
}


/**
 * Checks if a line intersects a rectangle.
 *
 * Note that the input arguments are in the form
 * ```
 *   (B) #------# (D)
 *       |      |
 *   (A) #------# (C)
 * ```
 * 
 * Meaning the segments AB, AC, BD, CD are checked. The orientation is irrelevant as long as you provide the arguments
 * in an order that meets these constraints.
 *
 * This function is inclusive, meaning tangential lines (including lines on the borders) are considered an
 * intersection. For tangential lines not to be included, use stc::math::g2d::lineIntersectsRectangleExclusive. 
 * 
 * Nether of the functions check if the line is contained within the rectangle. Use
 * stc::math::g2d::rectangleContainsPointExclusive and stc::math::g2d::rectangleContainsPointInclusive to
 * check point inclusion.
 */
template <typename IT, VectorType2D<IT> VT>
inline bool lineIntersectsRectangleInclusive(
    const VT& lineStart,
    const VT& lineEnd,

    const VT& rectCornerA,
    const VT& rectCornerB,
    const VT& rectCornerC,
    const VT& rectCornerD
) {
    return lineIntersectsLineInclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerA, rectCornerB
    ) || lineIntersectsLineInclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerA, rectCornerC
    ) || lineIntersectsLineInclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerB, rectCornerD
    ) || lineIntersectsLineInclusive<IT, VT>(
        lineStart, lineEnd,
        rectCornerC, rectCornerD
    );
}

/**
 * Checks whether or not a rectangle contains a given point.
 *
 * Note that the input arguments are in the form
 * ```
 *   (B) #------# (D)
 *       |      |
 *   (A) #------# (C)
 * ```
 *
 * Also note that there's no requirement each corner is in that exact position.
 *
 * This is an exclusive function, so points on the border are not included.
 */
template <std::signed_integral IT, VectorType2D<IT> VT>
inline bool rectangleContainsPointExclusive(
    const VT& point,

    const VT& rectCornerA,
    const VT& rectCornerB,
    const VT& rectCornerC,
    const VT& rectCornerD
) {
    return
        isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerA, rectCornerB
        ) > 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerC, rectCornerA
        ) > 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerB, rectCornerD
        ) > 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerD, rectCornerC
        ) > 0;
}


/**
 * Checks whether or not a rectangle contains a given point.
 *
 * Note that the input arguments are in the form
 * ```
 *   (B) #------# (D)
 *       |      |
 *   (A) #------# (C)
 * ```
 *
 * Also note that there's no requirement each corner is in that exact position.
 *
 * This is an inclusive function, so points on the border are included.
 */
template <std::signed_integral IT, VectorType2D<IT> VT>
inline bool rectangleContainsPointInclusive(
    const VT& point,

    const VT& rectCornerA,
    const VT& rectCornerB,
    const VT& rectCornerC,
    const VT& rectCornerD
) {
    return
        isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerA, rectCornerB
        ) >= 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerC, rectCornerA
        ) >= 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerB, rectCornerD
        ) >= 0
        && isPointOnLeftOfEdge<IT, VT>(
            point, rectCornerD, rectCornerC
        ) >= 0;
}


/**
 * Checks if an axis-aligned rectangle contains a provided point. 
 * The input assumes a rectangle defined by two opposing corners.
 * 
 * This function is exclusive, and excludes points on the border.
 */
template <std::signed_integral IT, VectorType2D<IT> VT>
inline bool rectangleContainsPointExclusive(
    const VT& point,

    const VT& startPosition,
    const VT& endPosition
) {
    auto right = std::max(startPosition.x, endPosition.x);
    auto left = std::min(startPosition.x, endPosition.x);
    auto bottom = std::min(startPosition.y, endPosition.y);
    auto top = std::max(startPosition.y, endPosition.y);

    return left < point.x && point.x < right
        && bottom < point.y && point.y < top;
}

/**
 * Checks if an axis-aligned rectangle contains a provided point. 
 * The input assumes a rectangle defined by two opposing corners.
 * 
 * This function is inclusive, and includes points on the border.
 */
template <std::signed_integral IT, VectorType2D<IT> VT>
inline bool rectangleContainsPointInclusive(
    const VT& point,

    const VT& startPosition,
    const VT& endPosition
) {
    auto right = std::max(startPosition.x, endPosition.x);
    auto left = std::min(startPosition.x, endPosition.x);
    auto bottom = std::min(startPosition.y, endPosition.y);
    auto top = std::max(startPosition.y, endPosition.y);

    return left <= point.x && point.x <= right
        && bottom <= point.y && point.y <= top;
}


}

}
