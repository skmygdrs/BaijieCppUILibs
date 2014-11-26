//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Stroke math
//
//----------------------------------------------------------------------------

#ifndef AGG_STROKE_MATH_INCLUDED
#define AGG_STROKE_MATH_INCLUDED

#include "agg_math.h"
#include "agg_vertex_sequence.h"

namespace agg
{
    //-------------------------------------------------------------line_cap_e
    enum line_cap_e
    {
        butt_cap,
        square_cap,
        round_cap
    };

    //------------------------------------------------------------line_join_e
    enum line_join_e
    {
        miter_join         = 0,
        miter_join_revert  = 1,
        round_join         = 2,
        bevel_join         = 3,
        miter_join_round   = 4
    };


    //-----------------------------------------------------------inner_join_e
    enum inner_join_e
    {
        inner_bevel,
        inner_miter,
        inner_jag,
        inner_round
    };

    //------------------------------------------------------------math_stroke
    template<class VertexConsumer> class math_stroke
    {
    public:
        typedef typename VertexConsumer::value_type coord_type;

        math_stroke();

        void line_cap(line_cap_e lc)     { m_line_cap = lc; }
        void line_join(line_join_e lj)   { m_line_join = lj; }
        void inner_join(inner_join_e ij) { m_inner_join = ij; }

        line_cap_e   line_cap()   const { return m_line_cap; }
        line_join_e  line_join()  const { return m_line_join; }
        inner_join_e inner_join() const { return m_inner_join; }

        void width(real w);
        void miter_limit(real ml) { m_miter_limit = ml; }
        void miter_limit_theta(real t);
        void inner_miter_limit(real ml) { m_inner_miter_limit = ml; }
        void approximation_scale(real as) { m_approx_scale = as; }

        real width() const { return m_width * 2.0f; }
        real miter_limit() const { return m_miter_limit; }
        real inner_miter_limit() const { return m_inner_miter_limit; }
        real approximation_scale() const { return m_approx_scale; }

        void calc_cap(VertexConsumer& vc,
                      const vertex_dist& v0, 
                      const vertex_dist& v1, 
                      real len);

        void calc_join(VertexConsumer& vc,
                       const vertex_dist& v0, 
                       const vertex_dist& v1, 
                       const vertex_dist& v2,
                       real len1, 
                       real len2);

    private:
        AGG_INLINE void add_vertex(VertexConsumer& vc, real x, real y)
        {
            vc.add(coord_type(x, y));
        }

        void calc_arc(VertexConsumer& vc,
                      real x,   real y, 
                      real dx1, real dy1, 
                      real dx2, real dy2);

        void calc_miter(VertexConsumer& vc,
                        const vertex_dist& v0, 
                        const vertex_dist& v1, 
                        const vertex_dist& v2,
                        real dx1, real dy1, 
                        real dx2, real dy2,
                        line_join_e lj,
                        real mlimit,
                        real dbevel);

        real       m_width;
        real       m_width_abs;
        real       m_width_eps;
        int          m_width_sign;
        real       m_miter_limit;
        real       m_inner_miter_limit;
        real       m_approx_scale;
        line_cap_e   m_line_cap;
        line_join_e  m_line_join;
        inner_join_e m_inner_join;
    };

    //-----------------------------------------------------------------------
    template<class VC> math_stroke<VC>::math_stroke() :
        m_width(0.5f),
        m_width_abs(0.5f),
        m_width_eps(0.5/1024.0f),
        m_width_sign(1),
        m_miter_limit(4.0f),
        m_inner_miter_limit(1.01f),
        m_approx_scale(1.0f),
        m_line_cap(butt_cap),
        m_line_join(miter_join),
        m_inner_join(inner_miter)
    {
    }

    //-----------------------------------------------------------------------
    template<class VC> void math_stroke<VC>::width(real w)
    { 
        m_width = w * 0.5f; 
        if(m_width < 0)
        {
            m_width_abs  = -m_width;
            m_width_sign = -1;
        }
        else
        {
            m_width_abs  = m_width;
            m_width_sign = 1;
        }
        m_width_eps = m_width / 1024.0f;
    }

