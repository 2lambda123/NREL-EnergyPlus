// EnergyPlus, Copyright (c) 1996-2024, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/DXFEarClipping.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace DXFEarClipping {

    // Module containing the routines dealing with triangulating a polygon of >4 sides

    // Module information:
    //       Author         Linda Lawrie
    //       Date written   October 2005

    // Purpose of this module:
    // This module provides the techniques and back up procedures for producing a triangulated
    // polygon from a >4 sided figure.  It is only used for DXF output.

    // Methodology employed:
    // Ear clipping has turned out to be the simplest, most robust technique.

    bool InPolygon(Vector3<Real64> const &point, Array1D<Vector3<Real64>> const &poly, int const nsides)
    {
        // this routine is not used in the current scheme

        // Return value
        bool InPolygon;

        // Argument array dimensioning
        EP_SIZE_CHECK(poly, nsides);

        // Locals
        //'Return TRUE if the point (xp,yp) lies inside the circumcircle
        //'made up by points (x1,y1) (x2,y2) (x3,y3)
        //'The circumcircle centre is returned in (xc,yc) and the radius r
        //'NOTE: A point on the edge is inside the circumcircle

        Real64 constexpr epsilon(0.0000001);
        Real64 costheta;
        Real64 m1;
        Real64 m2;
        Real64 acosval;

        Real64 anglesum = 0.0;

        for (int vert = 1; vert <= nsides - 1; ++vert) {

            Vector3<Real64> p1 = poly(vert) - point;
            Vector3<Real64> p2 = poly(vert + 1) - point;

            Real64 m1 = length(p1);
            Real64 m2 = length(p2);

            if (m1 * m2 <= epsilon) {
                return true;
            } else {
                Real64 costheta = dot(p1, p2) / (m1 * m2);
                Real64 acosval = std::acos(costheta);
                anglesum += acosval;
            }
        }

        if (std::abs(anglesum - Constant::TwoPi) <= epsilon) {
           return true;
        }

        return false;
    }

    int Triangulate(EnergyPlusData &state,
                    int const nsides, // number of sides to polygon
                    Array1D<Vector3<Real64>> const &polygon,
                    Array1D<Vector3<int>> &outtriangles,
                    Real64 const surfazimuth,            // surface azimuth angle (outward facing normal)
                    Real64 const surftilt,               // surface tilt angle
                    std::string const &surfname,         // surface name (for error messages)
                    DataSurfaces::SurfaceClass surfclass // surface class
    )
    {

        // Subroutine information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this subroutine:
        // This routine is a self-contained triangulation calculation from a polygon
        // of 3D vertices, nsides, to a returned set (as possible) of triangles -- noted
        // by vertex numbers.

        // Argument array dimensioning
        EP_SIZE_CHECK(polygon, nsides);

        // Subroutine parameter definitions:
        Real64 constexpr point_tolerance(0.00001);

        // Subroutine local variable declarations:
        Array1D_int ears(nsides);
        Array1D_int r_angles(nsides);
        Array1D<Real64> rangles(nsides);
        Array1D_int c_vertices(nsides);
        Array2D_int earvert(nsides, 3);
        Array1D_bool removed(nsides);
        Array1D_int earverts(3);
        Array1D<Vector3<Real64>> vt(nsides);

        int nears;
        int nrangles;
        int ncverts;

        // Object Data
        Array1D<Vector2<Real64>> vertex(nsides);
        Array1D<Vector3<int>> Triangle(nsides);

        if (surfclass == DataSurfaces::SurfaceClass::Floor || surfclass == DataSurfaces::SurfaceClass::Roof ||
            surfclass == DataSurfaces::SurfaceClass::Overhang) {
            CalcRfFlrCoordinateTransformation(nsides, polygon, surfazimuth, surftilt, vt);
            for (int svert = 1; svert <= nsides; ++svert) {
                for (int mvert = svert + 1; mvert <= nsides; ++mvert) {
                    if (std::abs(vt(svert).x - vt(mvert).x) <= point_tolerance) vt(svert).x = vt(mvert).x;
                    if (std::abs(vt(svert).z - vt(mvert).z) <= point_tolerance) vt(svert).z = vt(mvert).z;
                }
            }
            for (int svert = 1; svert <= nsides; ++svert) {
                vertex(svert).x = vt(svert).x;
                vertex(svert).y = vt(svert).z;
                //      if (trackit) write(outputfiledebug,*) 'x=',xvt(svert),' y=',zvt(svert)
            }
        } else {
            CalcWallCoordinateTransformation(nsides, polygon, surfazimuth, surftilt, vt);
            for (int svert = 1; svert <= nsides; ++svert) {
                for (int mvert = svert + 1; mvert <= nsides; ++mvert) {
                    if (std::abs(vt(svert).x - vt(mvert).x) <= point_tolerance) vt(svert).x = vt(mvert).x;
                    if (std::abs(vt(svert).z - vt(mvert).z) <= point_tolerance) vt(svert).z = vt(mvert).z;
                }
            }
            for (int svert = 1; svert <= nsides; ++svert) {
                vertex(svert).x = vt(svert).x;
                vertex(svert).y = vt(svert).y;
            }
        }

        // find ears
        int nvertcur = nsides;
        int ncount = 0;
        int svert = 1;
        int mvert = 2;
        int evert = 3;
        removed = false;
        while (nvertcur > 3) {
            generate_ears(state, nsides, vertex, ears, nears, r_angles, nrangles, c_vertices, ncverts, removed, earverts, rangles);
            if (!any_gt(ears, 0)) {
                ShowWarningError(state,
                                 format("DXFOut: Could not triangulate surface=\"{}\", type=\"{}\", check surface vertex order(entry)",
                                        surfname, 
                                        DataSurfaces::surfaceClassStrings[(int)surfclass]));
                ++state.dataDXFEarClipping->errcount;
                if (state.dataDXFEarClipping->errcount == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual surfaces.");
                }
                if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowMessage(state, format(" surface={} class={}", surfname, DataSurfaces::surfaceClassStrings[(int)surfclass]));

                    for (int j = 1; j <= nsides; ++j) {
                        ShowMessage(state, format(" side={} ({:.1R},{:.1R},{:.1R})", j, polygon(j).x, polygon(j).y, polygon(j).z));
                    }
                    ShowMessage(state, format(" number of triangles found={:12}", ncount));
                    for (int j = 1; j <= nrangles; ++j) {
                        ShowMessage(state, format(" r angle={} vert={} deg={:.1R}", j, r_angles(j), rangles(j) * Constant::RadToDeg));
                    }
                }
                break; // while loop
            }
            if (nears > 0) {
                svert = earverts(1);
                mvert = earverts(2);
                evert = earverts(3);
                // remove ear
                ++ncount;
                removed(mvert) = true;
                earvert(ncount, 1) = svert;
                earvert(ncount, 2) = mvert;
                earvert(ncount, 3) = evert;
                --nvertcur;
            }
            if (nvertcur == 3) {
                int j = 1;
                ++ncount;
                for (int i = 1; i <= nsides; ++i) {
                    if (removed(i)) continue;
                    earvert(ncount, j) = i;
                    ++j;
                }
            }
        }

        int ntri = ncount;

        for (int i = 1; i <= ntri; ++i) {
            Triangle(i).x = earvert(i, 1);
            Triangle(i).y = earvert(i, 2);
            Triangle(i).z = earvert(i, 3);
        }

        outtriangles.allocate(ntri);
        for (int i = 1; i <= ntri; ++i) {
            outtriangles(i) = Triangle(i);
        }

        return ntri;
    }

    Real64 angle_2dvector(Real64 const xa, // vertex coordinate
                          Real64 const ya, // vertex coordinate
                          Real64 const xb, // vertex coordinate
                          Real64 const yb, // vertex coordinate
                          Real64 const xc, // vertex coordinate
                          Real64 const yc  // vertex coordinate
    )
    {

        // Function information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this function:
        // This function calculates the angle between two sides of a 2d polygon.
        // It computes the interior angle in radians at vertex
        // (XB,YB) of the chain formed by the directed edges from
        // (XA,YA) to (XB,YB) to (XC,YC).  The interior is to the
        // left of the two directed edges.

        // References:
        // Geometry Tools for Computer Graphics

        // Return value
        Real64 angle; // the angle, between 0 and 2*PI.

        // angle is set to PI/2 in the degenerate case.

        // Function parameter definitions:
        Real64 constexpr epsilon(0.0000001);

        Real64 x1 = xa - xb;
        Real64 y1 = ya - yb;
        Real64 x2 = xc - xb;
        Real64 y2 = yc - yb;

        Real64 t = std::sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2));
        if (t == 0.0E+00) t = 1.0E+00;

        t = (x1 * x2 + y1 * y2) / t;

        if ((1.0E+00 - epsilon) < std::abs(t)) {
            t = sign(1.0E+00, t);
        }

        angle = std::acos(t);

        if (x2 * y1 - y2 * x1 < 0.0E+00) {
            angle = 2.0E+00 * Constant::Pi - angle;
        }

        return angle;
    }

    bool polygon_contains_point_2d(int const nsides,            // number of sides (vertices)
                                   Array1D<Vector2<Real64>> const &polygon, // points of polygon
                                   Vector2<Real64> const &point       // point to be tested
    )
    {

        // Function information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this function:
        // Determine if a point is inside a simple 2d polygon.  For a simple polygon (one whose
        // boundary never crosses itself).  The polygon does not need to be convex.

        // References:
        // M Shimrat, Position of Point Relative to Polygon, ACM Algorithm 112,
        // Communications of the ACM, Volume 5, Number 8, page 434, August 1962.

        // Return value
        bool inside; // return value, true=inside, false = not inside

        // Argument array dimensioning
        EP_SIZE_CHECK(polygon, nsides);

        // Function local variable declarations:
        int ip1;

        inside = false;

        for (int i = 1; i <= nsides; ++i) {

            if (i < nsides) {
                ip1 = i + 1;
            } else {
                ip1 = 1;
            }

            if ((polygon(i).y < point.y && point.y <= polygon(ip1).y) || (point.y <= polygon(i).y && polygon(ip1).y < point.y)) {
                if ((point.x - polygon(i).x) - (point.y - polygon(i).y) * (polygon(ip1).x - polygon(i).x) / (polygon(ip1).y - polygon(i).y) < 0) {
                    inside = !inside;
                }
            }
        }

        return inside;
    }

    void generate_ears(EnergyPlusData &state,
                       int const nvert, // number of vertices in polygon
                       Array1D<Vector2<Real64>> const &vertex,
                       Array1D_int &ears,       // number of ears possible (dimensioned to nvert)
                       int &nears,              // number of ears found
                       Array1D_int &r_vertices, // number of reflex vertices (>180) possible
                       int &nrverts,            // number of reflex vertices found (>=180)
                       Array1D_int &c_vertices, // number of convex vertices
                       int &ncverts,            // number of convex vertices found (< 180)
                       Array1D_bool &removed,   // array that shows if a vertex has been removed (calling routine)
                       Array1D_int &earvert,    // vertex indicators for first ear
                       Array1D<Real64> &rangles)
    {

        // Subroutine information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this subroutine:
        // This routine generates "ears", "reflex angles" and "convex angles" of the polygon
        // based on the method set for in the reference.

        // Methodology employed:
        // No elegance used here.  Always starts with first vertex in polygon.

        // References:
        // Geometric Tools for Computer Graphics, Philip Schneider, David Eberly. 2003.  Ear
        // clipping for triangulation is described in Chapter 13 on Polygon Partitioning.  Also
        // described in a small article "Triangulation by Ear Clipping", David Eberly, http://www.geometrictools.com

        // Argument array dimensioning
        EP_SIZE_CHECK(vertex, nvert);
        EP_SIZE_CHECK(ears, nvert);
        EP_SIZE_CHECK(r_vertices, nvert);
        EP_SIZE_CHECK(c_vertices, nvert);
        EP_SIZE_CHECK(removed, nvert);
        EP_SIZE_CHECK(earvert, 3);
        EP_SIZE_CHECK(rangles, nvert);

        // Subroutine local variable declarations:
        bool inpoly; // in polygon or not

        // Object Data
        Vector2<Real64> point;               // structure for point
        Array1D<Vector2<Real64>> testtri(3); // structure for triangle

        // initialize, always recalculate
        ears = 0;
        r_vertices = 0;
        rangles = 0.0;
        nears = 0;
        nrverts = 0;
        c_vertices = 0;
        ncverts = 0;

        for (int svert = 1; svert <= nvert; ++svert) {
            if (removed(svert)) continue;
            //  have starting vertex.  now need middle and end
            int mvert = svert + 1;
            for (int j = 1; j <= nvert; ++j) {
                if (mvert > nvert) mvert = 1;
                if (removed(mvert)) {
                    ++mvert;
                    if (mvert > nvert) mvert = 1;
                } else {
                    break;
                }
            }
            int evert = mvert + 1;
            for (int j = 1; j <= nvert; ++j) {
                if (evert > nvert) evert = 1;
                if (removed(evert)) {
                    ++evert;
                    if (evert > nvert) evert = 1;
                } else {
                    break;
                }
            }

            // have gotten start, middle and ending vertices.  test for reflex angle

            Real64 ang = angle_2dvector(vertex(svert).x, vertex(svert).y, vertex(mvert).x, vertex(mvert).y, vertex(evert).x, vertex(evert).y);

            if (ang > Constant::Pi) { // sufficiently close to 180 degrees.
                ++nrverts;
                r_vertices(nrverts) = mvert;
                rangles(nrverts) = ang;
                continue;
            } else {
                ++ncverts;
                c_vertices(ncverts) = mvert;
            }

            // convex angle, see if it's an ear
            testtri(1) = vertex(svert);
            testtri(2) = vertex(mvert);
            testtri(3) = vertex(evert);
            int tvert = evert;
            for (int j = 4; j <= nvert; ++j) {
                ++tvert;
                if (tvert > nvert) tvert = 1;
                if (removed(tvert)) continue;
                point = vertex(tvert);
                inpoly = polygon_contains_point_2d(3, testtri, point);
                if (!inpoly) continue;
                break;
            }
            //    if (trackit) then
            //      write(outputfiledebug,*) ' triangle=',svert,mvert,evert
            //      write(outputfiledebug,*) ' vertex1=',vertex(svert)%x,vertex(svert)%y
            //      write(outputfiledebug,*) ' vertex2=',vertex(mvert)%x,vertex(mvert)%y
            //      write(outputfiledebug,*) ' vertex3=',vertex(evert)%x,vertex(evert)%y
            //      write(outputfiledebug,*) ' inpoly=',inpoly
            //    endif
            if (!inpoly) {
                // found an ear
                ++nears;
                ears(nears) = mvert;
                if (nears == 1) {
                    earvert(1) = svert;
                    earvert(2) = mvert;
                    earvert(3) = evert;
                }
                if (state.dataDXFEarClipping->trackit) {
                    print(state.files.debug, "ear={} triangle={:12}{:12}{:12}\n", nears, svert, mvert, evert);
                }
            }
        }
    }

    void CalcWallCoordinateTransformation(int const nsides,
                                          Array1D<Vector3<Real64>> const &polygon,
                                          Real64 const surfazimuth,
                                          [[maybe_unused]] Real64 const surftilt, // unused1208
                                          Array1D<Vector3<Real64>> &vt)
    {

        // Subroutine information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this subroutine:
        // This routine transforms a "wall" (normally vertical polygon) to a south facing (180 deg outward
        // normal) polygon in 2 d (y vertices are then ignored).

        // Methodology employed:
        // Standard angle rotation

        // Argument array dimensioning
        EP_SIZE_CHECK(polygon, nsides);
        EP_SIZE_CHECK(vt, nsides);

        // convert surface (wall) to facing 180 (outward normal)

        Real64 const alpha = surfazimuth;

        Real64 const alpha180 = 180.0 - alpha; // amount to rotate
        Real64 const alphrad = alpha180 / Constant::RadToDeg;
        Real64 const cos_alphrad = std::cos(alphrad);
        Real64 const sin_alphrad = std::sin(alphrad);

        for (int i = 1; i <= nsides; ++i) {
            vt(i).x = cos_alphrad * polygon(i).x + sin_alphrad * polygon(i).y;
            vt(i).y = -sin_alphrad * polygon(i).x + cos_alphrad * polygon(i).y;
            vt(i).z = polygon(i).z;
        }
    } // CalcWallCoordianteTransformation()

    void CalcRfFlrCoordinateTransformation(int const nsides,
                                           Array1D<Vector3<Real64>> const &polygon,
                                           [[maybe_unused]] Real64 const surfazimuth, // unused1208
                                           Real64 const surftilt,
                                           Array1D<Vector3<Real64>> &vt)
    {

        // Subroutine information:
        //       Author         Linda Lawrie
        //       Date written   October 2005

        // Purpose of this subroutine:
        // This routine transforms a roof/floor (normally flat polygon) to a flat
        // polygon in 2 d (z vertices are then ignored).

        // Standard angle rotation

        // Argument array dimensioning
        EP_SIZE_CHECK(polygon, nsides);
        EP_SIZE_CHECK(vt, nsides);

        // Subroutine local variable declarations:

        Real64 const alpha = -surftilt;
        Real64 const alphrad = alpha / Constant::RadToDeg;
        Real64 const cos_alphrad = std::cos(alphrad);
        Real64 const sin_alphrad = std::sin(alphrad);

        for (int i = 1; i <= nsides; ++i) {
            vt(i).x = polygon(i).x;
            vt(i).y = cos_alphrad * polygon(i).x + sin_alphrad * polygon(i).y;
            vt(i).z = -sin_alphrad * polygon(i).x + cos_alphrad * polygon(i).y;
        }
    } // CalcRfFlrCoordinateTransformation()

    // void reorder([[maybe_unused]] int &nvert) // unused1208
    //{

    //    // Locals
    //    // type (Vector_2d) nvertex(nvert)
    //    // integer i
    //    // type (Vector_2d) point
    //    // integer nrep

    //    //  Vertex, nverts is in cw order, reorder for calc

    //    // nrep=1
    //    // nvertex(1)=vertex(1)
    //    // do i=nvert,1,-1
    //    //  nvertex(nrep)=vertex(i)
    //    //  nrep=nrep+1
    //    // enddo
    //    // do i=1,nvert
    //    //  vertex(i)=nvertex(i)
    //    // enddo
    //}

} // namespace DXFEarClipping

} // namespace EnergyPlus
