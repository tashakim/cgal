// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: $
// $Revision$ $Date$
// $Name:  $
//
// Author(s)     : Ron Wein   <wein@post.tau.ac.il>

#ifndef CGAL_ENVELOPE_DIVIDE_AND_CONQUER_2_H
#define CGAL_ENVELOPE_DIVIDE_AND_CONQUER_2_H

#include <CGAL/Env_default_diagram_1.h>

CGAL_BEGIN_NAMESPACE

/*! \class
 * A class implementing the divide-and-conquer algorithm for computing the
 * lower (or upper) envelope of a set of curves.
 */
template <class Traits_,
          class Diagram_ = Env_default_diagram_1<Traits_> >
class Envelope_divide_and_conquer_2
{
public:

  typedef Traits_                                  Traits_2;
  typedef typename Traits_2::Point_2               Point_2;
  typedef typename Traits_2::X_monotone_curve_2    X_monotone_curve_2;
  typedef typename Traits_2::Curve_2               Curve_2;

  typedef Diagram_                                 Envelope_diagram_1;
  
  typedef Envelope_divide_and_conquer_2<Traits_2, Envelope_diagram_1>  Self;

  enum Envelope_type
  {
    LOWER,
    UPPER
  };

protected:

  typedef typename Envelope_diagram_1::Vertex_const_handle Vertex_const_handle;
  typedef typename Envelope_diagram_1::Vertex_handle       Vertex_handle;
  typedef typename Envelope_diagram_1::Edge_const_handle   Edge_const_handle;
  typedef typename Envelope_diagram_1::Edge_handle         Edge_handle;

  typedef std::list<X_monotone_curve_2 *>          Curve_pointer_list;
  typedef typename Curve_pointer_list::iterator    Curve_pointer_iterator;

  // Data members:
  Traits_2        *traits;        // The traits object.
  bool             own_traits;    // Whether we own the traits object.
  Envelope_type    env_type;      // Either LOWER or UPPER.

  // Copy constructor and assignment operator - not supported.
  Envelope_divide_and_conquer_2 (const Self& );
  Self& operator= (const Self& );

public:

  /*!
   * Constructor.
   */
  Envelope_divide_and_conquer_2 () :
    own_traits(true),
    env_type(LOWER)
  {
    traits = new Traits_2;
  }

  /*!
   * Constructor with a traits object.
   * \param _traits The traits object.
   */
  Envelope_divide_and_conquer_2 (const Traits_2* _traits) :
    traits (_traits),
    own_traits(false),
    env_type(LOWER)
  {}

  /*!
   * Destructor.
   */
  ~Envelope_divide_and_conquer_2 ()
  {
    if (own_traits)
      delete traits;
  }

  /*!
   * Construct the lower (or upper) envelope to the given range of curves.
   * \param begin An iterator pointing at the beginning of the curves range. 
   * \param end A past-the-end iterator for the curves range.
   * \param type The envelope type (LOWER or UPPER).
   * \param diagram Output: The minimization (or maximization) diagram.
   */
  template <class CurvesIterator>
  void insert_curves (const CurvesIterator& begin,
                      const CurvesIterator& end,
                      const Envelope_type& type,
                      Envelope_diagram_1& diagram)
  {
    // Subdivide the curves into x-monotone subcurves.
    CurvesIterator                     it;
    std::list<CGAL::Object>            objects;
    std::list<CGAL::Object>::iterator  obj_it;
    X_monotone_curve_2                 xcv;
    std::list<X_monotone_curve_2>      x_curves;

    for (it = begin; it != end; it++)
    {
      // Split the current curve to x-monotone subcurves.
      objects.clear();
      traits->make_x_monotone_2_object()(*it, std::back_inserter(objects));

      for (obj_it = objects.begin(); obj_it != objects.end(); ++obj_it)
      {
        if(CGAL::assign (xcv, *obj_itr))
          x_curves.push_back (xcv);
      }
    }

    // Construct the envelope of the x-monotone curves.
    insert_x_monotone_curves (x_curves.begin(), x_curves.end(),
                              type,
                              diagram);
    return;
  }

  /*!
   * Construct the lower (or upper) envelope to the given range of
   * x-monotone curves.
   * \param begin An iterator pointing at the beginning of the curves range. 
   * \param end A past-the-end iterator for the curves range.
   * \param type The envelope type (LOWER or UPPER).
   * \param diagram Output: The minimization (or maximization) diagram.
   */
  template <class XCurvesIterator>
  void insert_x_monotone_curves (const XCurvesIterator& begin,
                                 const XCurvesIterator& end,
                                 const Envelope_type& type,
                                 Envelope_diagram_1& diagram)
  {
    // Set the envelope type.
    env_type = type;

    // Separate the regular curves from the vertical ones.
    typename Traits_2::Is_vertical_2  is_vertical = 
                                              traits->is_vertical_2_object();

    Curve_pointer_list    reg_list;
    Curve_pointer_list    vert_list;
    XCurvesIterator       iter;

    for (iter = begin; iter != end; ++iter)
    {
      if (is_vertical (*iter))
        vert_list.push_back (&(*iter));
      else
        reg_list.push_back (&(*iter));
    }

    // Construct the envelope for the non-vertical curves.
    _construct_envelope_non_vertical (reg_list.begin(), reg_list.end(),
                                      diagram);

    // Merge the vertical segments.
    if (vert_list.size() > 0)
      _merge_vertical_segments (vert_list,
                                diagram);

    return;
  }