    //-----------------------------------------------------------------------
    template<class VC> void math_stroke<VC>::miter_limit_theta(real t)
    { 
        m_miter_limit = 1.0f / (real)sin(t * 0.5f) ;
    }

    //-----------------------------------------------------------------------
    template<class VC> 
    void math_stroke<VC>::calc_arc(VC& vc,
                                   real x,   real y, 
                                   real dx1, real dy1, 
                                   real dx2, real dy2)
    {
        real a1 = (real)atan2(dy1 * m_width_sign, dx1 * m_width_sign);
        real a2 = (real)atan2(dy2 * m_width_sign, dx2 * m_width_sign);
        real da = a1 - a2;
        int i, n;

        da = (real)acos(m_width_abs / (m_width_abs + 0.125f / m_approx_scale)) * 2.0f;

        add_vertex(vc, x + dx1, y + dy1);
        if(m_width_sign > 0)
        {
            if(a1 > a2) a2 += 2 * pi;
            n = int((a2 - a1) / da);
            da = (a2 - a1) / (n + 1);
            a1 += da;
            for(i = 0; i < n; i++)
            {
                add_vertex(vc, x + (real)cos(a1) * m_width, y + (real)sin(a1) * m_width);
                a1 += da;
            }
        }
        else
        {
            if(a1 < a2) a2 -= 2 * pi;
            n = int((a1 - a2) / da);
            da = (a1 - a2) / (n + 1);
            a1 -= da;
            for(i = 0; i < n; i++)
            {
                add_vertex(vc, x + (real)cos(a1) * m_width, y + (real)sin(a1) * m_width);
                a1 -= da;
            }
        }
        add_vertex(vc, x + dx2, y + dy2);
    }

    //-----------------------------------------------------------------------
    template<class VC> 
    void math_stroke<VC>::calc_miter(VC& vc,
                                     const vertex_dist& v0, 
                                     const vertex_dist& v1, 
                                     const vertex_dist& v2,
                                     real dx1, real dy1, 
                                     real dx2, real dy2,
                                     line_join_e lj,
                                     real mlimit,
                                     real dbevel)
    {
        real xi  = v1.x;
        real yi  = v1.y;
        real di  = 1;
        real lim = m_width_abs * mlimit;
        bool miter_limit_exceeded = true; // Assume the worst
        bool intersection_failed  = true; // Assume the worst

        if(calc_intersection(v0.x + dx1, v0.y - dy1,
                             v1.x + dx1, v1.y - dy1,
                             v1.x + dx2, v1.y - dy2,
                             v2.x + dx2, v2.y - dy2,
                             &xi, &yi))
        {
            // Calculation of the intersection succeeded
            //---------------------
            di = calc_distance(v1.x, v1.y, xi, yi);
            if(di <= lim)
            {
                // Inside the miter limit
                //---------------------
                add_vertex(vc, xi, yi);
                miter_limit_exceeded = false;
            }
            intersection_failed = false;
        }
        else
        {
            // Calculation of the intersection failed, most probably
            // the three points lie one straight line. 
            // First check if v0 and v2 lie on the opposite sides of vector: 
            // (v1.x, v1.y) -> (v1.x+dx1, v1.y-dy1), that is, the perpendicular
            // to the line determined by vertices v0 and v1.
            // This condition determines whether the next line segments continues
            // the previous one or goes back.
            //----------------
            real x2 = v1.x + dx1;
            real y2 = v1.y - dy1;
            if((cross_product(v0.x, v0.y, v1.x, v1.y, x2, y2) < 0.0f) == 
               (cross_product(v1.x, v1.y, v2.x, v2.y, x2, y2) < 0.0f))
            {
                // This case means that the next segment continues 
                // the previous one (straight line)
                //-----------------
                add_vertex(vc, v1.x + dx1, v1.y - dy1);
                miter_limit_exceeded = false;
            }
        }

        if(miter_limit_exceeded)
        {
            // Miter limit exceeded
            //------------------------
            switch(lj)
            {
            case miter_join_revert:
                // For the compatibility with SVG, PDF, etc, 
                // we use a simple bevel join instead of
                // "smart" bevel
                //-------------------
                add_vertex(vc, v1.x + dx1, v1.y - dy1);
                add_vertex(vc, v1.x + dx2, v1.y - dy2);
                break;

            case miter_join_round:
                calc_arc(vc, v1.x, v1.y, dx1, -dy1, dx2, -dy2);
                break;

            default:
                // If no miter-revert, calculate new dx1, dy1, dx2, dy2
                //----------------
                if(intersection_failed)
                {
                    mlimit *= m_width_sign;
                    add_vertex(vc, v1.x + dx1 + dy1 * mlimit, 
                                   v1.y - dy1 + dx1 * mlimit);
                    add_vertex(vc, v1.x + dx2 - dy2 * mlimit, 
                                   v1.y - dy2 - dx2 * mlimit);
                }
                else
                {
                    real x1 = v1.x + dx1;
                    real y1 = v1.y - dy1;
                    real x2 = v1.x + dx2;
                    real y2 = v1.y - dy2;
                    di = (lim - dbevel) / (di - dbevel);
                    add_vertex(vc, x1 + (xi - x1) * di, 
                                   y1 + (yi - y1) * di);
                    add_vertex(vc, x2 + (xi - x2) * di, 
                                   y2 + (yi - y2) * di);
                }
                break;
            }
        }
    }

    //--------------------------------------------------------stroke_calc_cap
    template<class VC> 
    void math_stroke<VC>::calc_cap(VC& vc,
                                   const vertex_dist& v0, 
                                   const vertex_dist& v1, 
                                   real len)
    {
        vc.remove_all();

        real dx1 = (v1.y - v0.y) / len;
        real dy1 = (v1.x - v0.x) / len;
        real dx2 = 0;
        real dy2 = 0;

        dx1 *= m_width;
        dy1 *= m_width;

        if(m_line_cap != round_cap)
        {
            if(m_line_cap == square_cap)
            {
                dx2 = dy1 * m_width_sign;
                dy2 = dx1 * m_width_sign;
            }
            add_vertex(vc, v0.x - dx1 - dx2, v0.y + dy1 - dy2);
            add_vertex(vc, v0.x + dx1 - dx2, v0.y - dy1 - dy2);
        }
        else
        {
            real da = (real)acos(m_width_abs / (m_width_abs + 0.125f / m_approx_scale)) * 2.0f;
            real a1;
            int i;
            int n = int(pi / da);

            da = pi / (n + 1);
            add_vertex(vc, v0.x - dx1, v0.y + dy1);
            if(m_width_sign > 0)
            {
                a1 = (real)atan2(dy1, -dx1);
                a1 += da;
                for(i = 0; i < n; i++)
                {
                    add_vertex(vc, v0.x + (real)cos(a1) * m_width, 
                                   v0.y + (real)sin(a1) * m_width);
                    a1 += da;
                }
            }
            else
            {
                a1 = (real)atan2(-dy1, dx1);
                a1 -= da;
                for(i = 0; i < n; i++)
                {
                    add_vertex(vc, v0.x + (real)cos(a1) * m_width, 
                                   v0.y + (real)sin(a1) * m_width);
                    a1 -= da;
                }
            }
            add_vertex(vc, v0.x + dx1, v0.y - dy1);
        }
    }