  /*!
   * Get the traits object.
   * \return A pointer to the traits object.
   */
  Traits_2* get_traits () const
  {
    return (traits);
  }

protected:

  /*!
   * Construct the lower/upper envelope of the given list of non-vertical
   * curves.
   * \param begin The first x-monotone curve.
   * \param end A past-the-end iterator for the curves.
   * \param out_d Output: The minimization (or maximization) diagram.
   */
  void _construct_envelope_non_vertical (Curve_pointer_iterator begin,
                                         Curve_pointer_iterator end,
                                         Envelope_diagram_1& out_d);

  /*!
   * Construct a singleton diagram, which matches a single curve.
   * \param cv The x-monotone curve.
   * \param out_d Output: The minimization (or maximization) diagram.
   */
  void _construct_singleton_diagram (const X_monotone_curve_2& cv,
                                     Envelope_diagram_1& out_d);

  /*
   * Merge two minimization (or maximization) diagrams.
   * \param d1 The first diagram, 
   *           representing the envelope of the curve set C1.
   * \param d1 The first diagram,
   *           representing the envelope of the curve set C1.
   * \param out_d Output: The merged diagram, representing the envelope of
   *                      the union of C1 and C2.
   */
  void _merge_envelopes (const Envelope_diagram_1& d1,
                         const Envelope_diagram_1& d2,
                         Envelope_diagram_1& out_d);

  /*!
   * Compare two vertices.
   * \param v1 The first vertex.
   * \param v2 The second vertex.
   * \param same_x Output parameter: TRUE iff x(v1) = x(v2).
   * \return SMALLER if x(v1) < x(v2). Or, in case x(v1) = x(v2), and
   *                 - we compute the lower envelope, and y(v1) < y(v2),
   *                 - we compute the upper envelope, and y(v1) > y(v2).
   *         LARGER if x(v1) > x(v2). Or, in case x(v1) = x(v2), and
   *                - we compute the lower envelope, and y(v1) > y(v2),
   *                - we compute the upper envelope, and y(v1) < y(v2).
   *         EQUAL if v1 = v2.
   */
  Comparison_result _compare_vertices (Vertex_const_handle v1,
                                       Vertex_const_handle v2,
                                       bool& same_x) const;

  /*!
   * Deal with an interval which is non-empty in one of the merged diagrams and
   * empty in the other.
   * \param e The non-empty edge.
   * \param v The next vertex to the right.
   * \param v_exists Whether the next vertex exists.
   * \param same_org Whether e and v originate from the same diagram.
   * \param out_d The merged diagram.
   */
  void _merge_single_interval (Edge_const_handle e,
                               Vertex_const_handle v, bool v_exists,
                               bool same_org,
                               Envelope_diagram_1& out_d);

  /*!
   * Merge two non-empty intervals into the merged diagram.
   * \param e1 The first non-empty edge.
   * \param is_leftmost1 Is it the leftmost edge in its diagram.
   * \param e2 The second non-empty edge.
   * \param is_leftmost2 Is it the leftmost edge in its diagram.
   * \param v The next vertex.
   * \param v_exists Whether such a vertex exists.
   * \param org_v The origin of v: 1 if it is from e1, 2 if it is from e2.
   * \param out_d The merged diagram.
   */
  void _merge_two_intervals (Edge_const_handle e1, bool is_leftmost1,
                             Edge_const_handle e2, bool is_leftmost2,
                             Vertex_const_handle v, bool v_exists,
                             int org_v,
                             Envelope_diagram_1& out_d);

  /*!
   * Append a vertex to the given diagram: The new vertex that represents the 
   * given point as the new rightmost vertex of the diagram. The edge 
   * between the current rightmost vertex and the new one contains the same 
   * curves as the input edge.
   * \param diag The diagram.
   * \param p The point that the new vertex is associated with.
   * \param e The input edge.
   * \return A handle for the vertex.
   */
  Vertex_handle _append_vertex (Envelope_diagram_1& diag,
                                const Point_2& p, Edge_handle e);

  /*! \struct
   * A functor used to sort vertical segments by their x-coordinate.
   */
  class Vertical_strict_weak_ordering
  {
  private:    
    const Traits_2         *traits;

  public:
    Vertical_strict_weak_ordering (const Traits_2 *_traits) :
      traits(_traits)
    {}

    bool operator() (const M_curve_2 *mcv1, const M_curve_2 *mcv2) const
    {
      /* RWRW!
      return (traits->compare_x_2_object() (traits->construct_min_vertex_2_object()(mcv1->xcv),
              traits->construct_min_vertex_2_object()(mcv1->xcv)) == SMALLER);
      */
      return (true);
    }
  };

  /*!
   * Merge the vertical segments into the lower/upper envelope given as a
   * minimization (or maximization) diagram.
   * \param vert_list The list of vertical segments.
   * \param out_d The input minimization (or maximization) diagram.
   *             The function merges the vertical segments into this diagram.
   */
  void _merge_vertical_segments (Curve_pointer_list& vert_list,
                                 Envelope_diagram_1& out_d);
};

CGAL_END_NAMESPACE

#include <CGAL/Envelope_2/Env_divide_and_conquer_2_impl.h>

#endif