    //-----------------------------------------------------------------------
    template<class VC> 
    void math_stroke<VC>::calc_join(VC& vc,
                                    const vertex_dist& v0, 
                                    const vertex_dist& v1, 
                                    const vertex_dist& v2,
                                    real len1, 
                                    real len2)
    {
        real dx1 = m_width * (v1.y - v0.y) / len1;
        real dy1 = m_width * (v1.x - v0.x) / len1;
        real dx2 = m_width * (v2.y - v1.y) / len2;
        real dy2 = m_width * (v2.x - v1.x) / len2;

        vc.remove_all();

        real cp = cross_product(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
        if(cp != 0 && (cp > 0) == (m_width > 0))
        {
            // Inner join
            //---------------
            real limit = ((len1 < len2) ? len1 : len2) / m_width_abs;
            if(limit < m_inner_miter_limit)
            {
                limit = m_inner_miter_limit;
            }

            switch(m_inner_join)
            {
            default: // inner_bevel
                add_vertex(vc, v1.x + dx1, v1.y - dy1);
                add_vertex(vc, v1.x + dx2, v1.y - dy2);
                break;

            case inner_miter:
                calc_miter(vc,
                           v0, v1, v2, dx1, dy1, dx2, dy2, 
                           miter_join_revert, 
                           limit, 0);
                break;

            case inner_jag:
            case inner_round:
                cp = (dx1-dx2) * (dx1-dx2) + (dy1-dy2) * (dy1-dy2);
                if(cp < len1 * len1 && cp < len2 * len2)
                {
                    calc_miter(vc,
                               v0, v1, v2, dx1, dy1, dx2, dy2, 
                               miter_join_revert, 
                               limit, 0);
                }
                else
                {
                    if(m_inner_join == inner_jag)
                    {
                        add_vertex(vc, v1.x + dx1, v1.y - dy1);
                        add_vertex(vc, v1.x,       v1.y      );
                        add_vertex(vc, v1.x + dx2, v1.y - dy2);
                    }
                    else
                    {
                        add_vertex(vc, v1.x + dx1, v1.y - dy1);
                        add_vertex(vc, v1.x,       v1.y      );
                        calc_arc(vc, v1.x, v1.y, dx2, -dy2, dx1, -dy1);
                        add_vertex(vc, v1.x,       v1.y      );
                        add_vertex(vc, v1.x + dx2, v1.y - dy2);
                    }
                }
                break;
            }
        }
        else
        {
            // Outer join
            //---------------

            // Calculate the distance between v1 and 
            // the central point of the bevel line segment
            //---------------
            real dx = (dx1 + dx2) / 2;
            real dy = (dy1 + dy2) / 2;
            real dbevel = SQRT(dx * dx + dy * dy);

            if(m_line_join == round_join || m_line_join == bevel_join)
            {
                // This is an optimization that reduces the number of points 
                // in cases of almost collinear segments. If there's no
                // visible difference between bevel and miter joins we'd rather
                // use miter join because it adds only one point instead of two. 
                //
                // Here we calculate the middle point between the bevel points 
                // and then, the distance between v1 and this middle point. 
                // At outer joins this distance always less than stroke width, 
                // because it's actually the height of an isosceles triangle of
                // v1 and its two bevel points. If the difference between this
                // width and this value is small (no visible bevel) we can 
                // add just one point. 
                //
                // The constant in the expression makes the result approximately 
                // the same as in round joins and caps. You can safely comment 
                // out this entire "if".
                //-------------------
                if(m_approx_scale * (m_width_abs - dbevel) < m_width_eps)
                {
                    if(calc_intersection(v0.x + dx1, v0.y - dy1,
                                         v1.x + dx1, v1.y - dy1,
                                         v1.x + dx2, v1.y - dy2,
                                         v2.x + dx2, v2.y - dy2,
                                         &dx, &dy))
                    {
                        add_vertex(vc, dx, dy);
                    }
                    else
                    {
                        add_vertex(vc, v1.x + dx1, v1.y - dy1);
                    }
                    return;
                }
            }

            switch(m_line_join)
            {
            case miter_join:
            case miter_join_revert:
            case miter_join_round:
                calc_miter(vc, 
                           v0, v1, v2, dx1, dy1, dx2, dy2, 
                           m_line_join, 
                           m_miter_limit,
                           dbevel);
                break;

            case round_join:
                calc_arc(vc, v1.x, v1.y, dx1, -dy1, dx2, -dy2);
                break;

            default: // Bevel join
                add_vertex(vc, v1.x + dx1, v1.y - dy1);
                add_vertex(vc, v1.x + dx2, v1.y - dy2);
                break;
            }
        }
    }




}

#endif
